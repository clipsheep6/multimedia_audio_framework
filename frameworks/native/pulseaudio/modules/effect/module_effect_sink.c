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

#include "audio_effect_chain_adapter.h"
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
    struct BufferAttr *bufferAttr;
    pa_memblockq *bufInQ;
    int32_t frameLen;
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

#define MAX_16BIT 32768
#define MEMBLOCKQ_MAXLENGTH (16*1024*16)

// BEGIN QUEUE
static size_t MemblockqMissing(pa_memblockq *bq) {
    size_t l, tlength;
    pa_assert(bq);

    tlength = pa_memblockq_get_tlength(bq);
    if ((l = pa_memblockq_get_length(bq)) >= tlength)
        return 0;

    l = tlength - l;
    return l >= pa_memblockq_get_minreq(bq) ? l : 0;
}

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
    
    int offset = 0;
    size_t targetLength = pa_memblockq_get_tlength(u->bufInQ);
    int iterNum = pa_memblockq_get_length(u->bufInQ) / targetLength;
    chunk->index = 0;
    chunk->length = targetLength * iterNum;
    chunk->memblock = pa_memblock_new(si->sink->core->mempool, chunk->length);
    short *dst = pa_memblock_acquire_chunk(chunk);
    while (true) {
        if (pa_memblockq_peek_fixed_size(u->bufInQ, targetLength, &tchunk) != 0) {
            break;
        }
        if (tchunk.length < targetLength) {
            break;
        }
        pa_memblockq_drop(u->bufInQ, tchunk.length);

        float *bufIn = (float *)u->bufferAttr->bufIn;
        float *bufOut = (float *)u->bufferAttr->bufOut;
        short *src = pa_memblock_acquire_chunk(&tchunk);
        int i, tmp;
        for (i = 0; i < u->frameLen * 2; i++) {
            bufIn[i] = (float)(src[i]) / MAX_16BIT;
        }
        pa_memblock_release(tchunk.memblock);
        pa_memblock_unref(tchunk.memblock);
        EffectChainManagerProcess((void *)u->bufferAttr, si->origin_sink->name);
        
        for (i = 0; i < u->frameLen * 2; i++) {
            tmp = (int)(bufOut[i] * MAX_16BIT);
            if (tmp >= MAX_16BIT) {
                dst[i + offset] = MAX_16BIT - 1;
            } else if (tmp <= -MAX_16BIT) {
                dst[i + offset] = -MAX_16BIT;
            } else {
                dst[i + offset] = (short)tmp;
            }
        }
        offset += u->frameLen * 2;        
    }

    pa_memblock_release(chunk->memblock);
    
    return 0;
}
// END QUEUE

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
        pa_log("Failed to parse module arguments.");
        goto fail;
    }

    if (!(master = pa_namereg_get(m->core, pa_modargs_get_value(ma, "master", NULL), PA_NAMEREG_SINK))) {
        pa_log("Master sink not found");
        goto fail;
    }
	
    ss = m->core->default_sample_spec;
    sink_map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &sink_map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        pa_log("Invalid sample format specification or channel map");
        goto fail;
    }

    stream_map = sink_map;
    if (stream_map.channels != ss.channels) {
        pa_log("Number of channels doesn't match");
        goto fail;
    }

    if (pa_channel_map_equal(&stream_map, &master->channel_map))
        pa_log_warn("No effecting configured, proceeding nonetheless!");

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
        pa_log("Failed to create sink.");
        goto fail;
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

    if (!u->sinkInput)
        goto fail;

    u->sinkInput->pop = SinkInputPopCb;
    u->sinkInput->userdata = u;

    u->sink->input_to_master = u->sinkInput;

    // Set buffer attributes
    int32_t frameLen = EffectChainManagerGetFrameLen();
    u->frameLen = frameLen;
    size_t processSize = ss.channels * frameLen * sizeof(float);
    u->bufferAttr = pa_xnew0(struct BufferAttr, 1);
    pa_assert_se(u->bufferAttr->bufIn = (float *)malloc(processSize));
    pa_assert_se(u->bufferAttr->bufOut = (float *)malloc(processSize));
    u->bufferAttr->frameLen = frameLen;
    u->bufferAttr->numChan = ss.channels;
    
    u->bufInQ = pa_memblockq_new("module-effect-sink bufInQ", 0, MEMBLOCKQ_MAXLENGTH, ss.channels * frameLen * 2, &ss, 1, 1, 0, NULL);

    pa_sink_input_put(u->sinkInput);
    pa_sink_put(u->sink);
    pa_sink_input_cork(u->sinkInput, false);

    pa_modargs_free(ma);
    return 0;

fail:
    AUDIO_INFO_LOG("module_effect_sink: effect_sink create fail");
    if (ma)
        pa_modargs_free(ma);

    pa__done(m);

    return -1;
}

int pa__get_n_used(pa_module *m) {
    struct userdata *u;

    pa_assert(m);
    pa_assert_se(u = m->userdata);

    return pa_sink_linked_by(u->sink);
}

void pa__done(pa_module*m) {
    struct userdata *u;

    pa_assert(m);

    if (!(u = m->userdata))
        return;

    /* See comments in sink_input_kill_cb() above regarding
     * destruction order! */

    if (u->sinkInput)
        pa_sink_input_cork(u->sinkInput, true);

    if (u->sink)
        pa_sink_unlink(u->sink);

    if (u->sinkInput) {
        pa_sink_input_unlink(u->sinkInput);
        pa_sink_input_unref(u->sinkInput);
    }

    if (u->sink)
        pa_sink_unref(u->sink);

    pa_xfree(u);
}
