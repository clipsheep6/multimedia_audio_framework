/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#undef LOG_TAG
#define LOG_TAG "ModuleDuplicatedHidSink"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <pulse/pulseaudio.h>

#include <pulsecore/i18n.h>
#include <pulsecore/macro.h>
#include <pulsecore/sink.h>
#include <pulsecore/module.h>
#include <pulsecore/core-util.h>
#include <pulsecore/modargs.h>
#include <pulsecore/log.h>
#include <pulsecore/thread.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/proplist-util.h>

#include "audio_log.h"

PA_MODULE_AUTHOR("OpenHarmony");
PA_MODULE_DESCRIPTION(_("Duplicated Hdi Sink"));
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        "sink_name=<name of sink> "
        "sink_properties=<properties for the sink> "
        "format=<sample format> "
        "rate=<sample rate> "
        "channels=<number of channels> "
        "channel_map=<channel map>"
        "buffer_size=<custom buffer size>"
        "formats=<semi-colon separated sink formats>");

#define DEFAULT_SINK_NAME "DuplicatedHdi"
#define DEFAULT_BUFFER_SIZE 8192  // same as HDI Sink
#define PA_ERR (-1)

struct userdata {
    pa_core *core;
    pa_module *module;
    pa_sink *sink;

    pa_thread *thread;
    pa_thread_mq thread_mq;
    pa_rtpoll *rtpoll;

    uint32_t buffer_size;
    pa_usec_t block_usec;
    pa_usec_t timestamp;

    pa_idxset *formats;
    const char *sub_sink1;
    const char *sub_sink2;

    pa_stream *stream1;
    bool stream1_connected;
    bool update_stream1_bufferattr_after_connect;
    pa_stream *stream2;
    bool stream2_connected;
    bool update_stream2_bufferattr_after_connect;

    pa_threaded_mainloop *mainLoop;
    pa_mainloop_api *api;
    pa_context *context;
    bool isContextConnected;
    bool isMainLoopStarted;
};

static const char * const VALID_MODARGS[] = {
    "sink_name",
    "sink_properties",
    "format",
    "rate",
    "channels",
    "channel_map",
    "buffer_size"
    "formats",
    NULL
};

const uint64_t BUF_LENGTH_IN_MSEC = 20;

static int SinkProcessMsg(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk)
{
    switch (code) {
        case PA_SINK_MESSAGE_GET_LATENCY:
            *((int64_t*) data) = 0;
            return 0;
        default:
            break;
    }

    return pa_sink_process_msg(o, code, data, offset, chunk);
}

/* Called from the IO thread. */
static int SinkSetStateInIoThreadCb(pa_sink *s, pa_sink_state_t new_state, pa_suspend_cause_t new_suspend_cause)
{
    struct userdata *u;

    pa_assert(s);
    pa_assert_se(u = s->userdata);

    if (s->thread_info.state == PA_SINK_SUSPENDED || s->thread_info.state == PA_SINK_INIT) {
        if (PA_SINK_IS_OPENED(new_state)) {
            u->timestamp = pa_rtclock_now();
        }
    }

    return 0;
}

static void SinkUpdateRequestedLatencyCb(pa_sink *s)
{
    struct userdata *u;
    size_t nbytes;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    u->block_usec = pa_sink_get_requested_latency_within_thread(s);

    if (u->block_usec == (pa_usec_t) -1) {
        u->block_usec = s->thread_info.max_latency;
    }

    nbytes = pa_usec_to_bytes(u->block_usec, &s->sample_spec);
    pa_sink_set_max_rewind_within_thread(s, nbytes);
    pa_sink_set_max_request_within_thread(s, nbytes);
}

static void SinkReconfigureCb(pa_sink *s, pa_sample_spec *spec, bool passthrough)
{
    s->sample_spec = *spec;
}

static bool SinkSetFormatsCb(pa_sink *s, pa_idxset *formats)
{
    struct userdata *u = s->userdata;

    pa_assert(u);

    pa_idxset_free(u->formats, (pa_free_cb_t) pa_format_info_free);
    u->formats = pa_idxset_copy(formats, (pa_copy_func_t) pa_format_info_copy);

    return true;
}

static pa_idxset* SinkGetFormatsCb(pa_sink *s)
{
    struct userdata *u = s->userdata;

    pa_assert(u);

    return pa_idxset_copy(u->formats, (pa_copy_func_t) pa_format_info_copy);
}

void PAContextStateCb(pa_context *context, void *data)
{
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)data;
    AUDIO_INFO_LOG("Current Context State: %{public}d", pa_context_get_state(context));

    switch (pa_context_get_state(context)) {
        case PA_CONTEXT_READY:
            pa_threaded_mainloop_signal(mainLoop, 0);
            break;
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            pa_threaded_mainloop_signal(mainLoop, 0);
            break;

        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
    }
}

static void cork_stream(pa_stream *stream, bool cork) {
    pa_operation *operation;

    pa_assert(stream);

    if ((operation = pa_stream_cork(stream, cork, NULL, NULL)))
        pa_operation_unref(operation);
}

static void Stream1StateCb(pa_stream *stream, void *userdata) {
    struct userdata *u = userdata;

    pa_assert(u);

    switch (pa_stream_get_state(stream)) {
        case PA_STREAM_FAILED:
            pa_log_error("Stream1 failed.");
            u->stream1_connected = false;
            u->api->quit(u->api, -1);
            break;
        case PA_STREAM_TERMINATED:
            pa_log_debug("Stream terminated.");
            break;
        case PA_STREAM_READY:
            if (PA_SINK_IS_OPENED(u->sink->thread_info.state))
                cork_stream(stream, false);

            /* Only call our requested_latency_cb when requested_latency
             * changed between PA_STREAM_CREATING -> PA_STREAM_READY, because
             * we don't want to override the initial tlength set by the server
             * without a good reason. */
            if (u->update_stream1_bufferattr_after_connect)
                pa_log_debug("Stream1 ready.");
                // sink_update_requested_latency_cb(u->sink);
            // else
            //     stream_changed_buffer_attr_cb(stream, userdata);
        case PA_STREAM_CREATING:
        case PA_STREAM_UNCONNECTED:
            break;
    }
}

static void Stream2StateCb(pa_stream *stream, void *userdata) {
    struct userdata *u = userdata;

    pa_assert(u);

    switch (pa_stream_get_state(stream)) {
        case PA_STREAM_FAILED:
            pa_log_error("Stream2 failed.");
            u->stream2_connected = false;
            u->api->quit(u->api, -1);
            break;
        case PA_STREAM_TERMINATED:
            pa_log_debug("Stream2 terminated.");
            break;
        case PA_STREAM_READY:
            if (PA_SINK_IS_OPENED(u->sink->thread_info.state))
                cork_stream(stream, false);

            /* Only call our requested_latency_cb when requested_latency
             * changed between PA_STREAM_CREATING -> PA_STREAM_READY, because
             * we don't want to override the initial tlength set by the server
             * without a good reason. */
            if (u->update_stream2_bufferattr_after_connect)
                pa_log_debug("Stream2 ready.");
                // sink_update_requested_latency_cb(u->sink);
            // else
            //     stream_changed_buffer_attr_cb(stream, userdata);
        case PA_STREAM_CREATING:
        case PA_STREAM_UNCONNECTED:
            break;
    }
}

static void ProcessRewind(struct userdata *u, pa_usec_t now)
{
    size_t rewindNbytes;
    size_t inBuffer;
    pa_usec_t delay;

    pa_assert(u);

    rewindNbytes = u->sink->thread_info.rewind_nbytes;
    if (!PA_SINK_IS_OPENED(u->sink->thread_info.state) || rewindNbytes <= 0) {
        goto do_nothing;
    }
    AUDIO_DEBUG_LOG("Requested to rewind %lu bytes.", (unsigned long) rewindNbytes);

    if (u->timestamp <= now) {
        goto do_nothing;
    }

    delay = u->timestamp - now;
    inBuffer = pa_usec_to_bytes(delay, &u->sink->sample_spec);
    if (inBuffer <= 0) {
        goto do_nothing;
    }

    if (rewindNbytes > inBuffer) {
        rewindNbytes = inBuffer;
    }

    pa_sink_process_rewind(u->sink, rewindNbytes);
    u->timestamp -= pa_bytes_to_usec(rewindNbytes, &u->sink->sample_spec);

    AUDIO_DEBUG_LOG("Rewound %lu bytes.", (unsigned long) rewindNbytes);
    return;

do_nothing:
    pa_sink_process_rewind(u->sink, 0);
}

static void PaWriteStream(pa_stream *stream, pa_memchunk *pchunk) {
    size_t index;
    size_t length;
    ssize_t count = 0;
    void *p = NULL;

    pa_assert(pchunk);

    index = pchunk->index;
    length = pchunk->length;
    p = pa_memblock_acquire(pchunk->memblock);
    pa_assert(p);

    while (true) {
        uint64_t writeLen = 0;

        int32_t ret =  pa_stream_write(stream, (void *)p + index, length , NULL, 0LL, PA_SEEK_RELATIVE);
        if (writeLen > length) {
            AUDIO_ERR_LOG("Error writeLen > actual bytes. Length: %zu, Written: %" PRIu64 " bytes, %d ret",
                         length, writeLen, ret);
            count = -1 - count;
            break;
        }
        if (writeLen == 0) {
            AUDIO_ERR_LOG("Failed to render Length: %{public}zu, Written: %{public}" PRIu64 " bytes, %{public}d ret",
                length, writeLen, ret);
            count = -1 - count;
            break;
        } else {
            count += writeLen;
            index += writeLen;
            length -= writeLen;
            if (length == 0) {
                break;
            }
        }
    }
    pa_memblock_release(pchunk->memblock);
    pa_memblock_unref(pchunk->memblock);
}

static void ProcessRender(struct userdata *u, pa_usec_t now)
{
    size_t ate = 0;

    pa_assert(u);

    /* This is the configured latency. Sink inputs connected to us
    might not have a single frame more than the maxrequest value
    queued. Hence: at maximum read this many bytes from the sink
    inputs. */

    /* Fill the buffer up the latency size */
    while (u->timestamp < now + u->block_usec) {
        pa_memchunk chunk;

        pa_sink_render(u->sink, u->sink->thread_info.max_request, &chunk);
        pa_memblock_unref(chunk.memblock);

        u->timestamp += pa_bytes_to_usec(chunk.length, &u->sink->sample_spec);

        ate += chunk.length;
        if (ate >= u->sink->thread_info.max_request) {
            break;
        }
        PaWriteStream(u->stream1, &chunk);
        PaWriteStream(u->stream2, &chunk);
    }
}

static void ThreadFunc(void *userdata)
{
    struct userdata *u = userdata;

    pa_assert(u);

    AUDIO_DEBUG_LOG("Thread starting up");
    if (u->core->realtime_scheduling) {
        pa_thread_make_realtime(u->core->realtime_priority);
    }

    pa_thread_mq_install(&u->thread_mq);

    u->timestamp = pa_rtclock_now();

    for (;;) {
        pa_usec_t now = 0;
        int ret;

        if (PA_SINK_IS_OPENED(u->sink->thread_info.state)) {
            now = pa_rtclock_now();
        }

        if (PA_UNLIKELY(u->sink->thread_info.rewind_requested)) {
            ProcessRewind(u, now);
        }

        /* Render some data and drop it immediately */
        if (PA_SINK_IS_OPENED(u->sink->thread_info.state)) {
            if (u->timestamp <= now) {
                ProcessRender(u, now);
            }

            pa_rtpoll_set_timer_absolute(u->rtpoll, u->timestamp);
        } else {
            pa_rtpoll_set_timer_disabled(u->rtpoll);
        }

        /* Hmm, nothing to do. Let's sleep */
        if ((ret = pa_rtpoll_run(u->rtpoll)) < 0) {
            goto fail;
        }

        if (ret == 0) {
            goto finish;
        }
    }

fail:
    /* If this was no regular exit from the loop we have to continue
     * processing messages until we received PA_MESSAGE_SHUTDOWN */
    pa_asyncmsgq_post(u->thread_mq.outq, PA_MSGOBJECT(u->core),
        PA_CORE_MESSAGE_UNLOAD_MODULE, u->module, 0, NULL, NULL);
    pa_asyncmsgq_wait_for(u->thread_mq.inq, PA_MESSAGE_SHUTDOWN);

finish:
    AUDIO_DEBUG_LOG("Thread shutting down");
}

int InitFailed(pa_module *m, pa_modargs *ma)
{
    AUDIO_ERR_LOG("Inner Capturer Sink Init Failed");
    if (ma)
        pa_modargs_free(ma);

    pa__done(m);

    return PA_ERR;
}

int CreateSink(pa_module *m, pa_modargs *ma, struct userdata *u)
{
    pa_sample_spec ss;
    pa_channel_map map;
    pa_sink_new_data data;
    pa_format_info *format;

    pa_assert(m);

    ss = m->core->default_sample_spec;
    map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        AUDIO_ERR_LOG("Invalid sample format specification or channel map");
        return PA_ERR;
    }

    pa_sink_new_data_init(&data);
    data.driver = __FILE__;
    data.module = m;
    pa_sink_new_data_set_name(&data, pa_modargs_get_value(ma, "sink_name", DEFAULT_SINK_NAME));
    pa_sink_new_data_set_sample_spec(&data, &ss);
    pa_sink_new_data_set_channel_map(&data, &map);
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_DESCRIPTION, _("Null Output"));
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_CLASS, "capturer");
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING, "innercapturer");

    u->formats = pa_idxset_new(NULL, NULL);
    format = pa_format_info_new();
    format->encoding = PA_ENCODING_PCM;
    pa_idxset_put(u->formats, format, NULL);

    if (pa_modargs_get_proplist(ma, "sink_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        AUDIO_ERR_LOG("Invalid properties");
        pa_sink_new_data_done(&data);
        return PA_ERR;
    }

    u->sink = pa_sink_new(m->core, &data, PA_SINK_LATENCY | PA_SINK_DYNAMIC_LATENCY);
    pa_sink_new_data_done(&data);

    if (!u->sink) {
        AUDIO_ERR_LOG("Failed to create sink.");
        return PA_ERR;
    }

    u->sink->parent.process_msg = SinkProcessMsg;
    u->sink->set_state_in_io_thread = SinkSetStateInIoThreadCb;
    u->sink->update_requested_latency = SinkUpdateRequestedLatencyCb;
    u->sink->reconfigure = SinkReconfigureCb;
    u->sink->get_formats = SinkGetFormatsCb;
    u->sink->set_formats = SinkSetFormatsCb;
    u->sink->userdata = u;

    return 0;
}

int ResetPaContext(struct userdata *u)
{
    AUDIO_DEBUG_LOG("Enter ResetPaContext");
    if (u->context) {
        pa_context_set_state_callback(u->context, NULL, NULL);
        if (u->isContextConnected == true) {
            pa_threaded_mainloop_lock(u->mainLoop);
            pa_context_disconnect(u->context);
            pa_context_unref(u->context);
            u->isContextConnected = false;
            u->context = NULL;
        }
    }

    if (u->mainLoop) {
        pa_threaded_mainloop_free(u->mainLoop);
        u->isMainLoopStarted  = false;
        u->mainLoop = NULL;
    }

    u->api = NULL;
    return 0;
}

int HandleMainLoopStart(struct userdata *u)
{
    pa_threaded_mainloop_lock(u->mainLoop);
    if (pa_threaded_mainloop_start(u->mainLoop) < 0) {
        return -1;
    }
    u->isMainLoopStarted = true;

    while (true) {
        pa_context_state_t state = pa_context_get_state(u->context);
        if (state == PA_CONTEXT_READY) {
            AUDIO_INFO_LOG("pa context is ready");
            break;
        }

        if (!PA_CONTEXT_IS_GOOD(state)) {
            int error = pa_context_errno(u->context);
            AUDIO_ERR_LOG("Context bad state error: %{public}s", pa_strerror(error));
            pa_threaded_mainloop_unlock(u->mainLoop);
            ResetPaContext(u);
            return -1;
        }
        pa_threaded_mainloop_wait(u->mainLoop);
    }
    return 0;
}

int InitPaContext(struct userdata *u)
{
    AUDIO_DEBUG_LOG("Enter InitPaContext");
    if (u->context != NULL) {
        AUDIO_INFO_LOG("Context is not null, return");
        return 0;
    }
    u->mainLoop = pa_threaded_mainloop_new();
    CHECK_AND_RETURN_RET_LOG(u->mainLoop != NULL, -1, "Failed to init pa mainLoop");
    u->api = pa_threaded_mainloop_get_api(u->mainLoop);
    pa_threaded_mainloop_set_name(u->mainLoop, "OS_RendererML");
    if (u->api == NULL) {
        pa_threaded_mainloop_free(u->mainLoop);
        AUDIO_ERR_LOG("Get api from mainLoop failed");
        return -1;
    }

    char packageName[1024] = { 0 };
    snprintf(packageName, sizeof(packageName), "app-pid<%d>-uid<%d>", getpid(), getuid());
    u->context = pa_context_new(u->api, packageName);
    if (u->context == NULL) {
        pa_threaded_mainloop_free(u->mainLoop);
        AUDIO_ERR_LOG("New context failed");
        return -1;
    }

    pa_context_set_state_callback(u->context, PAContextStateCb, u->mainLoop);
    if (pa_context_connect(u->context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        int error = pa_context_errno(u->context);
        AUDIO_ERR_LOG("Context connect error: %{public}s", pa_strerror(error));
        return -1;
    }
    u->isContextConnected = true;
    HandleMainLoopStart(u);

    return 0;
}

int CheckReturnIfinvalid(bool expr, const int32_t retVal)
{
    do {
        if (!(expr)) {
            return retVal;
        }
    } while (false);
    return 0;
}

int CheckReturnIfStreamInvalid(pa_stream *paStream, const int32_t retVal)
{
    do {
        if (!(paStream && PA_STREAM_IS_GOOD(pa_stream_get_state(paStream)))) {
            return retVal;
        }
    } while (false);
    return 0;
}

int SetPaProplist(pa_proplist *propList, pa_sample_spec *sampleSpec, const char *streamName)
{
    pa_proplist_sets(propList, "stream.type", streamName);
    // char channels[20] = {0};
    // sprintf(channels, "%d", sampleSpec.channels);
    // pa_proplist_sets(propList, "stream.channelLayout", channels);
    pa_proplist_sets(propList, PA_PROP_APPLICATION_NAME, "PulseAudio");
    pa_proplist_sets(propList, PA_PROP_APPLICATION_ID, "org.PulseAudio.PulseAudio");
    pa_proplist_sets(propList, PA_PROP_APPLICATION_VERSION, PACKAGE_VERSION);
    pa_init_proplist(propList);
    return 0;
}

int32_t ConnectStreamToPA(pa_stream *paStream, pa_sample_spec *sampleSpec, const char *deviceName)
{
    uint32_t tlength = 4; // 4 is tlength of playback
    uint32_t maxlength = 4; // 4 is max buffer length of playback
    uint32_t prebuf = 1; // 1 is prebuf of playback

    AUDIO_INFO_LOG("Create ipc playback stream tlength: %{public}u, maxlength: %{public}u", tlength, maxlength);
    pa_buffer_attr bufferAttr;
    bufferAttr.fragsize = -1;
    bufferAttr.prebuf = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC * prebuf, sampleSpec);
    bufferAttr.maxlength = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC * maxlength, sampleSpec);
    bufferAttr.tlength = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC * tlength, sampleSpec);
    bufferAttr.minreq = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC, sampleSpec);
    AUDIO_INFO_LOG("bufferAttr, maxLength: %{public}u, tlength: %{public}u, prebuf: %{public}u",
        maxlength, tlength, prebuf);

    int32_t result = pa_stream_connect_playback(paStream, deviceName, &bufferAttr,
        (pa_stream_flags_t)(PA_STREAM_ADJUST_LATENCY | PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_START_CORKED |
        PA_STREAM_VARIABLE_RATE), NULL, NULL);
    if (result < 0) {
        return result;
    }
    return 0;
}

bool InitPaStream(pa_modargs *ma, struct userdata *u)
{
    AUDIO_DEBUG_LOG("InitPaStream");
    pa_threaded_mainloop_lock(u->mainLoop);
    if (CheckReturnIfinvalid(u->mainLoop && u->context, -1) < 0) {
        AUDIO_ERR_LOG("CheckReturnIfinvalid failed");
        return -1;
    }

    // Use struct to save spec size
    pa_sample_spec sampleSpec = { 0 };
    if (pa_modargs_get_sample_spec(ma, &sampleSpec) < 0) {
        AUDIO_ERR_LOG("pa_modargs_get_sample_spec failed");
        return false;
    }
    pa_proplist *propList = pa_proplist_new();
    if (propList == NULL) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        return false;
    }
    // const std::string streamName = GetStreamName(processConfig.streamType);
    const char* streamName = "ring";
    CHECK_AND_RETURN_RET_LOG(SetPaProplist(propList, &sampleSpec, streamName) == 0, NULL,
        "set pa proplist failed");

    pa_channel_map map;
    u->stream1 = pa_stream_new_with_proplist(u->context, streamName, &sampleSpec, &map, propList);
    if (!u->stream1) {
        int32_t error = pa_context_errno(u->context);
        pa_proplist_free(propList);
        AUDIO_ERR_LOG("pa_stream_new_with_proplist failed, error: %{public}d", error);
        return false;
    }
    u->stream2 = pa_stream_new_with_proplist(u->context, streamName, &sampleSpec, &map, propList);
    if (!u->stream2) {
        int32_t error = pa_context_errno(u->context);
        pa_proplist_free(propList);
        AUDIO_ERR_LOG("pa_stream_new_with_proplist failed, error: %{public}d", error);
        return false;
    }

    pa_proplist_free(propList);
    pa_stream_set_state_callback(u->stream1, Stream1StateCb, u);
    int ret = ConnectStreamToPA(u->stream1, &sampleSpec, u->sub_sink1);
    if (ret < 0) {
        AUDIO_ERR_LOG("ConnectStreamToPA Failed");
        int32_t error = pa_context_errno(u->context);
        AUDIO_ERR_LOG("connection to stream error: %{public}d", error);
        return NULL;
    }
    pa_stream_set_state_callback(u->stream2, Stream2StateCb, u);
    ret = ConnectStreamToPA(u->stream2, &sampleSpec, u->sub_sink2);
    if (ret < 0) {
        AUDIO_ERR_LOG("ConnectStreamToPA Failed");
        int32_t error = pa_context_errno(u->context);
        AUDIO_ERR_LOG("connection to stream error: %{public}d", error);
        return NULL;
    }

    pa_threaded_mainloop_unlock(u->mainLoop);

    return true;
}

int pa__init(pa_module *m)
{
    struct userdata *u = NULL;
    pa_modargs *ma = NULL;
    size_t nbytes;
    int mq;
    int mg;

    pa_assert(m);
    ma = pa_modargs_new(m->argument, VALID_MODARGS);
    CHECK_AND_RETURN_RET_LOG(ma != NULL, InitFailed(m, ma),
        "Failed to parse module arguments.");

    m->userdata = u = pa_xnew0(struct userdata, 1);
    u->core = m->core;
    u->module = m;
    u->rtpoll = pa_rtpoll_new();

    mq = pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll);
    CHECK_AND_RETURN_RET_LOG(mq >=0, InitFailed(m, ma), "pa_thread_mq_init() failed.");

    if (CreateSink(m, ma, u) != 0) {
        return InitFailed(m, ma);
    }

    pa_sink_set_asyncmsgq(u->sink, u->thread_mq.inq);
    pa_sink_set_rtpoll(u->sink, u->rtpoll);

    u->buffer_size = DEFAULT_BUFFER_SIZE;

    mg = pa_modargs_get_value_u32(ma, "buffer_size", &u->buffer_size);
    CHECK_AND_RETURN_RET_LOG(mg >= 0, InitFailed(m, ma),
        "Failed to parse buffer_size arg in capturer sink");

    u->block_usec = pa_bytes_to_usec(u->buffer_size, &u->sink->sample_spec);
    nbytes = pa_usec_to_bytes(u->block_usec, &u->sink->sample_spec);

    pa_sink_set_max_rewind(u->sink, nbytes);

    pa_sink_set_max_request(u->sink, u->buffer_size);


    u->sub_sink1 = pa_modargs_get_value(ma, "subSink1", "");
    u->sub_sink2 = pa_modargs_get_value(ma, "subSink2", "");
    if (!(u->thread = pa_thread_new("OS_InnerCap", ThreadFunc, u))) {
        AUDIO_ERR_LOG("Failed to create thread.");
        return InitFailed(m, ma);
    }
    pa_sink_set_latency_range(u->sink, 0, u->block_usec);
    pa_sink_put(u->sink);
    pa_modargs_free(ma);
    return 0;
}

int pa__get_n_used(pa_module *m)
{
    struct userdata *u;

    pa_assert(m);
    pa_assert_se(u = m->userdata);

    return pa_sink_linked_by(u->sink);
}

void pa__done(pa_module*m)
{
    struct userdata *u;

    pa_assert(m);

    if (!(u = m->userdata)) {
        return;
    }

    if (u->sink) {
        pa_sink_unlink(u->sink);
    }

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    pa_thread_mq_done(&u->thread_mq);

    if (u->sink) {
        pa_sink_unref(u->sink);
    }

    if (u->rtpoll) {
        pa_rtpoll_free(u->rtpoll);
    }

    if (u->formats) {
        pa_idxset_free(u->formats, (pa_free_cb_t) pa_format_info_free);
    }

    pa_xfree(u);
}