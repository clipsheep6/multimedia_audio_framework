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
#include "audio_policy_server_handler.h"
#include "audio_policy_service.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyServerHandler::AudioPolicyServerHandler() : AppExecFwk::EventHandler(
    AppExecFwk::EventRunner::Create("OS_APAsyncRunner"))
{
    AUDIO_DEBUG_LOG("AudioPolicyServerHandler created");
}

AudioPolicyServerHandler::~AudioPolicyServerHandler()
{
    AUDIO_WARNING_LOG("AudioPolicyServerHandler destroyed");
};

void AudioPolicyServerHandler::AddAudioPolicyClientProxyMap(int32_t clientPid, const sptr<IAudioPolicyClient>& cb)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    audioPolicyClientProxyAPSCbsMap_.emplace(clientPid, cb);
    AUDIO_INFO_LOG("AddAudioPolicyClientProxyMap, group data num [%{public}zu]",
        audioPolicyClientProxyAPSCbsMap_.size());
}

void AudioPolicyServerHandler::RemoveAudioPolicyClientProxyMap(pid_t clientPid)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    audioPolicyClientProxyAPSCbsMap_.erase(clientPid);
    AUDIO_INFO_LOG("RemoveAudioPolicyClientProxyMap, group data num [%{public}zu]",
        audioPolicyClientProxyAPSCbsMap_.size());
}

void AudioPolicyServerHandler::AddInterruptCbsMap(uint32_t sessionID,
    const std::shared_ptr<AudioInterruptCallback> &callback)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    interruptCbsMap_[sessionID] = callback;
    AUDIO_INFO_LOG("AddInterruptCbsMap, group data num [%{public}zu]", interruptCbsMap_.size());
}

int32_t AudioPolicyServerHandler::RemoveInterruptCbsMap(uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (interruptCbsMap_.erase(sessionID) == 0) {
        AUDIO_ERR_LOG("RemoveInterruptCbsMap session %{public}d not present", sessionID);
        return ERR_INVALID_OPERATION;
    }
    return SUCCESS;
}

void AudioPolicyServerHandler::AddExternInterruptCbsMap(int32_t clientId,
    const std::shared_ptr<AudioInterruptCallback> &callback)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    amInterruptCbsMap_[clientId] = callback;
    AUDIO_INFO_LOG("AddExternInterruptCbsMap, group data num [%{public}zu]",
        amInterruptCbsMap_.size());
}

int32_t AudioPolicyServerHandler::RemoveExternInterruptCbsMap(int32_t clientId)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (amInterruptCbsMap_.erase(clientId) == 0) {
        AUDIO_ERR_LOG("RemoveExternInterruptCbsMap client %{public}d not present", clientId);
        return ERR_INVALID_OPERATION;
    }
    return SUCCESS;
}

void AudioPolicyServerHandler::AddAvailableDeviceChangeMap(int32_t clientId, const AudioDeviceUsage usage,
    const sptr<IStandardAudioPolicyManagerListener> &callback)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    availableDeviceChangeCbsMap_[{clientId, usage}] = callback;
    AUDIO_INFO_LOG("AddAvailableDeviceChangeMap, group data num [%{public}zu]", availableDeviceChangeCbsMap_.size());
}

void AudioPolicyServerHandler::RemoveAvailableDeviceChangeMap(const int32_t clientId, AudioDeviceUsage usage)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (availableDeviceChangeCbsMap_.erase({clientId, usage}) == 0) {
        AUDIO_INFO_LOG("client not present in %{public}s", __func__);
    }
    // for routing manager napi remove all device change callback
    if (usage == AudioDeviceUsage::D_ALL_DEVICES) {
        for (auto it = availableDeviceChangeCbsMap_.begin(); it != availableDeviceChangeCbsMap_.end();) {
            if ((*it).first.first == clientId) {
                it = availableDeviceChangeCbsMap_.erase(it);
            } else {
                it++;
            }
        }
    }
    AUDIO_INFO_LOG("RemoveAvailableDeviceChangeMap, group data num [%{public}zu]",
        availableDeviceChangeCbsMap_.size());
}

void AudioPolicyServerHandler::AddDistributedRoutingRoleChangeCbsMap(int32_t clientId,
    const sptr<IStandardAudioRoutingManagerListener> &callback)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (callback != nullptr) {
        distributedRoutingRoleChangeCbsMap_[clientId] = callback;
    }
    AUDIO_DEBUG_LOG("SetDistributedRoutingRoleCallback: distributedRoutingRoleChangeCbsMap_ size: %{public}zu",
        distributedRoutingRoleChangeCbsMap_.size());
}

int32_t AudioPolicyServerHandler::RemoveDistributedRoutingRoleChangeCbsMap(int32_t clientId)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (distributedRoutingRoleChangeCbsMap_.erase(clientId) == 0) {
        AUDIO_ERR_LOG("RemoveDistributedRoutingRoleChangeCbsMap clientPid %{public}d not present", clientId);
        return ERR_INVALID_OPERATION;
    }

    AUDIO_DEBUG_LOG("UnsetDistributedRoutingRoleCallback: distributedRoutingRoleChangeCbsMap_ size: %{public}zu",
        distributedRoutingRoleChangeCbsMap_.size());
    return SUCCESS;
}

bool AudioPolicyServerHandler::SendDeviceChangedCallback(const vector<sptr<AudioDeviceDescriptor>> &desc,
    bool isConnected)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->deviceChangeAction.type = isConnected ? DeviceChangeType::CONNECT : DeviceChangeType::DISCONNECT;
    eventContextObj->deviceChangeAction.deviceDescriptors = desc;

    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::AUDIO_DEVICE_CHANGE, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendDeviceChangedCallback event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendAvailableDeviceChange(const vector<sptr<AudioDeviceDescriptor>> &desc,
    bool isConnected)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->deviceChangeAction.type = isConnected ? DeviceChangeType::CONNECT : DeviceChangeType::DISCONNECT;
    eventContextObj->deviceChangeAction.deviceDescriptors = desc;

    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::AVAILABLE_AUDIO_DEVICE_CHANGE,
        eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendAvailableDeviceChange event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendVolumeKeyEventCallback(const VolumeEvent &volumeEvent)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->volumeEvent = volumeEvent;
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::VOLUME_KEY_EVENT, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendVolumeKeyEventCallback event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendAudioFocusInfoChangeCallBack(int32_t callbackCategory,
    const AudioInterrupt &audioInterrupt, const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->audioInterrupt = audioInterrupt;
    eventContextObj->focusInfoList = focusInfoList;
    bool ret = false;

    lock_guard<mutex> runnerlock(runnerMutex_);
    if (callbackCategory == FocusCallbackCategory::REQUEST_CALLBACK_CATEGORY) {
        ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::REQUEST_CATEGORY_EVENT, eventContextObj));
        CHECK_AND_RETURN_RET_LOG(ret, ret, "Send REQUEST_CATEGORY_EVENT event failed");
    } else if (callbackCategory == FocusCallbackCategory::ABANDON_CALLBACK_CATEGORY) {
        ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::ABANDON_CATEGORY_EVENT, eventContextObj));
        CHECK_AND_RETURN_RET_LOG(ret, ret, "Send ABANDON_CATEGORY_EVENT event failed");
    }
    ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::FOCUS_INFOCHANGE, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "Send FOCUS_INFOCHANGE event failed");

    return ret;
}

bool AudioPolicyServerHandler::SendRingerModeUpdatedCallBack(const AudioRingerMode &ringMode)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->ringMode = ringMode;
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::RINGER_MODEUPDATE_EVENT, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "Send RINGER_MODEUPDATE_EVENT event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendMicStateUpdatedCallBack(const MicStateChangeEvent &micStateChangeEvent)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->micStateChangeEvent = micStateChangeEvent;
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::MIC_STATE_CHANGE_EVENT, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "Send MIC_STATE_CHANGE_EVENT event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendInterruptEventInternalCallBack(const InterruptEventInternal &interruptEvent)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->interruptEvent = interruptEvent;
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::INTERRUPT_EVENT, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "Send INTERRUPT_EVENT event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendInterruptEventWithSeesionIdCallBack(const InterruptEventInternal &interruptEvent,
    const uint32_t &sessionID)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->interruptEvent = interruptEvent;
    eventContextObj->sessionID = sessionID;
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::INTERRUPT_EVENT_WITH_SESSIONID,
        eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "Send INTERRUPT_EVENT_WITH_SESSIONID event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendInterruptEventWithClientIdCallBack(const InterruptEventInternal &interruptEvent,
    const int32_t &clientId)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->interruptEvent = interruptEvent;
    eventContextObj->clientId = clientId;
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::INTERRUPT_EVENT_WITH_CLIENTID,
        eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "Send INTERRUPT_EVENT_WITH_CLIENTID event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendPreferredOutputDeviceUpdated()
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::PREFERRED_OUTPUT_DEVICE_UPDATED));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendPreferredOutputDeviceUpdated event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendPreferredInputDeviceUpdated()
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::PREFERRED_INPUT_DEVICE_UPDATED));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendPreferredInputDeviceUpdated event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendDistributedRoutingRoleChange(const sptr<AudioDeviceDescriptor> &descriptor,
    const CastType &type)
{
    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->descriptor = descriptor;
    eventContextObj->type = type;
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::DISTRIBUTED_ROUTING_ROLE_CHANGE,
        eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendDistributedRoutingRoleChange event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendRendererInfoEvent(
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    for (const auto &changeInfo : audioRendererChangeInfos) {
        rendererChangeInfos.push_back(std::make_unique<AudioRendererChangeInfo>(*changeInfo));
    }

    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->audioRendererChangeInfos = move(rendererChangeInfos);

    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::RENDERER_INFO_EVENT,
        eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendRendererInfoEvent event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendCapturerInfoEvent(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    for (const auto &changeInfo : audioCapturerChangeInfos) {
        capturerChangeInfos.push_back(std::make_unique<AudioCapturerChangeInfo>(*changeInfo));
    }

    std::shared_ptr<EventContextObj> eventContextObj = std::make_shared<EventContextObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");
    eventContextObj->audioCapturerChangeInfos = move(capturerChangeInfos);

    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::CAPTURER_INFO_EVENT,
        eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendRendererInfoEvent event failed");
    return ret;
}

bool AudioPolicyServerHandler::SendRendererDeviceChangeEvent(const int32_t clientPid, const uint32_t sessionId,
    const DeviceInfo &outputDeviceInfo, const AudioStreamDeviceChangeReason reason)
{
    std::shared_ptr<RendererDeviceChangeEvent> eventContextObj = std::make_shared<RendererDeviceChangeEvent>(
        clientPid, sessionId, outputDeviceInfo, reason);
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "EventContextObj get nullptr");

    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAudioServerCmd::RENDERER_DEVICE_CHANGE_EVENT,
        eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendRendererDeviceChangeEvent event failed");
    return ret;
}

void AudioPolicyServerHandler::HandleDeviceChangedCallback(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        if (it->second && eventContextObj->deviceChangeAction.deviceDescriptors.size() > 0) {
            DeviceChangeAction deviceChangeAction = eventContextObj->deviceChangeAction;
            if (!(it->second->hasBTPermission_)) {
                AudioPolicyService::GetAudioPolicyService().
                    UpdateDescWhenNoBTPermission(deviceChangeAction.deviceDescriptors);
            }
            it->second->OnDeviceChange(deviceChangeAction);
        }
    }
}

void AudioPolicyServerHandler::HandleAvailableDeviceChange(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = availableDeviceChangeCbsMap_.begin(); it != availableDeviceChangeCbsMap_.end(); ++it) {
        AudioDeviceUsage usage = it->first.second;
        eventContextObj->deviceChangeAction.deviceDescriptors = AudioPolicyService::GetAudioPolicyService().
            DeviceFilterByUsage(it->first.second, eventContextObj->deviceChangeAction.deviceDescriptors);
        if (it->second && eventContextObj->deviceChangeAction.deviceDescriptors.size() > 0) {
            if (!(it->second->hasBTPermission_)) {
                AudioPolicyService::GetAudioPolicyService().
                    UpdateDescWhenNoBTPermission(eventContextObj->deviceChangeAction.deviceDescriptors);
            }
            it->second->OnAvailableDeviceChange(usage, eventContextObj->deviceChangeAction);
        }
    }
}

void AudioPolicyServerHandler::HandleVolumeKeyEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        sptr<IAudioPolicyClient> volumeChangeCb = it->second;
        if (volumeChangeCb == nullptr) {
            AUDIO_ERR_LOG("volumeChangeCb: nullptr for client : %{public}d", it->first);
            continue;
        }
        AUDIO_DEBUG_LOG("SetA2dpDeviceVolume trigger volumeChangeCb clientPid : %{public}d", it->first);
        volumeChangeCb->OnVolumeKeyEvent(eventContextObj->volumeEvent);
    }
}

void AudioPolicyServerHandler::HandleRequestCateGoryEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");

    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        it->second->OnAudioFocusRequested(eventContextObj->audioInterrupt);
    }
}

void AudioPolicyServerHandler::HandleAbandonCateGoryEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        it->second->OnAudioFocusAbandoned(eventContextObj->audioInterrupt);
    }
}

void AudioPolicyServerHandler::HandleFocusInfoChangeEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    AUDIO_INFO_LOG("HandleFocusInfoChangeEvent focusInfoList :%{public}zu", eventContextObj->focusInfoList.size());
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        it->second->OnAudioFocusInfoChange(eventContextObj->focusInfoList);
    }
}

void AudioPolicyServerHandler::HandleRingerModeUpdatedEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        sptr<IAudioPolicyClient> ringerModeListenerCb = it->second;
        if (ringerModeListenerCb == nullptr) {
            AUDIO_ERR_LOG("ringerModeListenerCb nullptr for client %{public}d", it->first);
            continue;
        }

        AUDIO_DEBUG_LOG("ringerModeListenerCb client %{public}d", it->first);
        ringerModeListenerCb->OnRingerModeUpdated(eventContextObj->ringMode);
    }
}

void AudioPolicyServerHandler::HandleMicStateUpdatedEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        sptr<IAudioPolicyClient> micStateChangeListenerCb = it->second;
        if (micStateChangeListenerCb == nullptr) {
            AUDIO_ERR_LOG("callback is nullptr for client %{public}d", it->first);
            continue;
        }
        micStateChangeListenerCb->OnMicStateUpdated(eventContextObj->micStateChangeEvent);
    }
}

void AudioPolicyServerHandler::HandleInterruptEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it : interruptCbsMap_) {
        if (it.second != nullptr) {
            it.second->OnInterrupt(eventContextObj->interruptEvent);
        }
    }
}

void AudioPolicyServerHandler::HandleInterruptEventWithSessionId(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");

    std::lock_guard<std::mutex> lock(runnerMutex_);
    std::shared_ptr<AudioInterruptCallback> policyListenerCb = interruptCbsMap_[eventContextObj->sessionID];
    CHECK_AND_RETURN_LOG(policyListenerCb != nullptr, "policyListenerCb get nullptr");
    policyListenerCb->OnInterrupt(eventContextObj->interruptEvent);
}

void AudioPolicyServerHandler::HandleInterruptEventWithClientId(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");

    std::lock_guard<std::mutex> lock(runnerMutex_);
    std::shared_ptr<AudioInterruptCallback> policyListenerCb = amInterruptCbsMap_[eventContextObj->clientId];
    CHECK_AND_RETURN_LOG(policyListenerCb != nullptr, "policyListenerCb get nullptr");
    policyListenerCb->OnInterrupt(eventContextObj->interruptEvent);
}

void AudioPolicyServerHandler::HandlePreferredOutputDeviceUpdated()
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        AudioRendererInfo rendererInfo;
        auto deviceDescs = AudioPolicyService::GetAudioPolicyService().
            GetPreferredOutputDeviceDescriptors(rendererInfo);
        if (!(it->second->hasBTPermission_)) {
            AudioPolicyService::GetAudioPolicyService().UpdateDescWhenNoBTPermission(deviceDescs);
        }
        it->second->OnPreferredOutputDeviceUpdated(deviceDescs);
    }
}

void AudioPolicyServerHandler::HandlePreferredInputDeviceUpdated()
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        AudioCapturerInfo captureInfo;
        auto deviceDescs = AudioPolicyService::GetAudioPolicyService().GetPreferredInputDeviceDescriptors(captureInfo);
        if (!(it->second->hasBTPermission_)) {
            AudioPolicyService::GetAudioPolicyService().UpdateDescWhenNoBTPermission(deviceDescs);
        }
        it->second->OnPreferredInputDeviceUpdated(deviceDescs);
    }
}

void AudioPolicyServerHandler::HandleDistributedRoutingRoleChangeEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = distributedRoutingRoleChangeCbsMap_.begin(); it != distributedRoutingRoleChangeCbsMap_.end(); it++) {
        it->second->OnDistributedRoutingRoleChange(eventContextObj->descriptor, eventContextObj->type);
    }
}

void AudioPolicyServerHandler::HandleRendererInfoEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        sptr<IAudioPolicyClient> rendererStateChangeCb = it->second;
        if (rendererStateChangeCb == nullptr) {
            AUDIO_ERR_LOG("rendererStateChangeCb : nullptr for client : %{public}d", it->first);
            continue;
        }
        rendererStateChangeCb->OnRendererStateChange(eventContextObj->audioRendererChangeInfos);
    }
}

void AudioPolicyServerHandler::HandleCapturerInfoEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<EventContextObj> eventContextObj = event->GetSharedObject<EventContextObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    for (auto it = audioPolicyClientProxyAPSCbsMap_.begin(); it != audioPolicyClientProxyAPSCbsMap_.end(); ++it) {
        sptr<IAudioPolicyClient> capturerStateChangeCb = it->second;
        if (capturerStateChangeCb == nullptr) {
            AUDIO_ERR_LOG("capturerStateChangeCb : nullptr for client : %{public}d", it->first);
            continue;
        }
        capturerStateChangeCb->OnCapturerStateChange(eventContextObj->audioCapturerChangeInfos);
    }
}

void AudioPolicyServerHandler::HandleRendererDeviceChangeEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<RendererDeviceChangeEvent> eventContextObj = event->GetSharedObject<RendererDeviceChangeEvent>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    const auto &[pid, sessionId, outputDeviceInfo, reason] = *eventContextObj;
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (audioPolicyClientProxyAPSCbsMap_.count(pid) == 0) {
        return;
    }
    sptr<IAudioPolicyClient> capturerStateChangeCb = audioPolicyClientProxyAPSCbsMap_.at(pid);
    if (capturerStateChangeCb == nullptr) {
        AUDIO_ERR_LOG("capturerStateChangeCb : nullptr for client : %{public}" PRId32 "", pid);
        return;
    }
    capturerStateChangeCb->OnRendererDeviceChange(sessionId, outputDeviceInfo, reason);
}

void AudioPolicyServerHandler::HandleServiceEvent(const uint32_t &eventId,
    const AppExecFwk::InnerEvent::Pointer &event)
{
    switch (eventId) {
        case EventAudioServerCmd::AUDIO_DEVICE_CHANGE:
            HandleDeviceChangedCallback(event);
            break;
        case EventAudioServerCmd::PREFERRED_OUTPUT_DEVICE_UPDATED:
            HandlePreferredOutputDeviceUpdated();
            break;
        case EventAudioServerCmd::PREFERRED_INPUT_DEVICE_UPDATED:
            HandlePreferredInputDeviceUpdated();
            break;
        case EventAudioServerCmd::AVAILABLE_AUDIO_DEVICE_CHANGE:
            HandleAvailableDeviceChange(event);
            break;
        case EventAudioServerCmd::RENDERER_INFO_EVENT:
            HandleRendererInfoEvent(event);
            break;
        case EventAudioServerCmd::CAPTURER_INFO_EVENT:
            HandleCapturerInfoEvent(event);
            break;
        case EventAudioServerCmd::RENDERER_DEVICE_CHANGE_EVENT:
            HandleRendererDeviceChangeEvent(event);
            break;
        default:
            break;
    }
}

void AudioPolicyServerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    AUDIO_DEBUG_LOG("handler process eventId:%{public}u", eventId);

    HandleServiceEvent(eventId, event);

    switch (eventId) {
        case EventAudioServerCmd::VOLUME_KEY_EVENT:
            HandleVolumeKeyEvent(event);
            break;
        case EventAudioServerCmd::REQUEST_CATEGORY_EVENT:
            HandleRequestCateGoryEvent(event);
            break;
        case EventAudioServerCmd::ABANDON_CATEGORY_EVENT:
            HandleAbandonCateGoryEvent(event);
            break;
        case EventAudioServerCmd::FOCUS_INFOCHANGE:
            HandleFocusInfoChangeEvent(event);
            break;
        case EventAudioServerCmd::RINGER_MODEUPDATE_EVENT:
            HandleRingerModeUpdatedEvent(event);
            break;
        case EventAudioServerCmd::MIC_STATE_CHANGE_EVENT:
            HandleMicStateUpdatedEvent(event);
            break;
        case EventAudioServerCmd::INTERRUPT_EVENT:
            HandleInterruptEvent(event);
            break;
        case EventAudioServerCmd::INTERRUPT_EVENT_WITH_SESSIONID:
            HandleInterruptEventWithSessionId(event);
            break;
        case EventAudioServerCmd::INTERRUPT_EVENT_WITH_CLIENTID:
            HandleInterruptEventWithClientId(event);
            break;
        case EventAudioServerCmd::DISTRIBUTED_ROUTING_ROLE_CHANGE:
            HandleDistributedRoutingRoleChangeEvent(event);
            break;
        default:
            break;
    }
}
} // namespace AudioStandard
} // namespace OHOS
