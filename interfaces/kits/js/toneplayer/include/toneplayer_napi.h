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

#ifndef TONE_PLAYER_NAPI_H_
#define TONE_PLAYER_NAPI_H_

#include <map>


#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "tone_player.h"
namespace OHOS {
namespace AudioStandard {
static const std::string TONE_PLAYER_NAPI_CLASS_NAME = "TonePlayer";

static const std::map<std::string, ToneType> toneTypeMap = {
    {"TONE_TYPE_DIAL_0", TONE_TYPE_DIAL_0},
    {"TONE_TYPE_DIAL_1", TONE_TYPE_DIAL_1},
    {"TONE_TYPE_DIAL_2", TONE_TYPE_DIAL_2},
    {"TONE_TYPE_DIAL_3", TONE_TYPE_DIAL_3},
    {"TONE_TYPE_DIAL_4", TONE_TYPE_DIAL_4},
    {"TONE_TYPE_DIAL_5", TONE_TYPE_DIAL_5},
    {"TONE_TYPE_DIAL_6", TONE_TYPE_DIAL_6},
    {"TONE_TYPE_DIAL_7", TONE_TYPE_DIAL_7},
    {"TONE_TYPE_DIAL_8", TONE_TYPE_DIAL_8},
    {"TONE_TYPE_DIAL_9", TONE_TYPE_DIAL_9},
    {"TONE_TYPE_DIAL_S", TONE_TYPE_DIAL_S},
    {"TONE_TYPE_DIAL_P", TONE_TYPE_DIAL_P},
    {"TONE_TYPE_DIAL_A", TONE_TYPE_DIAL_A},
    {"TONE_TYPE_DIAL_B", TONE_TYPE_DIAL_B},
    {"TONE_TYPE_DIAL_C", TONE_TYPE_DIAL_C},
    {"TONE_TYPE_DIAL_D", TONE_TYPE_DIAL_D},
    {"TONE_TYPE_DIAL_P", TONE_TYPE_DIAL_P},
    {"TONE_TYPE_DIAL_A", TONE_TYPE_DIAL_A},
    {"TONE_TYPE_DIAL_B", TONE_TYPE_DIAL_B},
    {"TONE_TYPE_DIAL_C", TONE_TYPE_DIAL_C},
    {"TONE_TYPE_DIAL_D", TONE_TYPE_DIAL_D},
    {"TONE_TYPE_DIAL_P", TONE_TYPE_DIAL_P},
};

class TonePlayerNapi {
public:
    TonePlayerNapi();
    ~TonePlayerNapi();
    static napi_value Init(napi_env env, napi_value exports);
private:
    struct TonePlayerAsyncContext {
        napi_env env;
        napi_async_work work;
        napi_deferred deferred;
        napi_ref callbackRef = nullptr;
        bool isTrue;
        int32_t intValue;
        TonePlayerNapi *objectInfo;
        AudioRendererInfo rendererInfo;
        int32_t toneType;
        int32_t status;
    };
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static bool ParseRendererInfo(napi_env env, napi_value root, AudioRendererInfo *rendererInfo);
    static napi_value CreateTonePlayer(napi_env env, napi_callback_info info);
    static napi_value Load(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value Release(napi_env env, napi_callback_info info);
    static napi_status AddNamedProperty(napi_env env, napi_value object, const std::string name, int32_t enumValue);
    static napi_value CreateTonePlayerWrapper(napi_env env, std::unique_ptr<AudioRendererInfo> &rendererInfo);
    static napi_value CreateToneTypeObject(napi_env env);
    static void CommonCallbackRoutine(napi_env env, TonePlayerAsyncContext* &asyncContext,
                                              const napi_value &valueParam);
    static void VoidAsyncCallbackComplete(napi_env env, napi_status status, void *data);
    static void GetTonePlayerAsyncCallbackComplete(napi_env env, napi_status status, void *data);
    static std::unique_ptr<AudioRendererInfo> sRendererInfo_;
    static napi_ref toneType_;
    napi_env env_;
    napi_ref wrapper_;
    std::shared_ptr<TonePlayer> tonePlayer_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* TONE_PLAYER_NAPI_H_ */