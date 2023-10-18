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

#ifndef NAPI_AUDIO_RENDERER_H_
#define NAPI_AUDIO_RENDERER_H_

#include <iostream>
#include <map>
#include <queue>

#include "audio_renderer.h"
#include "audio_errors.h"
#include "audio_system_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "audio_stream_manager.h"
#include "napi_param_utils.h"
#include "napi_async_work.h"
#include "napi_audio_error.h"

namespace OHOS {
namespace AudioStandard {
using namespace HiviewDFX;
using namespace std;
static const std::string NAPI_AUDIO_RENDERER_CLASS_NAME = "AudioRenderer";
namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NapiAudioRenderer"};
}
class NapiAudioRenderer {
public:
    NapiAudioRenderer();
    ~NapiAudioRenderer() = default;

    enum AudioSampleFormat {
        SAMPLE_FORMAT_INVALID = -1,
        SAMPLE_FORMAT_U8 = 0,
        SAMPLE_FORMAT_S16LE = 1,
        SAMPLE_FORMAT_S24LE = 2,
        SAMPLE_FORMAT_S32LE = 3,
        SAMPLE_FORMAT_F32LE = 4
    };
    std::unique_ptr<AudioRenderer> audioRenderer_;
    static napi_value Init(napi_env env, napi_value exports);
private:
    struct AudioRendererAsyncContext : public ContextBase {
        int32_t intValue;
        int32_t audioRendererRate;
        int32_t rendererFlags;
        int32_t interruptMode;
        bool isTrue;
        uint64_t time;
        size_t bufferLen;
        size_t bufferSize;
        int32_t volType;
        double volLevel;
        uint32_t rendererSampleRate;
        uint32_t audioStreamId;
        size_t totalBytesWritten;
        uint32_t underflowCount;
        void *data;
        int32_t audioEffectMode;
        int32_t channelBlendMode;
        AudioSampleFormat sampleFormat;
        AudioSamplingRate samplingRate;
        AudioChannel channelCount;
        AudioEncodingType encodingType;
        ContentType contentType;
        StreamUsage usage;
        DeviceRole deviceRole;
        DeviceType deviceType;
        AudioRendererOptions rendererOptions;
        DeviceInfo deviceInfo;
    };

    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    static void CreateRendererFailed();
    static napi_value Construct(napi_env env, napi_callback_info info);
    static unique_ptr<NapiAudioRenderer> CreateAudioRendererNativeObject(napi_env env);
#if 0
    static napi_value CreateAudioRenderer(napi_env env, napi_callback_info info);
    static napi_value CreateAudioRendererSync(napi_env env, napi_callback_info info);
#endif
    static napi_value SetRenderRate(napi_env env, napi_callback_info info);
    static napi_value GetRenderRate(napi_env env, napi_callback_info info);

    /* common interface in AudioRendererNapi */
    static bool CheckContextStatus(std::shared_ptr<AudioRendererAsyncContext> context);
    static bool CheckAudioRendererStatus(NapiAudioRenderer *napi, std::shared_ptr<AudioRendererAsyncContext> context);

    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;

    ContentType contentType_;
    StreamUsage streamUsage_;
    // DeviceRole deviceRole_;
    // DeviceType deviceType_;
    // int32_t rendererFlags_ = 0; // default flag 0
    napi_env env_;
    // std::queue<napi_async_work> writeRequestQ_;
    std::atomic<bool> scheduleFromApiCall_;
    std::atomic<bool> doNotScheduleWrite_;
    std::atomic<bool> isDrainWriteQInProgress_;
    static constexpr double MIN_VOLUME_IN_DOUBLE = 0.0;
    static constexpr double MAX_VOLUME_IN_DOUBLE = 1.0;
    static std::unique_ptr<AudioRendererOptions> sRendererOptions_;
#if 0
    std::shared_ptr<AudioRendererCallback> callbackNapi_ = nullptr;
    std::shared_ptr<RendererPositionCallback> positionCBNapi_ = nullptr;
    std::shared_ptr<RendererPeriodPositionCallback> periodPositionCBNapi_ = nullptr;
    std::shared_ptr<AudioRendererWriteCallback> dataRequestCBNapi_ = nullptr;
    std::shared_ptr<AudioRendererDeviceChangeCallback> rendererDeviceChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioRendererPolicyServiceDiedCallback> rendererPolicyServiceDiedCallbackNapi_ = nullptr;
#endif
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_RENDERER_NAPI_H_ */
