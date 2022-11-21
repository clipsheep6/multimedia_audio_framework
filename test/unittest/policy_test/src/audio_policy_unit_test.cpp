/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <thread>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_unit_test.h"
#include "audio_system_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t FAILURE = -1;
void AudioPolicyUnitTest::SetUpTestCase(void) {}
void AudioPolicyUnitTest::TearDownTestCase(void) {}
void AudioPolicyUnitTest::SetUp(void) {}
void AudioPolicyUnitTest::TearDown(void) {}
void AudioPolicyUnitTest::InitAudioPolicyProxy(std::shared_ptr<AudioPolicyProxy> &audioPolicyProxy)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        AUDIO_ERR_LOG("InitAudioPolicyProxy::GetSystemAbilityManager failed");
        return;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    if (object == nullptr) {
        AUDIO_DEBUG_LOG("InitAudioPolicyProxy::object is NULL.");
        return;
    }
    audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
}

void AudioPolicyUnitTest::GetIRemoteObject(sptr<IRemoteObject> &object)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        AUDIO_ERR_LOG("GetIRemoteObject::GetSystemAbilityManager failed");
        return;
    }

    object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    if (object == nullptr) {
        AUDIO_DEBUG_LOG("GetIRemoteObject::object is NULL.");
        return;
    }
}

void AudioPolicyUnitTest::InitAudioStream(std::shared_ptr<AudioStream> &audioStream)
{
    AppInfo appInfo_ = {};
    if (!(appInfo_.appPid)) {
        appInfo_.appPid = getpid();
    }

    if (appInfo_.appUid < 0) {
        appInfo_.appUid = static_cast<int32_t>(getuid());
    }
    
    audioStream = std::make_shared<AudioStream>(STREAM_NOTIFICATION, AUDIO_MODE_PLAYBACK, appInfo_.appUid);
    if (audioStream) {
        AUDIO_DEBUG_LOG("InitAudioStream::Audio stream created");
    }
}

uint32_t AudioPolicyUnitTest::GetSessionId(std::shared_ptr<AudioStream> &audioStream)
{
    uint32_t sessionID_ = static_cast<uint32_t>(-1);
    if (audioStream->GetAudioSessionID(sessionID_) != 0) {
        AUDIO_ERR_LOG("AudioPolicyUnitTest::GetSessionId Failed");
    }
    return sessionID_;
}

/**
* @tc.name  : Test Audio_Policy_SetMicrophoneMuteAudioConfig_001 via illegal state
* @tc.number: Audio_Policy_SetMicrophoneMuteAudioConfig_001
* @tc.desc  : Test SetMicrophoneMuteAudioConfig interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_SetMicrophoneMuteAudioConfig_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    bool isMute = true;
    int32_t ret = audioPolicyProxy->SetMicrophoneMuteAudioConfig(isMute);
    EXPECT_EQ(FAILURE, ret);
}

/**
* @tc.name  : Test Audio_Policy_GetSupportedTones_001 via legal state
* @tc.number: Audio_Policy_GetSupportedTones_001
* @tc.desc  : Test GetSupportedTones interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetSupportedTones_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    audioPolicyProxy->GetSupportedTones();
}

/**
* @tc.name  : Test Audio_Policy_GetToneConfig_001 via legal state
* @tc.number: Audio_Policy_GetToneConfig_001
* @tc.desc  : Test GetToneConfig interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetToneConfig_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    int32_t ltonetype = 0;
    std::shared_ptr<ToneInfo> toneInfo = audioPolicyProxy->GetToneConfig(ltonetype);
    ASSERT_NE(nullptr, toneInfo);
}

/**
* @tc.name  : Test Audio_Policy_IsStreamActive_001 via legal state
* @tc.number: Audio_Policy_IsStreamActive_001
* @tc.desc  : Test IsStreamActive interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_IsStreamActive_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    bool isStreamActive = audioPolicyProxy->IsStreamActive(streamType);
    EXPECT_EQ(false, isStreamActive);
}

/**
* @tc.name  : Test Audio_Policy_SelectInputDevice_001 via illegal state
* @tc.number: Audio_Policy_SelectInputDevice_001
* @tc.desc  : Test SelectInputDevice interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_SelectInputDevice_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    DeviceFlag deviceFlag = DeviceFlag::INPUT_DEVICES_FLAG;
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptorsVector;
    audioDeviceDescriptorsVector = audioSystemMgr->GetDevices(deviceFlag);

    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = DeviceFlag::INPUT_DEVICES_FLAG;

    int32_t ret = audioPolicyProxy->SelectInputDevice(audioCapturerFilter, audioDeviceDescriptorsVector);
    EXPECT_EQ(FAILURE, ret);
}

/**
* @tc.name  : Test Audio_Policy_DeviceChangeCallback_001 via illegal state
* @tc.number: Audio_Policy_DeviceChangeCallback_001
* @tc.desc  : Test SetDeviceChangeCallback and UnsetDeviceChangeCallback interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_DeviceChangeCallback_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    int32_t clientId = getpid();
    DeviceFlag flag = DeviceFlag::OUTPUT_DEVICES_FLAG;
    sptr<IRemoteObject> object;
    AudioPolicyUnitTest::GetIRemoteObject(object);

    int32_t ret = audioPolicyProxy->SetDeviceChangeCallback(clientId, flag, object);
    EXPECT_EQ(FAILURE, ret);

    ret = audioPolicyProxy->UnsetDeviceChangeCallback(clientId);
    EXPECT_EQ(FAILURE, ret);
}

/**
* @tc.name  : Test Audio_Policy_GetStreamInFocus_001 via legal state
* @tc.number: Audio_Policy_GetStreamInFocus_001
* @tc.desc  : Test GetStreamInFocus interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetStreamInFocus_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    audioPolicyProxy->GetStreamInFocus();
}

/**
* @tc.name  : Test Audio_Policy_IsAudioRendererLowLatencySupported_001 via legal state
* @tc.number: Audio_Policy_IsAudioRendererLowLatencySupported_001
* @tc.desc  : Test IsAudioRendererLowLatencySupported interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_IsAudioRendererLowLatencySupported_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::MONO;
    bool ret = audioPolicyProxy->IsAudioRendererLowLatencySupported(audioStreamInfo);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test Audio_Policy_RegisterAudioRendererEventListener_001 via illegal state
* @tc.number: Audio_Policy_RegisterAudioRendererEventListener_001
* @tc.desc  : Test RegisterAudioRendererEventListener interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_RegisterAudioRendererEventListener_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    int32_t clientId = getpid();
    sptr<IRemoteObject> object;
    AudioPolicyUnitTest::GetIRemoteObject(object);
    int32_t ret = audioPolicyProxy->RegisterAudioRendererEventListener(clientId, object);
    EXPECT_EQ(ERROR, ret);

    ret = audioPolicyProxy->UnregisterAudioRendererEventListener(clientId);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test Audio_Policy_RegisterAudioCapturerEventListener_001 via illegal state
* @tc.number: Audio_Policy_RegisterAudioCapturerEventListener_001
* @tc.desc  : Test RegisterAudioCapturerEventListener interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_RegisterAudioCapturerEventListener_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy;
    AudioPolicyUnitTest::InitAudioPolicyProxy(audioPolicyProxy);
    ASSERT_NE(nullptr, audioPolicyProxy);

    sptr<IRemoteObject> object;
    AudioPolicyUnitTest::GetIRemoteObject(object);
    int32_t clientId = getpid();

    int32_t ret = audioPolicyProxy->RegisterAudioCapturerEventListener(clientId, object);
    EXPECT_EQ(ERROR, ret);

    ret = audioPolicyProxy->UnregisterAudioCapturerEventListener(clientId);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test Audio_Policy_Manager_IsStreamActive_001 via illegal state
* @tc.number: Audio_Policy_Manager_IsStreamActive_001
* @tc.desc  : Test RegisterAudioCapturerEventListener interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_IsStreamActive_001, TestSize.Level1)
{
    bool isStreamActive = AudioPolicyManager::GetInstance().IsStreamActive(AudioStreamType::STREAM_MUSIC);
    EXPECT_EQ(false, isStreamActive);
}

/**
* @tc.name  : Test Audio_Policy_Manager_SetMicrophoneMuteAudioConfig_001 via legal state
* @tc.number: Audio_Policy_Manager_SetMicrophoneMuteAudioConfig_001
* @tc.desc  : Test SetMicrophoneMuteAudioConfig interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetMicrophoneMuteAudioConfig_001, TestSize.Level1)
{
    bool isMute = true;
    bool ret = AudioPolicyManager::GetInstance().SetMicrophoneMuteAudioConfig(isMute);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test Audio_Policy_Manager_GetSupportedTones_001 via legal state
* @tc.number: Audio_Policy_Manager_GetSupportedTones_001
* @tc.desc  : Test GetSupportedTones interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_GetSupportedTones_001, TestSize.Level1)
{
    AudioPolicyManager::GetInstance().GetSupportedTones();
}

/**
* @tc.name  : Test Audio_Policy_Manager_GetToneConfig_001 via legal state
* @tc.number: Audio_Policy_Manager_GetToneConfig_001
* @tc.desc  : Test GetToneConfig interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_GetToneConfig_001, TestSize.Level1)
{
    int32_t ltonetype = 0;
    std::shared_ptr<ToneInfo> toneInfo = AudioPolicyManager::GetInstance().GetToneConfig(ltonetype);
    ASSERT_NE(nullptr, toneInfo);
}

/**
* @tc.name  : Test Audio_Policy_Manager_SetDeviceChangeCallback_001 via legal state
* @tc.number: Audio_Policy_Manager_SetDeviceChangeCallback_001
* @tc.desc  : Test SetDeviceChangeCallback interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetDeviceChangeCallback_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    DeviceFlag flag = DeviceFlag::OUTPUT_DEVICES_FLAG;
    std::shared_ptr<AudioManagerDeviceChangeCallback> callback =
        std::make_shared<AudioManagerDeviceChangeCallbackTest>();

    int32_t ret = AudioPolicyManager::GetInstance().SetDeviceChangeCallback(clientId, flag, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnsetDeviceChangeCallback(clientId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test Audio_Policy_Manager_GetStreamInFocus_001 via legal state
* @tc.number: Audio_Policy_Manager_GetStreamInFocus_001
* @tc.desc  : Test GetStreamInFocus interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_GetStreamInFocus_001, TestSize.Level1)
{
    AudioPolicyManager::GetInstance().GetStreamInFocus();
}

/**
* @tc.name  : Test Audio_Policy_Manager_GetSessionInfoInFocus_001 via legal state
* @tc.number: Audio_Policy_Manager_GetSessionInfoInFocus_001
* @tc.desc  : Test GetSessionInfoInFocus interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_GetSessionInfoInFocus_001, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;

    std::shared_ptr<AudioStream> audioStream;
    AudioPolicyUnitTest::InitAudioStream(audioStream);
    ASSERT_NE(nullptr, audioStream);

    uint32_t sessionID_ = AudioPolicyUnitTest::GetSessionId(audioStream);
    audioInterrupt.sessionID = sessionID_;
    int32_t ret = AudioPolicyManager::GetInstance().GetSessionInfoInFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test Audio_Policy_Manager_RegisterAudioRendererEventListener_001 via legal state
* @tc.number: Audio_Policy_Manager_RegisterAudioRendererEventListener_001
* @tc.desc  : Test registerAudioRendererEventListener interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_RegisterAudioRendererEventListener_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    std::shared_ptr<AudioRendererStateChangeCallback> callback =
        std::make_shared<AudioRendererStateChangeCallbackTest>();
    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(clientId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test Audio_Policy_Manager_RegisterAudioCapturerEventListener_001 via legal state
* @tc.number: Audio_Policy_Manager_RegisterAudioCapturerEventListener_001
* @tc.desc  : Test RegisterAudioCapturerEventListener interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_RegisterAudioCapturerEventListener_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    std::shared_ptr<AudioCapturerStateChangeCallback> callback =
        std::make_shared<AudioCapturerStateChangeCallbackTest>();
    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterAudioCapturerEventListener(clientId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test Audio_Policy_Manager_IsAudioRendererLowLatencySupported_001 via legal state
* @tc.number: Audio_Policy_Manager_IsAudioRendererLowLatencySupported_001
* @tc.desc  : Test IsAudioRendererLowLatencySupported interface. Returns success.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_IsAudioRendererLowLatencySupported_001, TestSize.Level1)
{
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::MONO;
    bool ret = AudioPolicyManager::GetInstance().IsAudioRendererLowLatencySupported(audioStreamInfo);
    EXPECT_EQ(true, ret);
}
} // namespace AudioStandard
} // namespace OHOS