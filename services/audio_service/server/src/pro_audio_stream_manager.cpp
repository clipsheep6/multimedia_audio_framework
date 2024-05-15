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
#define LOG_TAG "ProAudioStreamManager"

#include "pro_audio_stream_manager.h"
#include <sstream>
#include <atomic>
#include "audio_log.h"
#include "audio_errors.h"
#include "policy_handler.h"
#include "pro_renderer_stream_impl.h"
#include "audio_engine_manager.h"
#include "none_mix_engine.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

ProAudioStreamManager::ProAudioStreamManager(ManagerType type) : managerType_(type), playbackEngine_(nullptr)
{
    AUDIO_DEBUG_LOG("ProAudioStreamManager");
}

ProAudioStreamManager::~ProAudioStreamManager()
{
    AUDIO_DEBUG_LOG("~ProAudioStreamManager");
}

int32_t ProAudioStreamManager::CreateRender(AudioProcessConfig processConfig, std::shared_ptr<IRendererStream> &stream)
{
    AUDIO_DEBUG_LOG("Create renderer start,manager type:%{public}d", managerType_);
    uint32_t sessionId = PolicyHandler::GetInstance().GenerateSessionId(processConfig.appInfo.appUid);
    DeviceInfo deviceInfo;

    std::shared_ptr<IRendererStream> rendererStream = CreateRendererStream(processConfig);
    CHECK_AND_RETURN_RET_LOG(rendererStream != nullptr, ERR_DEVICE_INIT, "Failed to init rendererStream");

    rendererStream->SetStreamIndex(sessionId);
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    rendererStreamMap_[sessionId] = rendererStream;
    stream = rendererStream;
    return SUCCESS;
}

int32_t ProAudioStreamManager::StartRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Start renderer enter");
    std::shared_ptr<IRendererStream> currentRender;
    {
        std::lock_guard<std::mutex> lock(streamMapMutex_);
        auto it = rendererStreamMap_.find(streamIndex);
        if (it == rendererStreamMap_.end()) {
            AUDIO_WARNING_LOG("No matching stream");
            return SUCCESS;
        }
    }
    currentRender = rendererStreamMap_[streamIndex];
    DeviceInfo deviceInfo;
    AudioProcessConfig config = currentRender->GetAudioProcessConfig();
    bool ret = PolicyHandler::GetInstance().GetProcessDeviceInfo(config, deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_DEVICE_INIT, "GetProcessDeviceInfo failed.");
    int32_t result = currentRender->Start();
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Failed to start rendererStream");
    if (!playbackEngine_) {
        playbackEngine_ = std::make_unique<NoneMixEngine>(deviceInfo, managerType_ == VOIP_PLAYBACK);
        playbackEngine_->AddRenderer(currentRender);
    }
    playbackEngine_->Start();
    return SUCCESS;
}

int32_t ProAudioStreamManager::StopRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Stop renderer enter");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    rendererStreamMap_[streamIndex]->Stop();
    if (playbackEngine_) {
        playbackEngine_->Stop();
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::PauseRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Pause renderer enter");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    rendererStreamMap_[streamIndex]->Pause();
    if (playbackEngine_) {
        playbackEngine_->Pause();
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::ReleaseRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Release renderer start");
    std::shared_ptr<IRendererStream> currentRender;
    {
        std::lock_guard<std::mutex> lock(streamMapMutex_);
        auto it = rendererStreamMap_.find(streamIndex);
        if (it == rendererStreamMap_.end()) {
            AUDIO_WARNING_LOG("No matching stream");
            return SUCCESS;
        }
        currentRender = rendererStreamMap_[streamIndex];
        rendererStreamMap_[streamIndex] = nullptr;
        rendererStreamMap_.erase(streamIndex);
    }
    if (playbackEngine_) {
        playbackEngine_->Stop();
        playbackEngine_->RemoveRenderer(currentRender);
        playbackEngine_ = nullptr;
    }
    if (currentRender->Release() < 0) {
        AUDIO_WARNING_LOG("Release stream %{public}d failed", streamIndex);
        return ERR_OPERATION_FAILED;
    }
    AUDIO_INFO_LOG("rendererStreamMap_.size() : %{public}zu", rendererStreamMap_.size());
    if (rendererStreamMap_.size() == 0) {
        AUDIO_INFO_LOG("Release the last stream");
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::TriggerStartIfNecessary()
{
    if (playbackEngine_ && !playbackEngine_->IsPlaybackEngineRunning()) {
        playbackEngine_->Start();
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::GetStreamCount() const noexcept
{
    return rendererStreamMap_.size();
}

std::shared_ptr<IRendererStream> ProAudioStreamManager::CreateRendererStream(AudioProcessConfig processConfig)
{
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    std::shared_ptr<ProRendererStreamImpl> rendererStream =
        std::make_shared<ProRendererStreamImpl>(processConfig, managerType_ == DIRECT_PLAYBACK);
    if (rendererStream->InitParams() != SUCCESS) {
        AUDIO_ERR_LOG("Create rendererStream Failed");
        return nullptr;
    }
    return rendererStream;
}

int32_t ProAudioStreamManager::CreateCapturer(AudioProcessConfig processConfig,
                                              std::shared_ptr<ICapturerStream> &stream)
{
    AUDIO_ERR_LOG("Unsupported operation: CreateCapturer");
    return SUCCESS;
}

int32_t ProAudioStreamManager::ReleaseCapturer(uint32_t streamIndex)
{
    AUDIO_ERR_LOG("Unsupported operation: ReleaseCapturer");
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS