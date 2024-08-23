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

#include "audio_interrupt_unit_test.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInterruptUnitTest::SetUpTestCase(void) {}
void AudioInterruptUnitTest::TearDownTestCase(void) {}
void AudioInterruptUnitTest::SetUp(void) {}

void AudioInterruptUnitTest::TearDown(void) {}

std::shared_ptr<AudioInterruptService> GetTnterruptServiceTest()
{
    return std::make_shared<AudioInterruptService>();
}

std::shared_ptr<AudioPolicyServerHandler> GetServerHandlerTest()
{
    return DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
}

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_001
* @tc.desc  : Test AudioSessionService interfaces.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    sptr<AudioPolicyServer> server = nullptr;
    interruptServiceTest->Init(server);
    interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());
    interruptServiceTest->OnSessionTimeout(0);

    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;

    auto sessionService = interruptServiceTest->sessionService_;
    interruptServiceTest->sessionService_ = nullptr;
    auto retStatus = interruptServiceTest->ActivateAudioSession(0, strategy);
    EXPECT_EQ(retStatus, ERR_UNKNOWN);
    interruptServiceTest->AddActiveInterruptToSession(0);
    retStatus = interruptServiceTest->DeactivateAudioSession(0);
    EXPECT_EQ(retStatus, ERR_UNKNOWN);

    interruptServiceTest->sessionService_ = sessionService;
    retStatus = interruptServiceTest->ActivateAudioSession(0, strategy);
    EXPECT_EQ(retStatus, SUCCESS);
    retStatus = interruptServiceTest->ActivateAudioSession(0, strategy);
    EXPECT_EQ(retStatus, ERR_ILLEGAL_STATE);
    interruptServiceTest->AddActiveInterruptToSession(0);
    retStatus = interruptServiceTest->DeactivateAudioSession(0);
    interruptServiceTest->sessionService_->sessionTimer_->timerThread_->join();
    EXPECT_EQ(retStatus, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_002
* @tc.desc  : Test RemovePlaceholderInterruptForSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_002, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = nullptr;
    auto interruptServiceTest = GetTnterruptServiceTest();
    EXPECT_EQ(interruptServiceTest->sessionService_, nullptr);
    interruptServiceTest->RemovePlaceholderInterruptForSession(0, false);
    interruptServiceTest->RemovePlaceholderInterruptForSession(0, true);
    auto retStatus = interruptServiceTest->IsAudioSessionActivated(0);
    EXPECT_EQ(retStatus, static_cast<bool>(ERR_UNKNOWN));

    interruptServiceTest->Init(server);
    EXPECT_NE(interruptServiceTest->sessionService_, nullptr);
    interruptServiceTest->RemovePlaceholderInterruptForSession(0, false);
    interruptServiceTest->RemovePlaceholderInterruptForSession(0, true);
    retStatus = interruptServiceTest->IsAudioSessionActivated(0);
    EXPECT_EQ(retStatus, interruptServiceTest->sessionService_->IsAudioSessionActivated(0));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_003
* @tc.desc  : Test Session MIX.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_003, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = nullptr;
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = true;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    bool status = interruptServiceTest->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(status, false);

    focusEntry.isReject = false;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    status = interruptServiceTest->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(status, false);

    interruptServiceTest->Init(nullptr); // populate "interruptServiceTest->sessionService_"
    incomingInterrupt.pid = 0;
    activeInterrupt.pid = 1;
    EXPECT_EQ(false, interruptServiceTest->sessionService_->IsAudioSessionActivated(incomingInterrupt.pid));
    status = interruptServiceTest->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(status, false);

    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;

    interruptServiceTest->sessionService_->ActivateAudioSession(incomingInterrupt.pid, strategy);
    EXPECT_EQ(true, interruptServiceTest->sessionService_->IsAudioSessionActivated(incomingInterrupt.pid));
    AUDIO_INFO_LOG("activate incomingInterrupt by tester");
    status = interruptServiceTest->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(status, false);

    auto retStatus = interruptServiceTest->DeactivateAudioSession(incomingInterrupt.pid);
    usleep(5000);
    EXPECT_EQ(retStatus, SUCCESS);
    interruptServiceTest->sessionService_->ActivateAudioSession(activeInterrupt.pid, strategy);
    EXPECT_EQ(true, interruptServiceTest->sessionService_->IsAudioSessionActivated(activeInterrupt.pid));
    AUDIO_INFO_LOG("activate activeInterrupt by tester");
    status = interruptServiceTest->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(status, false);

    retStatus = interruptServiceTest->DeactivateAudioSession(activeInterrupt.pid);
    interruptServiceTest->sessionService_->sessionTimer_->timerThread_->join();
    interruptServiceTest->SetAudioManagerInterruptCallback(nullptr);
    EXPECT_EQ(retStatus, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_004
* @tc.desc  : Test AddDumpInfo.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_004, TestSize.Level1)
{
    std::string dumpString;
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[0]->interruptCbsMap[0] = nullptr;
    interruptServiceTest->AudioInterruptZoneDump(dumpString);
    std::cout << dumpString << std::endl;
    EXPECT_NE(dumpString.find("1 AudioInterruptZoneDump (s) available"), std::string::npos);
    EXPECT_NE(dumpString.find("Interrupt callback size: 1"), std::string::npos);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_005
* @tc.desc  : Test AbandonAudioFocus.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_005, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioInterrupt incomingInterrupt;
    int32_t clientID = interruptServiceTest->clientOnFocus_;
    auto retStatus = interruptServiceTest->AbandonAudioFocus(clientID, incomingInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);
}

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_006
* @tc.desc  : Test SetAudioInterruptCallback and UnsetAudioInterruptCallback.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_006, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    auto retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, nullptr);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    retStatus = interruptServiceTest->UnsetAudioInterruptCallback(0, 0);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, sptr<RemoteObjectTestStub>::MakeSptr());
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, sptr<RemoteObjectTestStub>::MakeSptr());
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    retStatus = interruptServiceTest->UnsetAudioInterruptCallback(0, 0);
    EXPECT_EQ(retStatus, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_007
* @tc.desc  : Test ActivateAudioInterrupt.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_007, TestSize.Level1)
{
    const int32_t SYSTEM_ABILITY_ID = 3009;
    const bool RUN_ON_CREATE = false;
    auto interruptServiceTest = GetTnterruptServiceTest();
    sptr<AudioPolicyServer> server =
        sptr<AudioPolicyServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    interruptServiceTest->Init(server);

    AUDIO_INFO_LOG("========================ActivateAudioInterrupt 1===================================");
    AudioInterrupt audioInterrupt;
    EXPECT_NE(interruptServiceTest->policyServer_, nullptr);
    auto retStatus = interruptServiceTest->ActivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);

    AUDIO_INFO_LOG("========================ActivateAudioInterrupt 2===================================");
    AudioInterrupt audioInterrupt2;
    audioInterrupt2.sessionId = 2;
    audioInterrupt2.pid = 2;
    EXPECT_EQ(interruptServiceTest->zonesMap_.empty(), false);
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back(std::make_pair(audioInterrupt2, ACTIVE));
    retStatus = interruptServiceTest->ActivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(retStatus, ERR_FOCUS_DENIED);

    AUDIO_INFO_LOG("========================ActivateAudioInterrupt 3===================================");
    audioInterrupt.parallelPlayFlag = true;
    retStatus = interruptServiceTest->ActivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);

    AUDIO_INFO_LOG("========================ActivateAudioInterrupt 4===================================");
    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.empty(), true);
    audioInterrupt.parallelPlayFlag = false;
    retStatus = interruptServiceTest->ActivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(retStatus, ERR_FOCUS_DENIED);

    AUDIO_INFO_LOG("========================DeActivateAudioInterrupt 1===================================");
    retStatus = interruptServiceTest->DeactivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);

    AUDIO_INFO_LOG("========================DeActivateAudioInterrupt 2===================================");
    audioInterrupt.parallelPlayFlag = true;
    retStatus = interruptServiceTest->DeactivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_008
* @tc.desc  : Test Zone debug interfaces.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_008, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();

    auto retStatus = interruptServiceTest->CreateAudioInterruptZone(0, {});
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->AddAudioInterruptZonePids(0, {1, 2, 3});
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->RemoveAudioInterruptZonePids(-1, {1, 2});
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->RemoveAudioInterruptZonePids(0, {1, 2});
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->ReleaseAudioInterruptZone(2);
    EXPECT_EQ(retStatus, SUCCESS);

    std::shared_ptr<AudioInterruptZone> hold = interruptServiceTest->zonesMap_[0];
    interruptServiceTest->zonesMap_[0] = nullptr;

    retStatus = interruptServiceTest->RemoveAudioInterruptZonePids(0, {1, 2});
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->ReleaseAudioInterruptZone(0);
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->AddAudioInterruptZonePids(0, {1, 2, 3});
    EXPECT_EQ(retStatus, SUCCESS);

    interruptServiceTest->zonesMap_[0] = hold;
    retStatus = interruptServiceTest->ReleaseAudioInterruptZone(0);
    EXPECT_EQ(retStatus, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_009
* @tc.desc  : Test DeactivateAudioInterruptInternal.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_009, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioInterrupt audioInterrupt;
    bool isSessionTimeout = true;
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, isSessionTimeout);
    
    auto retStatus = interruptServiceTest->CreateAudioInterruptZone(0, {});
    EXPECT_EQ(retStatus, SUCCESS);
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, isSessionTimeout);
}
} // namespace AudioStandard
} // namespace OHOS
