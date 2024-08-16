/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioSafeVolumeNotification"
#endif

#include "audio_safe_volume_notification.h"

#include <map>

#include "want_agent_helper.h"
#include "want_agent_info.h"
#include "notification_helper.h"
#include "notification_content.h"
#include "notification_normal_content.h"
#include "notification_request.h"
#include "notification_constant.h"
#include "notification_bundle_option.h"
#include "res_config.h"
#include "rstate.h"
#include "ipc_skeleton.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
// static std::string GetStringByName(const char* name,
//     std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
// {
//     std::string resourceContext;
//     if (resourceManager == nullptr) {
//         AUDIO_ERR_LOG("Hello, resourceManager is null.");
//         return resourceContext;
//     }
    
//     auto ret = OHOS::Global::Resource::RState::SUCCESS;
//     if (name == SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID) {
//         ret = resourceManager->GetStringByName(SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID, resourceContext);
//         if (ret != OHOS::Global::Resource::RState::SUCCESS) {
//             AUDIO_ERR_LOG("Hello, get SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID failed.");
//         }
//     } else if (name == SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID) {
//         ret = resourceManager->GetStringByName(SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID, resourceContext);
//         if (ret != OHOS::Global::Resource::RState::SUCCESS) {
//             AUDIO_ERR_LOG("Hello, get SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID failed.");
//         }
//     } else if (name == SAFE_VOLUME_MUSIC_TIMER_TEXT_ID) {
//         ret = resourceManager->GetStringByName(SAFE_VOLUME_MUSIC_TIMER_TEXT_ID, resourceContext);
//         if (ret != OHOS::Global::Resource::RState::SUCCESS) {
//             AUDIO_ERR_LOG("Hello, get SAFE_VOLUME_MUSIC_TIMER_TEXT_ID failed.");
//         }
//     } else if (name == SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID) {
//         ret = resourceManager->GetStringByName(SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID, resourceContext);
//         if (ret != OHOS::Global::Resource::RState::SUCCESS) {
//             AUDIO_ERR_LOG("Hello, get SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID failed.");
//         }
//     } else {
//         AUDIO_ERR_LOG("Hello, resource name is error.");
//         return resourceContext;
//     }
//     return resourceContext;
// }

static bool SetTitleAndText(int32_t notificationId, std::shared_ptr<Notification::NotificationNormalContent> content,
    std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    if (content == nullptr) {
        AUDIO_ERR_LOG("Hello, notification normal content nullptr");
        return false;
    }

    if (resourceManager == nullptr) {
        AUDIO_ERR_LOG("Hello, resourceManager is null.");
        return false;
    }

    switch (notificationId) {
        case SAFE_VOLUME_NOTIFICATION_ID_1:
            //test notification
            AUDIO_INFO_LOG("Hello, SetTitleAndText SAFE_VOLUME_NOTIFICATION_ID_1.");
            content->SetTitle(SAFE_VOLUME_MUSIC_TIMER_TITTLE);
            content->SetText(SAFE_VOLUME_MUSIC_TIMER_TEXT);
            //to do
            // content->SetTitle(GetStringByName(SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID, resourceManager));
            // content->SetText(GetStringByName(SAFE_VOLUME_MUSIC_TIMER_TEXT_ID, resourceManager));
            break;
        case SAFE_VOLUME_NOTIFICATION_ID_2:
            //test notification
            AUDIO_INFO_LOG("Hello, SetTitleAndText SAFE_VOLUME_NOTIFICATION_ID_2.");
            content->SetTitle(SAFE_VOLUME_INCREASE_VOLUME_TITTLE);
            content->SetText(SAFE_VOLUME_INCREASE_VOLUME_TEXT);
            //to do
            // content->SetTitle(GetStringByName(SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID, resourceManager));
            // content->SetText(GetStringByName(SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID, resourceManager));
            break;
        default:
            AUDIO_ERR_LOG("Hello, error notificationId");
            return false;
    }
    return true;
}

static std::string GetButtonName(uint32_t notificationId, 
    std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    std::string buttonName;
    if (resourceManager == nullptr) {
        AUDIO_ERR_LOG("Hello, resourceManager is null.");
        return buttonName;
    }

    // auto ret = OHOS::Global::Resource::RState::SUCCESS;
    switch (notificationId) {
        case SAFE_VOLUME_NOTIFICATION_ID_1:
            AUDIO_INFO_LOG("Hello, GetButtonName SAFE_VOLUME_NOTIFICATION_ID_1.");
            buttonName = SAFE_VOLUME_RESTORE_VOL_BUTTON;
            // ret = resourceManager->GetStringByName(SAFE_VOLUME_RESTORE_VOL_BUTTON_ID, buttonName);
            // if (ret != OHOS::Global::Resource::RState::SUCCESS) {
            //     AUDIO_ERR_LOG("get SAFE_VOLUME_RESTORE_VOL_BUTTON_ID failed.");
            // }
            break;
        case SAFE_VOLUME_NOTIFICATION_ID_2:
            AUDIO_INFO_LOG("Hello, GetButtonName SAFE_VOLUME_NOTIFICATION_ID_2.");
            buttonName = SAFE_VOLUME_INCREASE_VOL_BUTTON;
            // ret = resourceManager->GetStringByName(SAFE_VOLUME_INCREASE_VOL_BUTTON_ID, buttonName);
            // if (ret != OHOS::Global::Resource::RState::SUCCESS) {
            //     AUDIO_ERR_LOG("get SAFE_VOLUME_INCREASE_VOL_BUTTON_ID failed.");
            // }
            break;
        default:
            AUDIO_ERR_LOG("resource name is error.");
            return buttonName;
    }
    return buttonName;
}

static void SetActionButton(int32_t notificationId, const std::string& buttonName,
    Notification::NotificationRequest& request)
{
    auto want = std::make_shared<AAFwk::Want>();
    if (notificationId == SAFE_VOLUME_NOTIFICATION_ID_1) {
        AUDIO_INFO_LOG("Hello, SetActionButton AUDIO_RESTORE_VOLUME_EVENT.");
        want->SetAction(AUDIO_RESTORE_VOLUME_EVENT);
    } else {
        AUDIO_INFO_LOG("Hello, SetActionButton AUDIO_INCREASE_VOLUME_EVENT.");
        want->SetAction(AUDIO_INCREASE_VOLUME_EVENT);
    }
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(want);
    std::vector<AbilityRuntime::WantAgent::WantAgentConstant::Flags> flags;
    flags.push_back(AbilityRuntime::WantAgent::WantAgentConstant::Flags::CONSTANT_FLAG);
    AbilityRuntime::WantAgent::WantAgentInfo wantAgentInfo(
        0, AbilityRuntime::WantAgent::WantAgentConstant::OperationType::SEND_COMMON_EVENT,
        flags, wants, nullptr
    );
    auto wantAgentDeal = AbilityRuntime::WantAgent::WantAgentHelper::GetWantAgent(wantAgentInfo);
    std::shared_ptr<Notification::NotificationActionButton> actionButtonDeal =
        Notification::NotificationActionButton::Create(nullptr, buttonName, wantAgentDeal);
    if (actionButtonDeal == nullptr) {
        AUDIO_ERR_LOG("get notification actionButton nullptr");
        return;
    }
    AUDIO_INFO_LOG("Hello, SetActionButton AddActionButton.");
    request.AddActionButton(actionButtonDeal);
}

bool AudioSafeVolumeNotification::GetPixelMap(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    if (iconPixelMap_ != nullptr) {
        AUDIO_ERR_LOG("Hello, icon pixel map already exists.");
        return false;
    }

    if (resourceManager == nullptr) {
        AUDIO_ERR_LOG("Hello, resourceManager is null.");
        return false;
    }

    std::unique_ptr<uint8_t[]> resourceData;
    size_t resourceDataLength = 0;
    auto ret = resourceManager->GetMediaDataByName(SAFE_VOLUME_ICON_ID, resourceDataLength, resourceData);
    if (ret != Global::Resource::RState::SUCCESS) {
        AUDIO_ERR_LOG("Hello, get (%{public}s) failed, errorCode:%{public}d", SAFE_VOLUME_ICON_ID, static_cast<int32_t>(ret));
        return false;
    }

    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(resourceData.get(), resourceDataLength, opts, errorCode);
    Media::DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
    if (imageSource) {
        AUDIO_INFO_LOG("Hello, GetPixelMap SUCCESS.");
        auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
        iconPixelMap_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
    }
    if (errorCode != 0 || (iconPixelMap_ == nullptr)) {
        AUDIO_ERR_LOG("Hello, get badge failed, errorCode:%{public}u", errorCode);
        return false;
    }
    return true;
}

AudioSafeVolumeNotification& AudioSafeVolumeNotification::GetInstance()
{
    AUDIO_INFO_LOG("Hello, AudioSafeVolumeNotification GetInstance enter.");
    static AudioSafeVolumeNotification instance;
    return instance;
}

AudioSafeVolumeNotification::AudioSafeVolumeNotification()
{
    AUDIO_INFO_LOG("Hello, AudioSafeVolumeNotification constructor enter.");
}

AudioSafeVolumeNotification::~AudioSafeVolumeNotification()
{
    AUDIO_INFO_LOG("Hello, AudioSafeVolumeNotification destructor enter.");
}

void AudioSafeVolumeNotification::PublishSafeVolumeNotification(int32_t notificationId)
{
    std::shared_ptr<Notification::NotificationNormalContent> normalContent =
        std::make_shared<Notification::NotificationNormalContent>();
    if (normalContent == nullptr) {
        AUDIO_ERR_LOG("Hello, get notification normal content nullptr");
        return;
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resourceManager->UpdateResConfig(*resConfig);
    if (!SetTitleAndText(notificationId, normalContent, resourceManager)) {
        AUDIO_ERR_LOG("Hello, error setting title and text");
        return;
    }

    std::shared_ptr<Notification::NotificationContent> content =
        std::make_shared<Notification::NotificationContent>(normalContent);
    if (content == nullptr) {
        AUDIO_ERR_LOG("Hello, get notification content nullptr");
        return;
    }

    Notification::NotificationRequest request;
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    request.SetCreatorUid(callerUid);
    request.SetNotificationId(notificationId);
    request.SetContent(content);
    request.SetSlotType(Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    request.SetTapDismissed(false);
    request.SetAutoDeletedTime(Notification::NotificationConstant::INVALID_AUTO_DELETE_TIME);
    request.SetCreatorBundleName("audio_policy");

    GetPixelMap(resourceManager);
    if (iconPixelMap_ != nullptr) {
        request.SetLittleIcon(iconPixelMap_);
        request.SetBadgeIconStyle(Notification::NotificationRequest::BadgeStyle::LITTLE);
    }

    std::string buttonName = GetButtonName(notificationId, resourceManager);
    if (!buttonName.empty()) {
        SetActionButton(notificationId, buttonName, request);
    }

    Notification::NotificationBundleOption bundle("audio_policy", callerUid);
    Notification::NotificationHelper::SetNotificationSlotFlagsAsBundle(bundle, SLOT_CONTROL_FLAG);
    auto ret = Notification::NotificationHelper::PublishNotification(request);
    AUDIO_INFO_LOG("Hello, safe volume service publish notification result = %{public}d", ret);
}
}  // namespace AudioStandard
}  // namespace OHOS