/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEffectHdiParam"

#include "audio_effect.h"
#include "audio_effect_hdi_param.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
AudioEffectHdiParam::AudioEffectHdiParam()
{
    AUDIO_DEBUG_LOG("constructor.");
    DeviceTypeToHdiControlMap_.clear();
    int32_t ret = memset_s(static_cast<void *>(input_), sizeof(input_), 0, sizeof(input_));
    if (ret != 0) {
        AUDIO_ERR_LOG("hdi constructor memset input failed");
    }
    ret = memset_s(static_cast<void *>(output_), sizeof(output_), 0, sizeof(output_));
    if (ret != 0) {
        AUDIO_ERR_LOG("hdi constructor memset output failed");
    }
    replyLen_ = GET_HDI_BUFFER_LEN;
    hdiModel_ = nullptr;
}

AudioEffectHdiParam::~AudioEffectHdiParam()
{
    AUDIO_DEBUG_LOG("destructor!");
}

void AudioEffectHdiParam::CreateHdiControl()
{
    for (const auto &item : HDI_EFFECT_LIB_MAP) {
        libName_ = strdup(item.second[0].c_str());
        effectId_ = strdup(item.second[1].c_str());
        EffectInfo info = {
            .libName = &libName_[0],
            .effectId = &effectId_[0],
            .ioDirection = 1,
        };
        ControllerId controllerId;
        IEffectControl *hdiControl = nullptr;
        int32_t ret = hdiModel_->CreateEffectController(hdiModel_, &info, &hdiControl, &controllerId);
        if ((ret != 0) || (hdiControl == nullptr)) {
            AUDIO_WARNING_LOG("hdi init failed");
        } else {
            DeviceTypeToHdiControlMap_.emplace(item.first, hdiControl);
        }
    }
    return;
}

void AudioEffectHdiParam::InitHdi()
{
    hdiModel_ = IEffectModelGet(false);
    if (hdiModel_ == nullptr) {
        AUDIO_ERR_LOG("IEffectModelGet failed");
        return;
    }

    CreateHdiControl();
}

int32_t AudioEffectHdiParam::SetHdiCommand(IEffectControl *hdiControl, int8_t *effectHdiInput)
{
    int32_t ret;
    if (hdiControl == nullptr) {
        AUDIO_WARNING_LOG("hdiControl is nullptr.");
        continue;
    }
    ret = memcpy_s(static_cast<void *>(input_), sizeof(input_),
        static_cast<void *>(effectHdiInput), sizeof(input_));
    CHECK_AND_CONTINUE_LOG(ret == 0, "hdi memcpy failed");
    uint32_t replyLen = GET_HDI_BUFFER_LEN;
    ret = hdiControl->SendCommand(hdiControl, HDI_SET_PATAM, input_, SEND_HDI_COMMAND_LEN,
        output_, &replyLen);
    return ret;
}

int32_t AudioEffectHdiParam::UpdateHdiState(int8_t *effectHdiInput)
{
    int32_t ret;
    for (const auto &item : DeviceTypeToHdiControlMap_) {
        IEffectControl *hdiControl = item.second;
        ret = SetHdiCommand(hdiControl, effectHdiInput);
        CHECK_AND_CONTINUE_LOG(ret == 0, "hdi send command failed");
    }
    return ret;
}

int32_t AudioEffectHdiParam::UpdateHdiState(int8_t *effectHdiInput, DeviceType deviceType)
{
    int32_t ret;
    IEffectControl *hdiControl = DeviceTypeToHdiControlMap_[deviceType];
    ret = SetHdiCommand(hdiControl, effectHdiInput);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "hdi send command failed");
    return ret;
}
}  // namespace AudioStandard
}  // namespace OHOS