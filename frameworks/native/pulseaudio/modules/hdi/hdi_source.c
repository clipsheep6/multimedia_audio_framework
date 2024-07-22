/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#undef LOG_TAG
#define LOG_TAG "HdiSource"

#include <config.h>
#include <inttypes.h>
#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulse/util.h>
#include <pulse/xmalloc.h>
#include <pulsecore/core.h>
#include <pulsecore/log.h>
#include <pulsecore/memchunk.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/thread.h>
#include <pulsecore/mix.h>
#include <pulsecore/memblockq.c>
#include <pulsecore/source.h>
#include <pulsecore/source-output.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "userdata.h"
#include "securec.h"
#include "audio_hdiadapter_info.h"
#include "audio_log.h"
#include "audio_source_type.h"
#include "audio_utils_c.h"
#include "capturer_source_adapter.h"
#include "audio_enhance_chain_adapter.h"
#include "hdi_adapter_manager_api.h"
#include "v4_0/audio_types.h"
#include "v4_0/iaudio_manager.h"

#define DEFAULT_SOURCE_NAME "hdi_input"
#define DEFAULT_DEVICE_CLASS "primary"
#define DEFAULT_AUDIO_DEVICE_NAME "Internal Mic"
#define DEFAULT_DEVICE_NETWORKID "LocalDevice"

#define DEFAULT_BUFFER_SIZE (1024 * 16)
#define MAX_VOLUME_VALUE 15.0
#define DEFAULT_LEFT_VOLUME MAX_VOLUME_VALUE
#define DEFAULT_RIGHT_VOLUME MAX_VOLUME_VALUE
#define MAX_LATENCY_USEC (PA_USEC_PER_SEC * 2)
#define MIN_LATENCY_USEC 500
#define AUDIO_POINT_NUM  1024
#define AUDIO_FRAME_NUM_IN_BUF 30
#define HDI_WAKEUP_BUFFER_TIME (PA_USEC_PER_SEC * 2)
#define DEVICE_TYPE_MIC 15
#define BASE_TEN 10

const char *DEVICE_CLASS_REMOTE = "remote";
const int32_t SUCCESS = 0;
const int32_t ERROR = -1;

static int PaHdiCapturerInit(struct Userdata *u);
static void PaHdiCapturerExit(struct Userdata *u);

static char *GetStateInfo(pa_source_state_t state)
{
    switch (state) {
        case PA_SOURCE_INVALID_STATE:
            return "INVALID";
        case PA_SOURCE_RUNNING:
            return "RUNNING";
        case PA_SOURCE_IDLE:
            return "IDLE";
        case PA_SOURCE_SUSPENDED:
            return "SUSPENDED";
        case PA_SOURCE_INIT:
            return "INIT";
        case PA_SOURCE_UNLINKED:
            return "UNLINKED";
        default:
            return "error state";
    }
}

static FrameDesc *AllocateFrameDesc(char *frame, uint64_t frameLen)
{
    FrameDesc *fdesc = (struct FrameDesc *)calloc(1, sizeof(FrameDesc));
    if (fdesc != NULL) {
        fdesc->frame = frame;
        fdesc->frameLen = frameLen;
    }

    return fdesc;
}

static void FreeFrameDesc(FrameDesc *fdesc)
{
    if (fdesc != NULL) {
        // frame in desc is allocated outside, do not free here
        free(fdesc);
    }
}

static void InitAuxCapture(struct Userdata *u)
{
    if (u->captureHandleEc != NULL) {
        u->captureHandleEc->Init(u->captureHandleEc->capture);
    }
    if (u->captureHandleMicRef != NULL) {
        u->captureHandleMicRef->Init(u->captureHandleMicRef->capture);
    }
}

static void DeinitAuxCapture(struct Userdata *u)
{
    if (u->captureHandleEc != NULL) {
        u->captureHandleEc->Deinit(u->captureHandleEc->capture);
    }
    if (u->captureHandleMicRef != NULL) {
        u->captureHandleMicRef->Deinit(u->captureHandleMicRef->capture);
    }
}

static void StartAuxCapture(struct Userdata *u)
{
    if (u->captureHandleEc != NULL) {
        u->captureHandleEc->Start(u->captureHandleEc->capture);
    }
    if (u->captureHandleMicRef != NULL) {
        u->captureHandleMicRef->Start(u->captureHandleMicRef->capture);
    }
}

static void StopAuxCapture(struct Userdata *u)
{
    if (u->captureHandleEc != NULL) {
        u->captureHandleEc->Stop(u->captureHandleEc->capture);
    }
    if (u->captureHandleMicRef != NULL) {
        u->captureHandleMicRef->Stop(u->captureHandleMicRef->capture);
    }
}

static void UserdataFree(struct Userdata *u)
{
    pa_assert(u);
    if (u->source) {
        pa_source_unlink(u->source);
    }

    if (u->thread) {
        pa_asyncmsgq_send(u->threadMq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    pa_thread_mq_done(&u->threadMq);

    if (u->source) {
        pa_source_unref(u->source);
    }

    if (u->rtpoll) {
        pa_rtpoll_free(u->rtpoll);
    }

    if (u->sourceAdapter) {
        u->sourceAdapter->CapturerSourceStop(u->sourceAdapter->wapper);
        u->sourceAdapter->CapturerSourceDeInit(u->sourceAdapter->wapper);
        StopAuxCapture(u);
        DeinitAuxCapture(u);
        ReleaseCaptureHandle(u->captureHandleEc);
        u->captureHandleEc = NULL;
        ReleaseCaptureHandle(u->captureHandleMicRef);
        u->captureHandleMicRef = NULL;
        UnLoadSourceAdapter(u->sourceAdapter);
    }

    if (u->sceneToCountMap) {
        pa_hashmap_free(u->sceneToCountMap);
    }
    if (u->sceneToResamplerMap) {
        pa_hashmap_free(u->sceneToResamplerMap);
    }
    pa_xfree(u);
}

static int SourceProcessMsg(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk)
{
    AUTO_CTRACE("hdi_source::SourceProcessMsg code: %d", code);
    struct Userdata *u = PA_SOURCE(o)->userdata;
    pa_assert(u);

    switch (code) {
        case PA_SOURCE_MESSAGE_GET_LATENCY: {
            pa_usec_t now;
            now = pa_rtclock_now();
            *((int64_t*)data) = (int64_t)now - (int64_t)u->timestamp;
            return 0;
        }
        default: {
            pa_log("SourceProcessMsg default case");
            return pa_source_process_msg(o, code, data, offset, chunk);
        }
    }
}

/* Called from the IO thread. */
static int SourceSetStateInIoThreadCb(pa_source *s, pa_source_state_t newState,
    pa_suspend_cause_t newSuspendCause)
{
    struct Userdata *u = NULL;
    pa_assert(s);
    pa_assert_se(u = s->userdata);
    AUDIO_INFO_LOG("Source[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass(u->sourceAdapter->deviceClass), GetStateInfo(s->thread_info.state), GetStateInfo(newState));

    if ((s->thread_info.state == PA_SOURCE_SUSPENDED || s->thread_info.state == PA_SOURCE_INIT) &&
        PA_SOURCE_IS_OPENED(newState)) {
        u->delayTime = 0;
        u->timestamp = pa_rtclock_now();
        if (u->attrs.sourceType == SOURCE_TYPE_WAKEUP) {
            u->timestamp -= HDI_WAKEUP_BUFFER_TIME;
        }
        if (newState == PA_SOURCE_RUNNING && !u->isCapturerStarted) {
            if (u->sourceAdapter->CapturerSourceStart(u->sourceAdapter->wapper)) {
                AUDIO_ERR_LOG("HDI capturer start failed");
                return -PA_ERR_IO;
            }
            StartAuxCapture(u);
            u->isCapturerStarted = true;
            AUDIO_DEBUG_LOG("Successfully started HDI capturer");
        }
    } else if (s->thread_info.state == PA_SOURCE_IDLE) {
        if (newState == PA_SOURCE_SUSPENDED) {
            if (u->isCapturerStarted) {
                u->sourceAdapter->CapturerSourceStop(u->sourceAdapter->wapper);
                u->isCapturerStarted = false;
                AUDIO_DEBUG_LOG("Stopped HDI capturer");
                StopAuxCapture(u);
            }
        } else if (newState == PA_SOURCE_RUNNING && !u->isCapturerStarted) {
            AUDIO_DEBUG_LOG("Idle to Running starting HDI capturing device");
            if (u->sourceAdapter->CapturerSourceStart(u->sourceAdapter->wapper)) {
                AUDIO_ERR_LOG("Idle to Running HDI capturer start failed");
                return -PA_ERR_IO;
            }
            StartAuxCapture(u);
            u->isCapturerStarted = true;
            AUDIO_DEBUG_LOG("Idle to Running: Successfully reinitialized HDI renderer");
        }
    }

    return 0;
}

static void PushData(pa_source_output *sourceOutput, pa_memchunk *chunk)
{
    pa_source_output_assert_ref(sourceOutput);
    pa_source_output_assert_io_context(sourceOutput);
    pa_assert(chunk);
    AUDIO_DEBUG_LOG("chunk length: %{public}zu", chunk->length);

    if (!sourceOutput->thread_info.direct_on_input) {
        pa_source_output_push(sourceOutput, chunk);
    }
}

static void PostSourceData(pa_source *source, pa_source_output *sourceOutput, pa_memchunk *chunk)
{
    pa_source_assert_ref(source);
    pa_source_assert_io_context(source);
    pa_assert(PA_SOURCE_IS_LINKED(source->thread_info.state));
    pa_assert(chunk);

    if (source->thread_info.state == PA_SOURCE_SUSPENDED) {
        return;
    }

    if (source->thread_info.soft_muted || !pa_cvolume_is_norm(&source->thread_info.soft_volume)) {
        pa_memchunk vchunk = *chunk;
        pa_memblock_ref(vchunk.memblock);
        pa_memchunk_make_writable(&vchunk, 0);
        if (source->thread_info.soft_muted || pa_cvolume_is_muted(&source->thread_info.soft_volume)) {
            pa_silence_memchunk(&vchunk, &source->sample_spec);
        } else {
            pa_volume_memchunk(&vchunk, &source->sample_spec, &source->thread_info.soft_volume);
        }
        PushData(sourceOutput, &vchunk);
        pa_memblock_unref(vchunk.memblock);
    } else {
        PushData(sourceOutput, chunk);
    }
}

static void EnhanceProcess(const uint32_t sceneKeyCode, pa_memchunk *chunk)
{
    pa_assert(chunk);
    void *src = pa_memblock_acquire_chunk(chunk);
    AUDIO_DEBUG_LOG("chunk length: %{public}zu sceneKey: %{public}u", chunk->length, sceneKeyCode);
    pa_memblock_release(chunk->memblock);

    if (CopyToEnhanceBufferAdapter(src, chunk->length) != 0) {
        return;
    }
    if (EnhanceChainManagerProcess(sceneKeyCode, chunk->length) != 0) {
        return;
    }
    void *dst = pa_memblock_acquire_chunk(chunk);
    CopyFromEnhanceBufferAdapter(dst, chunk->length);
    pa_memblock_release(chunk->memblock);
}

static void EnhanceProcessAndPost(struct Userdata *u, const uint32_t sceneKeyCode, pa_memchunk *enhanceChunk)
{
    pa_assert(u);
    pa_assert(enhanceChunk);
    pa_source *source = u->source;
    pa_source_assert_ref(source);
    
    void *state = NULL;
    pa_source_output *sourceOutput;
    EnhanceProcess(sceneKeyCode, enhanceChunk);

    uint32_t capturerId = u->capturerId;
    uint32_t rendererId = u->rendererId;
    while ((sourceOutput = pa_hashmap_iterate(source->thread_info.outputs, &state, NULL))) {
        pa_source_output_assert_ref(sourceOutput);
        const char *sourceOutputSceneType = pa_proplist_gets(sourceOutput->proplist, "scene.type");
        
        uint32_t sceneTypeCode = 0;
        if (GetSceneTypeCode(sourceOutputSceneType, &sceneTypeCode) != 0) {
            AUDIO_ERR_LOG("GetSceneTypeCode failed");
            continue;
        }
        uint32_t sceneKeyCodeTemp = 0;
        sceneKeyCodeTemp = (sceneTypeCode << SCENE_TYPE_OFFSET) + (capturerId << CAPTURER_ID_OFFSET) + rendererId;
        if (sceneKeyCode != sceneKeyCodeTemp) {
            continue;
        }
        PostSourceData(source, sourceOutput, enhanceChunk);
    }
}

static void PostDataBypass(pa_source *source, pa_memchunk *chunk)
{
    pa_source_assert_ref(source);
    pa_assert(chunk);
    void *state = NULL;
    pa_source_output *sourceOutput;
    while ((sourceOutput = pa_hashmap_iterate(source->thread_info.outputs, &state, NULL))) {
        pa_source_output_assert_ref(sourceOutput);
        const char *sourceOutputSceneBypass = pa_proplist_gets(sourceOutput->proplist, "scene.bypass");
        if (sourceOutputSceneBypass == NULL) {
            continue;
        }
        if (strcmp(sourceOutputSceneBypass, DEFAULT_SCENE_BYPASS) == 0) {
            PostSourceData(source, sourceOutput, chunk);
        }
    }
}

static int32_t HandleCaptureFrame(const struct Userdata *u,
    char *buffer, uint64_t requestBytes, uint64_t *replyBytes)
{
    uint64_t replyBytesEc = 0;
    if (u->ecType == EC_NONE) {
        u->sourceAdapter->CapturerSourceFrame(
            u->sourceAdapter->wapper, buffer, requestBytes, replyBytes);
    } else {
        if (u->ecType == EC_SAME_ADAPTER) {
            FrameDesc *fdesc = AllocateFrameDesc(buffer, requestBytes);
            FrameDesc *fdescEc = AllocateFrameDesc((char *)(u->bufferEc), u->requestBytesEc);
            u->sourceAdapter->CapturerSourceFrameWithEc(u->sourceAdapter->wapper,
                fdesc, replyBytes, fdescEc, &replyBytesEc);
            FreeFrameDesc(fdesc);
            FreeFrameDesc(fdescEc);
        } else if (u->ecType == EC_DIFFERENT_ADAPTER) {
            u->sourceAdapter->CapturerSourceFrame(
                u->sourceAdapter->wapper, buffer, requestBytes, replyBytes);

            if (u->captureHandleEc != NULL) {
                FrameDesc *fdesc = AllocateFrameDesc(NULL, 0);
                FrameDesc *fdescEc = AllocateFrameDesc((char *)(u->bufferEc), u->requestBytesEc);
                uint64_t replyBytesUnused = 0;
                u->captureHandleEc->CaptureFrameWithEc(u->captureHandleEc->capture,
                    fdesc, &replyBytesUnused, fdescEc, &replyBytesEc);
                FreeFrameDesc(fdesc);
                FreeFrameDesc(fdescEc);
            }
        } else {
            AUDIO_WARNING_LOG("should not be here");
        }
    }

    uint64_t replyBytesMicRef = 0;
    if (u->micRef == REF_ON) {
        u->captureHandleMicRef->CaptureFrame(
            u->captureHandleMicRef->capture,
            (char *)(u->bufferMicRef), u->requestBytesMicRef, &replyBytesMicRef);
    }

    // handle ec & mic ref buffer and reply here

    return 0;
}

static int GetCapturerFrameFromHdi(pa_memchunk *chunk, const struct Userdata *u)
{
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 0;

    void *p = NULL;

    pa_assert(chunk->memblock);
    p = pa_memblock_acquire(chunk->memblock);
    pa_assert(p);

    requestBytes = pa_memblock_get_length(chunk->memblock);

    HandleCaptureFrame(u, (char *)p, requestBytes, &replyBytes);

    pa_memblock_release(chunk->memblock);

    AUDIO_DEBUG_LOG("HDI Source: request bytes: %{public}" PRIu64 ", replyBytes: %{public}" PRIu64,
            requestBytes, replyBytes);

    if (replyBytes > requestBytes) {
        AUDIO_ERR_LOG("HDI Source: Error replyBytes > requestBytes. Requested data Length: "
                "%{public}" PRIu64 ", Read: %{public}" PRIu64 " bytes", requestBytes, replyBytes);
        pa_memblock_unref(chunk->memblock);
        return -1;
    }

    if (replyBytes == 0) {
        AUDIO_ERR_LOG("HDI Source: Failed to read, Requested data Length: %{public}" PRIu64 " bytes,"
                " Read: %{public}" PRIu64 " bytes", requestBytes, replyBytes);
        pa_memblock_unref(chunk->memblock);
        return -1;
    }

    chunk->index = 0;
    chunk->length = replyBytes;

    return 0;
}

static int32_t SampleAlignment(const char *sceneKey, pa_memchunk *enhanceChunk, pa_memchunk *rChunk, struct Userdata *u)
{
    CHECK_AND_RETURN_RET_LOG(sceneKey != NULL, ERROR, "sceneKey is null");
    CHECK_AND_RETURN_RET_LOG(enhanceChunk != NULL, ERROR, "enhanceChunk is null");
    CHECK_AND_RETURN_RET_LOG(u != NULL, ERROR, "Userdata is null");

    pa_resampler *resampler = (pa_resampler *)pa_hashmap_get(u->sceneToResamplerMap, sceneKey);
    if (resampler != NULL) {
        pa_resampler_run(resampler, enhanceChunk, rChunk);
    } else {
        *rChunk = *enhanceChunk;
        pa_memblock_ref(rChunk->memblock);
    }
    return SUCCESS;
}

static int32_t GetCapturerFrameFromHdiAndProcess(pa_memchunk *chunk, struct Userdata *u)
{
    // new chunks
    chunk->length = u->bufferSize;
    AUDIO_DEBUG_LOG("HDI Source: chunk.length = u->bufferSize: %{public}zu", chunk->length);
    chunk->memblock = pa_memblock_new(u->core->mempool, chunk->length);

    if (GetCapturerFrameFromHdi(chunk, u) != 0) {
        return -1;
    }

    bool ret = EnhanceChainManagerIsEmptyEnhanceChain();
    if (ret) {
        // if none enhance chain exist, post data as the original method
        pa_source_post(u->source, chunk);
        pa_memblock_unref(chunk->memblock);
        return 0;
    }

    PostDataBypass(u->source, chunk);

    void *state = NULL;
    uint32_t *sceneKeyNum;
    const void *sceneKey;
    while ((sceneKeyNum = pa_hashmap_iterate(u->sceneToCountMap, &state, &sceneKey))) {
        uint32_t sceneKeyCode = (uint32_t)std::strtoul((char *)sceneKey, NULL, BASE_TEN);
        AUDIO_DEBUG_LOG("Now sceneKeyCode is : %{public}u", sceneKeyCode);

        pa_memchunk enhanceChunk, rChunk;
        enhanceChunk.length = chunk->length;
        enhanceChunk.memblock = pa_memblock_new(u->core->mempool, enhanceChunk.length);
        pa_memchunk_memcpy(&enhanceChunk, chunk);
        SampleAlignment((char *)sceneKey, &enhanceChunk, &rChunk, u);
        EnhanceProcessAndPost(u, sceneKeyCode, &rChunk);
        pa_memblock_unref(enhanceChunk.memblock);
        if (rChunk.memblock) {
            pa_memblock_unref(rChunk.memblock);
        }
    }
    pa_memblock_unref(chunk->memblock);
    
    return 0;
}

static bool PaRtpollSetTimerFunc(struct Userdata *u, bool timerElapsed)
{
    bool flag = (u->attrs.sourceType == SOURCE_TYPE_WAKEUP) ?
        (u->source->thread_info.state == PA_SOURCE_RUNNING && u->isCapturerStarted) :
        (PA_SOURCE_IS_OPENED(u->source->thread_info.state) && u->isCapturerStarted);
    if (!flag) {
        pa_rtpoll_set_timer_disabled(u->rtpoll);
        AUDIO_DEBUG_LOG("HDI Source: pa_rtpoll_set_timer_disabled done ");
        return true;
    }
    pa_memchunk chunk;
    pa_usec_t now;

    now = pa_rtclock_now();
    AUDIO_DEBUG_LOG("HDI Source: now: %{public}" PRIu64 " timerElapsed: %{public}d", now, timerElapsed);

    if (timerElapsed) {
        chunk.length = pa_usec_to_bytes(now - u->timestamp, &u->source->sample_spec);
        if (chunk.length > 0) {
            int ret = GetCapturerFrameFromHdiAndProcess(&chunk, u);
            if (ret != 0) {
                return false;
            }

            u->timestamp += pa_bytes_to_usec(chunk.length, &u->source->sample_spec);
            AUDIO_DEBUG_LOG("HDI Source: new u->timestamp : %{public}" PRIu64, u->timestamp);
        }
    }

    int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE];
    size_t count = 0;
    void *state = NULL;
    pa_source_output *sourceOutput;
    while ((sourceOutput = pa_hashmap_iterate(u->source->thread_info.outputs, &state, NULL))) {
        const char *cstringClientUid = pa_proplist_gets(sourceOutput->proplist, "stream.client.uid");
        if (cstringClientUid && (sourceOutput->state == PA_SOURCE_OUTPUT_RUNNING)) {
            appsUid[count++] = atoi(cstringClientUid);
        }
    }

    if (u->sourceAdapter) {
        u->sourceAdapter->CapturerSourceAppsUid(u->sourceAdapter->wapper, appsUid, count);
    }

    pa_usec_t costTime = pa_rtclock_now() - now;
    if (costTime > u->blockUsec) {
        u->delayTime += (costTime - u->blockUsec);
    }

    pa_rtpoll_set_timer_absolute(u->rtpoll, u->timestamp + u->blockUsec + u->delayTime);
    return true;
}

static void ThreadFuncCapturerTimer(void *userdata)
{
    struct Userdata *u = userdata;
    bool timerElapsed = false;

    pa_assert(u);

    if (u->core->realtime_scheduling) {
        pa_thread_make_realtime(u->core->realtime_priority);
    }

    pa_thread_mq_install(&u->threadMq);
    u->timestamp = pa_rtclock_now();

    if (u->attrs.sourceType == SOURCE_TYPE_WAKEUP) {
        u->timestamp -= HDI_WAKEUP_BUFFER_TIME;
    }

    AUDIO_DEBUG_LOG("HDI Source: u->timestamp : %{public}" PRIu64, u->timestamp);

    while (true) {
        AUTO_CTRACE("FuncCapturerLoop");
        bool result = PaRtpollSetTimerFunc(u, timerElapsed);
        if (!result) {
            break;
        }
        /* Hmm, nothing to do. Let's sleep */
        int ret = pa_rtpoll_run(u->rtpoll);
        if (ret < 0) {
            /* If this was no regular exit from the loop we have to continue
            * processing messages until we received PA_MESSAGE_SHUTDOWN */
            AUDIO_ERR_LOG("HDI Source: pa_rtpoll_run ret:%{public}d failed", ret);
            pa_asyncmsgq_post(u->threadMq.outq, PA_MSGOBJECT(u->core), PA_CORE_MESSAGE_UNLOAD_MODULE, u->module,
                0, NULL, NULL);
            pa_asyncmsgq_wait_for(u->threadMq.inq, PA_MESSAGE_SHUTDOWN);
            return;
        }

        timerElapsed = pa_rtpoll_timer_elapsed(u->rtpoll);

        if (ret == 0) {
            return;
        }
    }
}

static int PaHdiCapturerInit(struct Userdata *u)
{
    int ret;
    ret = u->sourceAdapter->CapturerSourceInit(u->sourceAdapter->wapper, &u->attrs);
    if (ret != 0) {
        AUDIO_ERR_LOG("Audio capturer init failed!");
        return ret;
    }

    u->capturerId = 0;
    u->rendererId = 0;
    ret = u->sourceAdapter->CapturerSourceGetCaptureId(u->sourceAdapter->wapper, &u->capturerId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Audio capturer get capturer id failed!");
        return ret;
    }

    InitAuxCapture(u);

    // No start test for remote device.
    if (strcmp(GetDeviceClass(u->sourceAdapter->deviceClass), DEVICE_CLASS_REMOTE)) {
        ret = u->sourceAdapter->CapturerSourceStart(u->sourceAdapter->wapper);
        if (ret != 0) {
            AUDIO_ERR_LOG("Audio capturer start failed!");
            goto fail;
        }
    }

    u->isCapturerStarted = true;
    return ret;

fail:
    PaHdiCapturerExit(u);
    return ret;
}

static void PaHdiCapturerExit(struct Userdata *u)
{
    u->sourceAdapter->CapturerSourceStop(u->sourceAdapter->wapper);
    u->sourceAdapter->CapturerSourceDeInit(u->sourceAdapter->wapper);
    StopAuxCapture(u);
    DeinitAuxCapture(u);
}

static int PaSetSourceProperties(pa_module *m, pa_modargs *ma, const pa_sample_spec *ss, const pa_channel_map *map,
    struct Userdata *u)
{
    pa_source_new_data data;

    pa_source_new_data_init(&data);
    data.driver = __FILE__;
    data.module = m;

    // if sourcetype is wakeup, source suspend after init
    if (u->attrs.sourceType == SOURCE_TYPE_WAKEUP) {
        data.suspend_cause = PA_SUSPEND_IDLE;
    }

    pa_source_new_data_set_name(&data, pa_modargs_get_value(ma, "source_name", DEFAULT_SOURCE_NAME));
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING,
        (u->attrs.adapterName ? u->attrs.adapterName : DEFAULT_AUDIO_DEVICE_NAME));
    pa_proplist_setf(data.proplist, PA_PROP_DEVICE_DESCRIPTION, "HDI source is %s",
        (u->attrs.adapterName ? u->attrs.adapterName : DEFAULT_AUDIO_DEVICE_NAME));
    pa_source_new_data_set_sample_spec(&data, ss);
    pa_source_new_data_set_channel_map(&data, map);
    pa_proplist_setf(data.proplist, PA_PROP_DEVICE_BUFFERING_BUFFER_SIZE, "%lu", (unsigned long)u->bufferSize);

    // set suspend on idle timeout to 0s
    pa_proplist_setf(data.proplist, "module-suspend-on-idle.timeout", "%d", 0);

    if (pa_modargs_get_proplist(ma, "source_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        AUDIO_ERR_LOG("Invalid properties");
        pa_source_new_data_done(&data);
        return -1;
    }

    u->source = pa_source_new(m->core, &data, PA_SOURCE_HARDWARE | PA_SOURCE_LATENCY);
    pa_source_new_data_done(&data);

    if (!u->source) {
        AUDIO_ERR_LOG("Failed to create source object");
        return -1;
    }

    u->source->parent.process_msg = SourceProcessMsg;
    u->source->set_state_in_io_thread = SourceSetStateInIoThreadCb;
    u->source->userdata = u;

    pa_source_set_asyncmsgq(u->source, u->threadMq.inq);
    pa_source_set_rtpoll(u->source, u->rtpoll);

    u->blockUsec = pa_bytes_to_usec(u->bufferSize, &u->source->sample_spec);
    pa_source_set_latency_range(u->source, 0, u->blockUsec);
    u->source->thread_info.max_rewind = pa_usec_to_bytes(u->blockUsec, &u->source->sample_spec);

    return 0;
}

static enum HdiAdapterFormat ConvertPaToHdiAdapterFormat(pa_sample_format_t format)
{
    enum HdiAdapterFormat adapterFormat;
    switch (format) {
        case PA_SAMPLE_U8:
            adapterFormat = SAMPLE_U8;
            break;
        case PA_SAMPLE_S16LE:
        case PA_SAMPLE_S16BE:
            adapterFormat = SAMPLE_S16;
            break;
        case PA_SAMPLE_S24LE:
        case PA_SAMPLE_S24BE:
            adapterFormat = SAMPLE_S24;
            break;
        case PA_SAMPLE_S32LE:
        case PA_SAMPLE_S32BE:
            adapterFormat = SAMPLE_S32;
            break;
        default:
            adapterFormat = SAMPLE_S16;
            break;
    }

    return adapterFormat;
}

static bool GetEndianInfo(pa_sample_format_t format)
{
    bool isBigEndian = false;
    switch (format) {
        case PA_SAMPLE_S16BE:
        case PA_SAMPLE_S24BE:
        case PA_SAMPLE_S32BE:
        case PA_SAMPLE_FLOAT32BE:
        case PA_SAMPLE_S24_32BE:
            isBigEndian = true;
            break;
        default:
            isBigEndian = false;
            break;
    }

    return isBigEndian;
}

static void InitUserdataAttrs(pa_modargs *ma, struct Userdata *u, const pa_sample_spec *ss)
{
    if (pa_modargs_get_value_s32(ma, "source_type", &u->attrs.sourceType) < 0) {
        AUDIO_ERR_LOG("Failed to parse source_type argument");
    }

    if (pa_modargs_get_value_u32(ma, "buffer_size", &u->bufferSize) < 0) {
        AUDIO_ERR_LOG("Failed to parse buffer_size argument.");
        u->bufferSize = DEFAULT_BUFFER_SIZE;
    }
    u->attrs.bufferSize = u->bufferSize;

    u->attrs.sampleRate = ss->rate;
    u->attrs.filePath = pa_modargs_get_value(ma, "file_path", "");
    if (pa_modargs_get_value_u32(ma, "open_mic_speaker", &u->openMicSpeaker) < 0) {
        AUDIO_ERR_LOG("Failed to parse open_mic_speaker argument");
    }
    u->attrs.channel = ss->channels;
    u->attrs.format = ConvertPaToHdiAdapterFormat(ss->format);
    u->attrs.isBigEndian = GetEndianInfo(ss->format);
    u->attrs.adapterName = pa_modargs_get_value(ma, "adapter_name", DEFAULT_DEVICE_CLASS);
    u->attrs.deviceNetworkId = pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID);
    if (pa_modargs_get_value_s32(ma, "device_type", &u->attrs.deviceType) < 0) {
        AUDIO_ERR_LOG("Failed to parse deviceType argument");
    }

    AUDIO_DEBUG_LOG("AudioDeviceCreateCapture format: %{public}d, isBigEndian: %{public}d channel: %{public}d,"
        "sampleRate: %{public}d", u->attrs.format, u->attrs.isBigEndian, u->attrs.channel, u->attrs.sampleRate);

    u->attrs.openMicSpeaker = u->openMicSpeaker;

    u->sceneToCountMap = pa_hashmap_new_full(pa_idxset_trivial_hash_func, pa_idxset_trivial_compare_func,
        pa_xfree, pa_xfree);

    u->sceneToResamplerMap = pa_hashmap_new_full(pa_idxset_string_hash_func, pa_idxset_string_compare_func,
        NULL, (pa_free_cb_t) pa_resampler_free);

    // get ec config into attrs
}

static void InitEcAttr(struct Userdata *u, CaptureAttr *attr)
{
    // set attr for different adapter ec
    attr->sourceType = SOURCE_TYPE_EC;
    // device attrs
    attr->adapterName = "primary";
    attr->deviceType = DEVICE_TYPE_MIC; // not needed, updateAudioRoute later
    // common audio attrs
    attr->sampleRate = u->attrs.sampleRate;
    attr->channelCount = u->attrs.channel;
    attr->format = u->attrs.format;
    attr->isBigEndian = u->attrs.isBigEndian;
}

static void InitMicRefAttr(struct Userdata *u, CaptureAttr *attr)
{
    // set attr for mic ref
    attr->sourceType = SOURCE_TYPE_MIC_REF;
    // device attrs
    attr->adapterName = "primary";
    attr->deviceType = DEVICE_TYPE_MIC;
    // common audio attrs
    attr->sampleRate = u->attrs.sampleRate;
    attr->channelCount = u->attrs.channel;
    attr->format = u->attrs.format;
    attr->isBigEndian = u->attrs.isBigEndian;
}

static void InitEcAndMicRefAttrs(pa_modargs *ma, struct Userdata *u)
{
    if (pa_modargs_get_value_u32(ma, "ec_type", &u->ecType) < 0) {
        u->ecType = EC_NONE;
    }
    u->ecAdapaterName = pa_modargs_get_value(ma, "ec_adapter", "");
    if (pa_modargs_get_value_u32(ma, "ec_sampling_rate", &u->ecSamplingRate) < 0) {
        u->ecSamplingRate = 0;
    }
    const char *ecFormatStr = pa_modargs_get_value(ma, "ec_format", "");
    u->ecFormat = pa_parse_sample_format(ecFormatStr);
    if (pa_modargs_get_value_u32(ma, "ec_channels", &u->ecChannels) < 0) {
        u->ecChannels = 0;
    }
    if (pa_modargs_get_value_u32(ma, "open_mic_ref", &u->micRef) < 0) {
        u->micRef = REF_OFF;
    }
    if (pa_modargs_get_value_u32(ma, "mic_ref_rate", &u->micRefRate) < 0) {
        u->micRefRate = 0;
    }
    const char *micRefFormatStr = pa_modargs_get_value(ma, "mic_ref_format", "");
    u->micRefFormat = pa_parse_sample_format(micRefFormatStr);
    if (pa_modargs_get_value_u32(ma, "mic_ref_channels", &u->micRefChannels) < 0) {
        u->micRefChannels = 0;
    }
}

static void CreateEcCapture(struct Userdata *u)
{
    if (u->ecType == EC_NONE) {
        u->captureHandleEc = NULL;
        u->bufferEc = NULL;
        u->requestBytesEc = 0;
        return;
    }

    // allocate ec buffer and decide request length, both same and different adapter needed
    u->bufferEc = NULL;
    u->requestBytesEc = 0;

    // only ec different adapter need create aux capture
    if (u->ecType == EC_DIFFERENT_ADAPTER) {
        CaptureAttr *attr = (struct CaptureAttr *)calloc(1, sizeof(CaptureAttr));
        if (attr == NULL) {
            AUDIO_ERR_LOG("capture attr allocate failed");
            return;
        }
        InitEcAttr(u, attr);
        int32_t res = CreateCaptureHandle(&u->captureHandleEc, attr);
        if (res) {
            AUDIO_ERR_LOG("create ec handle failed");
            free(attr);
            return;
        }
    }
}

static void CreateMicRefCapture(struct Userdata *u)
{
    if (u->micRef != REF_ON) {
        u->captureHandleMicRef = NULL;
        u->bufferMicRef = NULL;
        u->requestBytesMicRef = 0;
        return;
    }

    // allocate mic ref buffer and decide request length
    u->bufferMicRef = NULL;
    u->requestBytesMicRef = 0;

    CaptureAttr *attr = (struct CaptureAttr *)calloc(1, sizeof(CaptureAttr));
    if (attr == NULL) {
        AUDIO_ERR_LOG("capture attr allocate failed");
        return;
    }

    InitMicRefAttr(u, attr);
    int32_t res = CreateCaptureHandle(&u->captureHandleMicRef, attr);
    if (res) {
        AUDIO_ERR_LOG("create mic ref handle failed");
        free(attr);
        return;
    }
}

pa_source *PaHdiSourceNew(pa_module *m, pa_modargs *ma, const char *driver)
{
    int ret;

    pa_assert(m);
    pa_assert(ma);

    pa_sample_spec ss = m->core->default_sample_spec;
    pa_channel_map map = m->core->default_channel_map;

    /* Override with modargs if provided */
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        AUDIO_ERR_LOG("Failed to parse sample specification and channel map");
        return NULL;
    }

    struct Userdata *u = pa_xnew0(struct Userdata, 1);

    u->core = m->core;
    u->module = m;
    u->rtpoll = pa_rtpoll_new();

    if (pa_thread_mq_init(&u->threadMq, m->core->mainloop, u->rtpoll) < 0) {
        AUDIO_ERR_LOG("pa_thread_mq_init() failed.");
        goto fail;
    }

    InitUserdataAttrs(ma, u, &ss);

    InitEcAndMicRefAttrs(ma, u);

    ret = LoadSourceAdapter(pa_modargs_get_value(ma, "device_class", DEFAULT_DEVICE_CLASS),
        pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID), u->attrs.sourceType,
        pa_modargs_get_value(ma, "source_name", DEFAULT_SOURCE_NAME), &u->sourceAdapter);
    if (ret) {
        AUDIO_ERR_LOG("Load adapter failed");
        goto fail;
    }

    CreateEcCapture(u);
    CreateMicRefCapture(u);

    if (PaSetSourceProperties(m, ma, &ss, &map, u) != 0) {
        AUDIO_ERR_LOG("Failed to PaSetSourceProperties");
        goto fail;
    }

    if (PaHdiCapturerInit(u) != 0) {
        AUDIO_ERR_LOG("Failed to PaHdiCapturerInit");
        goto fail;
    }

    if (!(u->thread = pa_thread_new("OS_ReadHdi", ThreadFuncCapturerTimer, u))) {
        AUDIO_ERR_LOG("Failed to create hdi-source-record thread!");
        goto fail;
    }

    pa_source_put(u->source);
    return u->source;

fail:

    if (u->isCapturerStarted) {
        PaHdiCapturerExit(u);
    }
    UserdataFree(u);

    return NULL;
}

void PaHdiSourceFree(pa_source *s)
{
    AUTO_CTRACE("PaHdiSourceFree");
    struct Userdata *u = NULL;
    pa_source_assert_ref(s);
    pa_assert_se(u = s->userdata);
    UserdataFree(u);
}
