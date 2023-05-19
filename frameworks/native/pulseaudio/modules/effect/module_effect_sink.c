/***
  This file is part of PulseAudio.

  Copyright 2004-2009 Lennart Poettering

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pulse/xmalloc.h>

#include <pulsecore/namereg.h>
#include <pulsecore/sink.h>
#include <pulsecore/module.h>
#include <pulsecore/core-util.h>
#include <pulsecore/modargs.h>
#include <pulsecore/log.h>
#include <pulsecore/rtpoll.h>

#include "securec.h"
#include "audio_effect_chain_adapter.h"
// #include "audio_effect.h"
#include "audio_log.h"

PA_MODULE_AUTHOR("OpenHarmony");
PA_MODULE_DESCRIPTION(_("Effect sink"));
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        "sink_name=<name for the sink> "
        "rate=<sample rate> "
);

struct userdata {
    pa_module *module;
    pa_sink *sink;
    pa_sink_input *sinkInput;
    // struct BufferAttr *bufferAttr;
    struct AudioEffectConfigAdapter *ioBufferConfig;
    pa_memblockq *bufInQ;
    int32_t processLen;
    size_t processSize;
    pa_sample_format_t format;
    bool auto_desc;
};

static const char* const valid_modargs[] = {
    "sink_name",
    "rate",
    NULL
};

/* Called from I/O thread context */
static int SinkProcessMsg(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk) {
    struct userdata *u = PA_SINK(o)->userdata;

    switch (code) {

        case PA_SINK_MESSAGE_GET_LATENCY:

            /* The sink is _put() before the sink input is, so let's
             * make sure we don't access it yet */
            if (!PA_SINK_IS_LINKED(u->sink->thread_info.state) ||
                !PA_SINK_INPUT_IS_LINKED(u->sinkInput->thread_info.state)) {
                *((int64_t*) data) = 0;
                return 0;
            }

            *((int64_t*) data) =
                /* Get the latency of the master sink */
                pa_sink_get_latency_within_thread(u->sinkInput->sink, true) +

                /* Add the latency internal to our sink input on top */
                pa_bytes_to_usec(pa_memblockq_get_length(u->sinkInput->thread_info.render_memblockq), &u->sinkInput->sink->sample_spec);

            return 0;
    }

    return pa_sink_process_msg(o, code, data, offset, chunk);
}

/* Called from main context */
static int SinkSetStateInMainThread(pa_sink *s, pa_sink_state_t state, pa_suspend_cause_t suspend_cause) {
    struct userdata *u;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    if (!PA_SINK_IS_LINKED(state) ||
        !PA_SINK_INPUT_IS_LINKED(u->sinkInput->state))
        return 0;

    pa_sink_input_cork(u->sinkInput, state == PA_SINK_SUSPENDED);
    return 0;
}

/* Called from the IO thread. */
static int SinkSetStateInIoThreadCb(pa_sink *s, pa_sink_state_t new_state, pa_suspend_cause_t new_suspend_cause) {
    struct userdata *u;

    pa_assert(s);
    pa_assert_se(u = s->userdata);

    /* When set to running or idle for the first time, request a rewind
     * of the master sink to make sure we are heard immediately */
    if (PA_SINK_IS_OPENED(new_state) && s->thread_info.state == PA_SINK_INIT) {
        pa_log_debug("Requesting rewind due to state change.");
        pa_sink_input_request_rewind(u->sinkInput, 0, false, true, true);
    }

    return 0;
}

/* Called from I/O thread context */
static void SinkRequestRewind(pa_sink *s) {
    struct userdata *u;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    if (!PA_SINK_IS_LINKED(u->sink->thread_info.state) ||
        !PA_SINK_INPUT_IS_LINKED(u->sinkInput->thread_info.state))
        return;

    pa_sink_input_request_rewind(u->sinkInput, s->thread_info.rewind_nbytes, true, false, false);
}

/* Called from I/O thread context */
static void SinkUpdateRequestedLatency(pa_sink *s) {
    struct userdata *u;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    if (!PA_SINK_IS_LINKED(u->sink->thread_info.state) ||
        !PA_SINK_INPUT_IS_LINKED(u->sinkInput->thread_info.state))
        return;

    /* Just hand this one over to the master sink */
    pa_sink_input_set_requested_latency_within_thread(
            u->sinkInput,
            pa_sink_get_requested_latency_within_thread(s));
}

// BEGIN Utility functions
#define FLOAT_EPS 1e-9f;
#define MEMBLOCKQ_MAXLENGTH (16*1024*16)
static uint32_t Read24Bit(const uint8_t *p) {
    return
        ((uint32_t) p[2] << 16) |
        ((uint32_t) p[1] << 8) |
        ((uint32_t) p[0]);
}

static void Write24Bit(uint8_t *p, uint32_t u) {
    p[2] = (uint8_t) (u >> 16);
    p[1] = (uint8_t) (u >> 8);
    p[0] = (uint8_t) u;
}

void ConvertFrom16BitToFloat(unsigned n, const int16_t *a, float *b) {
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1 << 15));
    }
}

void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b) {
    for (; n > 0; n--) {
        int32_t s = Read24Bit(a) << 8;
        *b = s * (1.0f / (1U << 31));
        a += 3;
        b++;
    }
}

void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b) {
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << 31));
    }
}

float CapMax(float v) {
    if (v > 1.0f) {
        return 1.0f - FLOAT_EPS;
    } else if (v < -1.0f) {
        return -1.0f + FLOAT_EPS;
    } else {
        return v;
    }
}

void ConvertFromFloatTo16Bit(unsigned n, const float *a, int16_t *b) {
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1 << 15);
        *(b++) = (int16_t) v;
    }
}

void ConvertFromFloatTo24Bit(unsigned n, const float *a, uint8_t *b) {
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << 31);        
        Write24Bit(b, ((uint32_t) v) >> 8);
        a++;
        b += 3;
    }
}

void ConvertFromFloatTo32Bit(unsigned n, const float *a, int32_t *b) {
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << 31);
        *(b++) = (int32_t) v;
    }
}

static void ConvertToFloat(pa_sample_format_t format, unsigned n, void *src, float *dst) {
    pa_assert(a);
    pa_assert(b);
    switch (format) {
        case PA_SAMPLE_S16LE:
            ConvertFrom16BitToFloat(n, src, dst);
            break;
        case PA_SAMPLE_S24LE:
            ConvertFrom24BitToFloat(n, src, dst);
            break;
        case PA_SAMPLE_S32LE:
            ConvertFrom32BitToFloat(n, src, dst);
            break;
        default:
            memcpy_s(src, n, dst, n);
            break;
    }
}

static void ConvertFromFloat(pa_sample_format_t format, unsigned n, float *src, void *dst) {
    pa_assert(a);
    pa_assert(b);
    switch (format) {
        case PA_SAMPLE_S16LE:
            ConvertFromFloatTo16Bit(n, src, dst);
            break;
        case PA_SAMPLE_S24LE:
            ConvertFromFloatTo24Bit(n, src, dst);
            break;
        case PA_SAMPLE_S32LE:
            ConvertFromFloatTo32Bit(n, src, dst);
            break;
        default:
            memcpy_s(src, n, dst, n);
            break;
    }
}

static size_t MemblockqMissing(pa_memblockq *bq) {
    size_t l, tlength;
    pa_assert(bq);

    tlength = pa_memblockq_get_tlength(bq);
    if ((l = pa_memblockq_get_length(bq)) >= tlength)
        return 0;

    l = tlength - l;
    return l >= pa_memblockq_get_minreq(bq) ? l : 0;
}

// END Utility functions

/* Called from I/O thread context */
static int SinkInputPopCb(pa_sink_input *si, size_t nbytes, pa_memchunk *chunk) {
    struct userdata *u;
    size_t bytesMissing;
    pa_memchunk tchunk;

    pa_sink_input_assert_ref(si);
    pa_assert(chunk);
    pa_assert_se(u = si->userdata);

    if (!PA_SINK_IS_LINKED(u->sink->thread_info.state))
        return -1;

    while ((bytesMissing = MemblockqMissing(u->bufInQ)) != 0) {
        pa_memchunk nchunk;
        pa_sink_render(u->sink, bytesMissing, &nchunk);
        pa_memblockq_push(u->bufInQ, &nchunk);
        pa_memblock_release(nchunk.memblock);
        pa_memblock_unref(nchunk.memblock);
    }
    
    void *src;
    float *bufIn, *bufOut;
    size_t targetLength = pa_memblockq_get_tlength(u->bufInQ);
    int iterNum = pa_memblockq_get_length(u->bufInQ) / targetLength;
    chunk->index = 0;
    chunk->length = targetLength * iterNum;
    chunk->memblock = pa_memblock_new(si->sink->core->mempool, chunk->length);
    short *dst = pa_memblock_acquire_chunk(chunk);
    for (int k = 0; k < iterNum; k++) {
        if (pa_memblockq_peek_fixed_size(u->bufInQ, targetLength, &tchunk) != 0) {
            break;
        }
        if (tchunk.length < targetLength) {
            break;
        }
        pa_memblockq_drop(u->bufInQ, tchunk.length);

        src = pa_memblock_acquire_chunk(&tchunk);
        bufIn = u->ioBufferConfig->inputCfg.buffer.f32;
        bufOut = u->ioBufferConfig->outputCfg.buffer.f32;
        // bufIn = u->bufferAttr->bufIn;
        // bufOut = u->bufferAttr->bufOut;
        
        ConvertToFloat(u->format, u->processLen, src, bufIn);
        pa_memblock_release(tchunk.memblock);
        pa_memblock_unref(tchunk.memblock);

        EffectChainManagerProcess(u->ioBufferConfig, si->origin_sink->name);
        
        ConvertFromFloat(u->format, u->processLen, bufOut, dst);
        
        dst += u->processSize;
    }

    pa_memblock_release(chunk->memblock);
    
    return 0;
}
// END QUEUE

int InitFail(pa_module *m, pa_modargs *ma)
{
    AUDIO_ERR_LOG("Failed to create effect sink");
    if (ma)
        pa_modargs_free(ma);
    pa__done(m);
    return -1;
}

int pa__init(pa_module *m) {
    struct userdata *u;
    pa_sample_spec ss;
    pa_resample_method_t resampleMethod = PA_RESAMPLER_SRC_SINC_FASTEST; //PA_RESAMPLER_INVALID;
    pa_channel_map sink_map, stream_map;
    pa_modargs *ma;
    pa_sink *master;
    pa_sink_input_new_data sinkInputData;
    pa_sink_new_data sinkData;
    bool remix = true;

    pa_assert(m);

    if (!(ma = pa_modargs_new(m->argument, valid_modargs))) {
        AUDIO_ERR_LOG("Failed to parse module arguments.");
        return InitFail(m, ma);
    }

    if (!(master = pa_namereg_get(m->core, pa_modargs_get_value(ma, "master", NULL), PA_NAMEREG_SINK))) {
        AUDIO_ERR_LOG("Master sink not found");
        return InitFail(m, ma);
    }
	
    ss = m->core->default_sample_spec;
    sink_map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &sink_map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        AUDIO_ERR_LOG("Invalid sample format specification or channel map");
        return InitFail(m, ma);
    }

    stream_map = sink_map;
    if (stream_map.channels != ss.channels) {
        AUDIO_ERR_LOG("Number of channels doesn't match");
        return InitFail(m, ma);
    }

    u = pa_xnew0(struct userdata, 1);
    u->module = m;
    m->userdata = u;

    /* Create sink */
    pa_sink_new_data_init(&sinkData);
    sinkData.driver = __FILE__;
    sinkData.module = m;
    if (!(sinkData.name = pa_xstrdup(pa_modargs_get_value(ma, "sink_name", NULL))))
        sinkData.name = pa_sprintf_malloc("%s.effected", master->name);
    pa_sink_new_data_set_sample_spec(&sinkData, &ss);
    pa_sink_new_data_set_channel_map(&sinkData, &sink_map);
    pa_proplist_sets(sinkData.proplist, PA_PROP_DEVICE_MASTER_DEVICE, master->name);
    pa_proplist_sets(sinkData.proplist, PA_PROP_DEVICE_CLASS, "filter");

    if ((u->auto_desc = !pa_proplist_contains(sinkData.proplist, PA_PROP_DEVICE_DESCRIPTION))) {
        const char *k;
        k = pa_proplist_gets(master->proplist, PA_PROP_DEVICE_DESCRIPTION);
        pa_proplist_setf(sinkData.proplist, PA_PROP_DEVICE_DESCRIPTION, "effected %s", k ? k : master->name);
    }

    u->sink = pa_sink_new(m->core, &sinkData, master->flags & (PA_SINK_LATENCY|PA_SINK_DYNAMIC_LATENCY));
    pa_sink_new_data_done(&sinkData);

    if (!u->sink) {
        return InitFail(m, ma);
    }

    u->sink->parent.process_msg = SinkProcessMsg;
    u->sink->set_state_in_main_thread = SinkSetStateInMainThread;
    u->sink->set_state_in_io_thread = SinkSetStateInIoThreadCb;
    u->sink->update_requested_latency = SinkUpdateRequestedLatency;
    u->sink->request_rewind = SinkRequestRewind;
    u->sink->userdata = u;

    pa_sink_set_asyncmsgq(u->sink, master->asyncmsgq);

    /* Create sink input */
    pa_sink_input_new_data_init(&sinkInputData);
    sinkInputData.driver = __FILE__;
    sinkInputData.module = m;
    pa_sink_input_new_data_set_sink(&sinkInputData, master, false, true);
    sinkInputData.origin_sink = u->sink;
    const char *name = pa_sprintf_malloc("%s effected Stream", sinkData.name);
    pa_proplist_sets(sinkInputData.proplist, PA_PROP_MEDIA_NAME, name);
    pa_proplist_sets(sinkInputData.proplist, PA_PROP_MEDIA_ROLE, "filter");
    pa_proplist_sets(sinkInputData.proplist, "scene.type", "N/A");
    pa_proplist_sets(sinkInputData.proplist, "scene.mode", "N/A");
    pa_sink_input_new_data_set_sample_spec(&sinkInputData, &ss);
    pa_sink_input_new_data_set_channel_map(&sinkInputData, &stream_map);
    sinkInputData.flags = (remix ? 0 : PA_SINK_INPUT_NO_REMIX) | PA_SINK_INPUT_START_CORKED;
    sinkInputData.resample_method = resampleMethod;

    pa_sink_input_new(&u->sinkInput, m->core, &sinkInputData);
    pa_sink_input_new_data_done(&sinkInputData);

    if (!u->sinkInput) {
        return InitFail(m, ma);
    }

    u->sinkInput->pop = SinkInputPopCb;
    u->sinkInput->userdata = u;

    u->sink->input_to_master = u->sinkInput;

    // Set buffer attributes
    int32_t frameLen = EffectChainManagerGetFrameLen();
    u->format = ss.format;
    u->processLen = ss.channels * frameLen;
    u->processSize = u->processLen * sizeof(float);
    u->ioBufferConfig = pa_xnew0(struct AudioEffectConfigAdapter, 1);
    u->ioBufferConfig->inputCfg.samplingRate = ss.rate;
    u->ioBufferConfig->inputCfg.channels = ss.channels;
    u->ioBufferConfig->inputCfg.format = 3; // auto set to float
    u->ioBufferConfig->inputCfg.buffer.frameLength = frameLen;
    u->ioBufferConfig->outputCfg.samplingRate = ss.rate;
    u->ioBufferConfig->outputCfg.channels = ss.channels;
    u->ioBufferConfig->outputCfg.format = 3; // auto set to float
    u->ioBufferConfig->outputCfg.buffer.frameLength = frameLen;
    
    // u->bufferAttr = pa_xnew0(struct BufferAttr, 1);
    // pa_assert_se(u->ioBufferConfig.inputCfg.buffer.raw = (float *)malloc(u->processSize));
    // pa_assert_se(u->ioBufferConfig.outputCfg.buffer.raw = (float *)malloc(u->processSize));
    // u->bufferAttr->frameLen = frameLen;
    // u->bufferAttr->numChan = ss.channels;
    
    int32_t bitSize = pa_sample_size_of_format(ss.format);
    size_t targetSize = ss.channels * frameLen * bitSize;
    u->bufInQ = pa_memblockq_new("module-effect-sink bufInQ", 0, MEMBLOCKQ_MAXLENGTH, targetSize, &ss, 1, 1, 0, NULL);

    pa_sink_input_put(u->sinkInput);
    pa_sink_put(u->sink);
    pa_sink_input_cork(u->sinkInput, false);

    pa_modargs_free(ma);
    return 0;
}

int pa__get_n_used(pa_module *m) {
    struct userdata *u;

    pa_assert(m);
    pa_assert_se(u = m->userdata);

    return pa_sink_linked_by(u->sink);
}

void pa__done(pa_module *m) {
    struct userdata *u;

    pa_assert(m);

    if (!(u = m->userdata))
        return;

    if (u->sinkInput) {
        pa_sink_input_cork(u->sinkInput, true);
    }

    if (u->sink) {
        pa_sink_unlink(u->sink);
    }

    if (u->sinkInput) {
        pa_sink_input_unlink(u->sinkInput);
        pa_sink_input_unref(u->sinkInput);
    }

    if (u->sink) {
        pa_sink_unref(u->sink);
    }

    pa_xfree(u);
}
