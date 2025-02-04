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

#ifndef LOG_TAG
#define LOG_TAG "AudioEffectChainAdapterUnitTest"
#endif

#include "audio_effect_chain_adapter_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_utils.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_adapter.h"
#include "audio_effect_chain_manager.h"
#include "audio_errors.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {

#define DEFAULT_NUM_CHANNEL 2
#define DEFAULT_CHANNELLAYOUT 3

vector<EffectChain> DEFAULT_EFFECT_CHAINS = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};

unordered_map<string, string> DEFAULT_MAP = {
    {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
    {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"},
};

EffectChainManagerParam DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

vector<shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};
}

void AudioEffectChainAdapterUnitTest::SetUpTestCase(void) {}
void AudioEffectChainAdapterUnitTest::TearDownTestCase(void) {}
void AudioEffectChainAdapterUnitTest::SetUp(void) {}
void AudioEffectChainAdapterUnitTest::TearDown(void) {}

/**
* @tc.name   : Test EffectChainManagerInitCb API
* @tc.number : EffectChainManagerInitCb_001
* @tc.desc   : Test EffectChainManagerInitCb interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerInitCb_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerInitCb API
* @tc.number : EffectChainManagerInitCb_002
* @tc.desc   : Test EffectChainManagerInitCb interface(using empty use case).
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerInitCb_002, TestSize.Level1)
{
    const char *sceneType = "";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerInitCb API
* @tc.number : EffectChainManagerInitCb_003
* @tc.desc   : Test EffectChainManagerInitCb interface(using empty use case).
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerInitCb_003, TestSize.Level1)
{
    const char *sceneType = nullptr;
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerCreateCb API
* @tc.number : EffectChainManagerCreateCb_001
* @tc.desc   : Test EffectChainManagerCreateCb interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerCreateCb_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "111";
    result = EffectChainManagerCreateCb(sceneType, sessionid);
    EXPECT_EQ(SUCCESS, result);

    result = EffectChainManagerReleaseCb(sceneType, sessionid);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerCreateCb API
* @tc.number : EffectChainManagerCreateCb_002
* @tc.desc   : Test EffectChainManagerCreateCb interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerCreateCb_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MOVIE";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "222";
    result = EffectChainManagerCreateCb(sceneType, sessionid);
    EXPECT_EQ(SUCCESS, result);

    result = EffectChainManagerReleaseCb(sceneType, sessionid);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerCreateCb API
* @tc.number : EffectChainManagerCreateCb_003
* @tc.desc   : Test EffectChainManagerCreateCb interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerCreateCb_003, TestSize.Level1)
{
    const char *sceneType = "";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(ERROR, result);

    const char *sessionid = "";
    result = EffectChainManagerCreateCb(sceneType, sessionid);
    EXPECT_EQ(ERROR, result);

    result = EffectChainManagerReleaseCb(sceneType, sessionid);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerCreateCb API
* @tc.number : EffectChainManagerCreateCb_004
* @tc.desc   : Test EffectChainManagerCreateCb interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerCreateCb_004, TestSize.Level1)
{
    const char *sceneType = nullptr;
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(ERROR, result);

    const char *sessionid = nullptr;
    result = EffectChainManagerCreateCb(sceneType, sessionid);
    EXPECT_EQ(ERROR, result);

    result = EffectChainManagerReleaseCb(sceneType, sessionid);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerSetHdiParam API
* @tc.number : EffectChainManagerSetHdiParam_001
* @tc.desc   : Test EffectChainManagerSetHdiParam interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerSetHdiParam_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *effectMode = "EFFECT_DEFAULT";
    result = EffectChainManagerSetHdiParam(sceneType, effectMode, true);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerSetHdiParam API
* @tc.number : EffectChainManagerSetHdiParam_002
* @tc.desc   : Test EffectChainManagerSetHdiParam interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerSetHdiParam_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    sceneType = nullptr;
    const char *effectMode = nullptr;
    result = EffectChainManagerSetHdiParam(sceneType, effectMode, true);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerCheckEffectOffload API
* @tc.number : EffectChainManagerCheckEffectOffload_001
* @tc.desc   : Test EffectChainManagerCheckEffectOffload interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerCheckEffectOffload_001, TestSize.Level1)
{
    const char *sceneType = "";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(ERROR, result);

    bool result2 = EffectChainManagerCheckEffectOffload();
    EXPECT_EQ(false, result2);
}

/**
* @tc.name   : Test EffectChainManagerAddSessionInfo API
* @tc.number : EffectChainManagerAddSessionInfo_001
* @tc.desc   : Test EffectChainManagerAddSessionInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerAddSessionInfo_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "123456";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerAddSessionInfo API
* @tc.number : EffectChainManagerAddSessionInfo_002
* @tc.desc   : Test EffectChainManagerAddSessionInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerAddSessionInfo_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerAddSessionInfo API
* @tc.number : EffectChainManagerAddSessionInfo_003
* @tc.desc   : Test EffectChainManagerAddSessionInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerAddSessionInfo_003, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    sceneType = "";
    const char *sessionid = "123456";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerAddSessionInfo API
* @tc.number : EffectChainManagerAddSessionInfo_004
* @tc.desc   : Test EffectChainManagerAddSessionInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerAddSessionInfo_004, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    sceneType = "";
    const char *sessionid = "";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerAddSessionInfo API
* @tc.number : EffectChainManagerAddSessionInfo_005
* @tc.desc   : Test EffectChainManagerAddSessionInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerAddSessionInfo_005, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    sceneType = nullptr;
    const char *sessionid = nullptr;
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerDeleteSessionInfo API
* @tc.number : EffectChainManagerDeleteSessionInfo_001
* @tc.desc   : Test EffectChainManagerDeleteSessionInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerDeleteSessionInfo_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "123456";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(ERROR, result);

    result = EffectChainManagerDeleteSessionInfo(sceneType, sessionid);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerDeleteSessionInfo API
* @tc.number : EffectChainManagerDeleteSessionInfo_002
* @tc.desc   : Test EffectChainManagerDeleteSessionInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerDeleteSessionInfo_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "123456";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(SUCCESS, result);

    sceneType = nullptr;
    result = EffectChainManagerDeleteSessionInfo(sceneType, sessionid);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerDeleteSessionInfo API
* @tc.number : EffectChainManagerDeleteSessionInfo_003
* @tc.desc   : Test EffectChainManagerDeleteSessionInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerDeleteSessionInfo_003, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "123456";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(ERROR, result);

    sessionid = nullptr;
    result = EffectChainManagerDeleteSessionInfo(sceneType, sessionid);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnEffectChannelInfo API
* @tc.number : EffectChainManagerReturnEffectChannelInfo_001
* @tc.desc   : Test EffectChainManagerReturnEffectChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnEffectChannelInfo_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "123456";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true"};
    result = EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EXPECT_EQ(ERROR, result);

    uint32_t processChannels = DEFAULT_NUM_CHANNEL;
    uint64_t processChannelLayout = DEFAULT_CHANNELLAYOUT;
    result = EffectChainManagerReturnEffectChannelInfo(sceneType, &processChannels, &processChannelLayout);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnEffectChannelInfo API
* @tc.number : EffectChainManagerReturnEffectChannelInfo_002
* @tc.desc   : Test EffectChainManagerReturnEffectChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnEffectChannelInfo_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    uint32_t processChannels = DEFAULT_NUM_CHANNEL;
    uint64_t processChannelLayout = DEFAULT_CHANNELLAYOUT;
    result = EffectChainManagerReturnEffectChannelInfo(nullptr, &processChannels, &processChannelLayout);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnEffectChannelInfo API
* @tc.number : EffectChainManagerReturnEffectChannelInfo_003
* @tc.desc   : Test EffectChainManagerReturnEffectChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnEffectChannelInfo_003, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    uint64_t processChannelLayout = DEFAULT_CHANNELLAYOUT;
    result = EffectChainManagerReturnEffectChannelInfo(sceneType, nullptr, &processChannelLayout);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnEffectChannelInfo API
* @tc.number : EffectChainManagerReturnEffectChannelInfo_004
* @tc.desc   : Test EffectChainManagerReturnEffectChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnEffectChannelInfo_004, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    uint32_t processChannels = DEFAULT_NUM_CHANNEL;
    result = EffectChainManagerReturnEffectChannelInfo(sceneType, &processChannels, nullptr);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnEffectChannelInfo API
* @tc.number : EffectChainManagerReturnEffectChannelInfo_005
* @tc.desc   : Test EffectChainManagerReturnEffectChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnEffectChannelInfo_005, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    result = EffectChainManagerReturnEffectChannelInfo(nullptr, nullptr, nullptr);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnMultiChannelInfo API
* @tc.number : EffectChainManagerReturnMultiChannelInfo_001
* @tc.desc   : Test EffectChainManagerReturnMultiChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnMultiChannelInfo_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    uint32_t processChannels = DEFAULT_NUM_CHANNEL;
    uint64_t processChannelLayout = DEFAULT_CHANNELLAYOUT;
    result = EffectChainManagerReturnMultiChannelInfo(&processChannels, &processChannelLayout);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnMultiChannelInfo API
* @tc.number : EffectChainManagerReturnMultiChannelInfo_002
* @tc.desc   : Test EffectChainManagerReturnMultiChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnMultiChannelInfo_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    uint64_t processChannelLayout = DEFAULT_CHANNELLAYOUT;
    result = EffectChainManagerReturnMultiChannelInfo(nullptr, &processChannelLayout);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnMultiChannelInfo API
* @tc.number : EffectChainManagerReturnMultiChannelInfo_003
* @tc.desc   : Test EffectChainManagerReturnMultiChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnMultiChannelInfo_003, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    uint32_t processChannels = DEFAULT_NUM_CHANNEL;
    result = EffectChainManagerReturnMultiChannelInfo(&processChannels, nullptr);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerReturnMultiChannelInfo API
* @tc.number : EffectChainManagerReturnMultiChannelInfo_004
* @tc.desc   : Test EffectChainManagerReturnMultiChannelInfo interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerReturnMultiChannelInfo_004, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    result = EffectChainManagerReturnMultiChannelInfo(nullptr, nullptr);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerGetSpatializationEnabled API
* @tc.number : EffectChainManagerGetSpatializationEnabled_001
* @tc.desc   : Test EffectChainManagerGetSpatializationEnabled interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerGetSpatializationEnabled_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    bool result2 = EffectChainManagerGetSpatializationEnabled();
    EXPECT_EQ(false, result2);

    EffectChainManagerFlush();
}

/**
* @tc.name   : Test EffectChainManagerSceneCheck API
* @tc.number : EffectChainManagerSceneCheck_001
* @tc.desc   : Test EffectChainManagerSceneCheck interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerSceneCheck_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);
    
    const char *sinkSceneType = "SCENE_MUSIC";
    bool result2 = EffectChainManagerSceneCheck(sinkSceneType, sceneType);
    EXPECT_EQ(false, result2);
}


/**
* @tc.name   : Test EffectChainManagerSceneCheck API
* @tc.number : EffectChainManagerSceneCheck_002
* @tc.desc   : Test EffectChainManagerSceneCheck interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerSceneCheck_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);
    
    const char *sinkSceneType = "";
    sceneType = "";
    bool result2 = EffectChainManagerSceneCheck(sinkSceneType, sceneType);
    EXPECT_EQ(false, result2);
}

/**
* @tc.name   : Test EffectChainManagerSceneCheck API
* @tc.number : EffectChainManagerSceneCheck_003
* @tc.desc   : Test EffectChainManagerSceneCheck interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerSceneCheck_003, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);
    
    const char *sinkSceneType = "";
    bool result2 = EffectChainManagerSceneCheck(sinkSceneType, sceneType);
    EXPECT_EQ(false, result2);
}

/**
* @tc.name   : Test EffectChainManagerSceneCheck API
* @tc.number : EffectChainManagerSceneCheck_004
* @tc.desc   : Test EffectChainManagerSceneCheck interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerSceneCheck_004, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    sceneType = "";
    const char *sinkSceneType = "SCENE_MUSIC";
    bool result2 = EffectChainManagerSceneCheck(sinkSceneType, sceneType);
    EXPECT_EQ(false, result2);
}

/**
* @tc.name   : Test EffectChainManagerProcess API
* @tc.number : EffectChainManagerProcess_001
* @tc.desc   : Test EffectChainManagerProcess interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerProcess_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);
    
    struct BufferAttr bufferAttr;
    char sceneType2[] = "SCENE_MUSIC";
    result = EffectChainManagerProcess(sceneType2, &bufferAttr);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerProcess API
* @tc.number : EffectChainManagerProcess_002
* @tc.desc   : Test EffectChainManagerProcess interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerProcess_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);
    
    struct BufferAttr bufferAttr;
    char *sceneType2 = nullptr;
    result = EffectChainManagerProcess(sceneType2, &bufferAttr);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerProcess API
* @tc.number : EffectChainManagerProcess_003
* @tc.desc   : Test EffectChainManagerProcess interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerProcess_003, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);
    
    struct BufferAttr bufferAttr;
    char sceneType2[] = "";
    result = EffectChainManagerProcess(sceneType2, &bufferAttr);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerMultichannelUpdate API
* @tc.number : EffectChainManagerMultichannelUpdate_001
* @tc.desc   : Test EffectChainManagerMultichannelUpdate interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerMultichannelUpdate_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    result = EffectChainManagerMultichannelUpdate(sceneType);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerMultichannelUpdate API
* @tc.number : EffectChainManagerMultichannelUpdate_002
* @tc.desc   : Test EffectChainManagerMultichannelUpdate interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerMultichannelUpdate_002, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    result = EffectChainManagerMultichannelUpdate(nullptr);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EffectChainManagerExist API
* @tc.number : EffectChainManagerExist_001
* @tc.desc   : Test EffectChainManagerExist interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerExist_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *effectMode = "EFFECT_DEFAULT";
    const char *spatializationEnabled = "0";
    result = EffectChainManagerExist(sceneType, effectMode, spatializationEnabled);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerExist API
* @tc.number : EffectChainManagerExist_002
* @tc.desc   : Test EffectChainManagerExist interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerExist_002, TestSize.Level1)
{
    const char *sceneType = "";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(ERROR, result);

    const char *effectMode = "";
    const char *spatializationEnabled = "";
    result = EffectChainManagerExist(sceneType, effectMode, spatializationEnabled);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerExist API
* @tc.number : EffectChainManagerExist_003
* @tc.desc   : Test EffectChainManagerExist interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerExist_003, TestSize.Level1)
{
    const char *sceneType = nullptr;
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(ERROR, result);

    const char *effectMode = nullptr;
    const char *spatializationEnabled = nullptr;
    result = EffectChainManagerExist(sceneType, effectMode, spatializationEnabled);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectChainManagerVolumeUpdate API
* @tc.number : EffectChainManagerVolumeUpdate_001
* @tc.desc   : Test EffectChainManagerVolumeUpdate interface.
*/
HWTEST(AudioEffectChainAdapterUnitTest, EffectChainManagerVolumeUpdate_001, TestSize.Level1)
{
    const char *sceneType = "SCENE_MUSIC";
    int32_t result = EffectChainManagerInitCb(sceneType);
    EXPECT_EQ(SUCCESS, result);

    const char *sessionid = "111";
    result = EffectChainManagerVolumeUpdate(sessionid);
    EXPECT_EQ(ERROR, result);
}
} // namespace AudioStandard
} // namespace OHOS