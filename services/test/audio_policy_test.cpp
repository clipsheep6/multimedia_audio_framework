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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <securec.h>
#include <unistd.h>

#include "audio_errors.h"
#include "audio_system_manager.h"
#include "audio_log.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace AudioPolicyTest {
    const int FIRST_ARG = 1;
    const int SECOND_ARG = 2;
    const int THIRD_ARG = 3;
    const int FOURTH_ARG = 4;
    const int FIFTH_ARG = 5;
    const int SIXTH_ARG = 6;
    const int SEVENTH_ARG = 7;
    const int EIGHTH_ARG = 8;
    const int NINTH_ARG = 9;
    const int TENTH_ARG =10;
    const int ELEVENTH_ARG = 11;
    const int TWELFTH_ARG = 12;
    const int OPT_ARG_BASE = 10;
    const int OPT_SHORT_LEN = 3;
}

static void PrintUsage(void)
{
    cout << "NAME" << endl << endl;
    cout << "\taudio_policy_test - Audio Policy Test " << endl << endl;
    cout << "SYNOPSIS" << endl << endl;
    cout << "\t#include <audio_system_manager.h>" << endl << endl;
    cout << "\t./audio_policy_test [OPTIONS]..." << endl << endl;
    cout << "DESCRIPTION" << endl << endl;
    cout << "\tControls audio volume, audio routing, audio mute" << endl << endl;
    cout << "-V\n\tSets Volume for streams, -S to setStream" << endl << endl;
    cout << "-v\n\tGets Volume for streams, -S to setStream" << endl << endl;
    cout << "-S\n\tSet stream type" << endl << endl;
    cout << "\tSupported Streams are" << endl << endl;
    cout << "\t4\tMUSIC" << endl << endl;
    cout << "\t3\tRING" << endl << endl;
    cout << "-D\n\tSets Device Active" << endl << endl;
    cout << "\tSupported Devices are" << endl << endl;
    cout << "\t2\tSPEAKER" << endl << endl;
    cout << "\t7\tBLUETOOTH_SCO" << endl << endl;
    cout << "-d\n\tGets Device Active" << endl << endl;
    cout << "-M\n\tSets Mute for streams, -S to setStream" << endl << endl;
    cout << "-m\n\tGets Mute for streams, -S to setStream" << endl << endl;
    cout << "-U\n\t Mutes the Microphone" << endl << endl;
    cout << "-u\n\t Checks if the Microphone is muted " << endl << endl;
    cout << "-R\n\tSets RingerMode" << endl << endl;
    cout << "-r\n\tGets RingerMode status" << endl << endl;
    cout << "-C\n\tSets AudioScene" << endl << endl;
    cout << "-c\n\tGets AudioScene status" << endl << endl;
    cout << "-s\n\tGet Stream Status" << endl << endl;
    cout << "AUTHOR" << endl << endl;
    cout << "\tWritten by Sajeesh Sidharthan and Anurup M" << endl << endl;
}


static void ShowAudioDeviceDescriptorsVector(std::vector<sptr<AudioDeviceDescriptor>> &audioDeviceDescriptorsVector)
{
    int vectorLen = audioDeviceDescriptorsVector.size();
    for (int i = 0; i < vectorLen; i ++)
    {
        cout << "------------show Audio Device Descriptors Vector------------" << endl;
        cout << "deviceRole: "       << audioDeviceDescriptorsVector[i]->deviceRole_ << endl;
        cout << "deviceType: "       << audioDeviceDescriptorsVector[i]->deviceType_ << endl;
        cout << "networkId: "        << audioDeviceDescriptorsVector[i]->networkId_ << endl;
        cout << "interruptGroupId: " << audioDeviceDescriptorsVector[i]->interruptGroupId_ << endl;
        cout << "volumeGroupId: "    << audioDeviceDescriptorsVector[i]->volumeGroupId_ << endl;
    }
}

static void ShowAudioRendererFilter(sptr<AudioRendererFilter> audioRendererFilter)
{
    cout << "------------show Audio Renderer Filter------------" << endl;
    cout << "uid: "           << audioRendererFilter->uid << endl;
    cout << "contentType: "   << audioRendererFilter->rendererInfo.contentType << endl;
    cout << "streamUsage: "   << audioRendererFilter->rendererInfo.streamUsage << endl;
    cout << "rendererFlags: " << audioRendererFilter->rendererInfo.rendererFlags << endl;
    cout << "streamId: "      << audioRendererFilter->streamId << endl;
}


static void HandleGetDevices(char option)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    if (option == 'g')
    {
        DeviceFlag deviceFlag = DeviceFlag::OUTPUT_DEVICES_FLAG;
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptorsVector;
        audioDeviceDescriptorsVector = audioSystemMgr->GetDevices(deviceFlag);
        cout << "GetDevices(Output Devices) Result: " << endl;
        ShowAudioDeviceDescriptorsVector(audioDeviceDescriptorsVector);
    }
    else
    {
        DeviceFlag deviceFlag = DeviceFlag::INPUT_DEVICES_FLAG;
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptorsVector;
        audioDeviceDescriptorsVector = audioSystemMgr->GetDevices(deviceFlag);
        cout << "GetDevices(Input Devices) Result: " << endl;
        ShowAudioDeviceDescriptorsVector(audioDeviceDescriptorsVector);
    }
}


static void CallSelectOutputDevice(char option,std::vector<sptr<AudioDeviceDescriptor>> &audioDeviceDescriptorsVector,sptr<AudioRendererFilter> audioRendererFilter)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    if (option == 'o')
    {
        int result = audioSystemMgr->SelectOutputDevice(audioDeviceDescriptorsVector);
        cout << "SelectOutputDevice Result: " << result << endl;
    }
    else
    {
        int result = audioSystemMgr->SelectOutputDevice(audioRendererFilter,audioDeviceDescriptorsVector);
        cout << "SelectOutputDevice by filter Result: " << result << endl;
    }
}

static void HandleSelectOutputDevice(int argc, char* argv[], char opt)
{
    if (argc == AudioPolicyTest::SEVENTH_ARG)
    {
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptorsVector;
        sptr<AudioDeviceDescriptor> audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();

        int32_t intValue;
        intValue = atoi(argv[AudioPolicyTest::SECOND_ARG]);
        audioDeviceDescriptor->deviceRole_ = static_cast<DeviceRole>(intValue);
        intValue = atoi(argv[AudioPolicyTest::THIRD_ARG]);
        audioDeviceDescriptor->deviceType_  = static_cast<DeviceType>(intValue);
        audioDeviceDescriptor->networkId_   = std::string(argv[AudioPolicyTest::FOURTH_ARG]);
        intValue = atoi(argv[AudioPolicyTest::FIFTH_ARG]);
        audioDeviceDescriptor->interruptGroupId_ = intValue;
        intValue = atoi(argv[AudioPolicyTest::SIXTH_ARG]);
        audioDeviceDescriptor->volumeGroupId_   = intValue;
        audioDeviceDescriptorsVector.push_back(audioDeviceDescriptor);
        ShowAudioDeviceDescriptorsVector(audioDeviceDescriptorsVector);
        CallSelectOutputDevice(opt,audioDeviceDescriptorsVector,nullptr);
    }
    else if (argc == AudioPolicyTest::TWELFTH_ARG)
    {
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptorsVector;
        sptr<AudioDeviceDescriptor> audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();

        int32_t intValue;
        intValue = atoi(argv[AudioPolicyTest::SECOND_ARG]);
        audioDeviceDescriptor->deviceRole_ = static_cast<DeviceRole>(intValue);
        intValue = atoi(argv[AudioPolicyTest::THIRD_ARG]);
        audioDeviceDescriptor->deviceType_  = static_cast<DeviceType>(intValue);
        audioDeviceDescriptor->networkId_   = std::string(argv[AudioPolicyTest::FOURTH_ARG]);
        intValue = atoi(argv[AudioPolicyTest::FIFTH_ARG]);
        audioDeviceDescriptor->interruptGroupId_ = intValue;
        intValue = atoi(argv[AudioPolicyTest::SIXTH_ARG]);
        audioDeviceDescriptor->volumeGroupId_   = intValue;
        audioDeviceDescriptorsVector.push_back(audioDeviceDescriptor);
        ShowAudioDeviceDescriptorsVector(audioDeviceDescriptorsVector);

        sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
        intValue = atoi(argv[AudioPolicyTest::SEVENTH_ARG]);
        audioRendererFilter->uid = static_cast<DeviceRole>(intValue);
        intValue = atoi(argv[AudioPolicyTest::EIGHTH_ARG]);
        audioRendererFilter->rendererInfo.contentType = static_cast<ContentType>(intValue);
        intValue = atoi(argv[AudioPolicyTest::NINTH_ARG]);
        audioRendererFilter->rendererInfo.streamUsage = static_cast<StreamUsage>(intValue);
        intValue = atoi(argv[AudioPolicyTest::TENTH_ARG]);
        audioRendererFilter->rendererInfo.rendererFlags = intValue;
        intValue = atoi(argv[AudioPolicyTest::ELEVENTH_ARG]);
        audioRendererFilter->streamId = intValue;
        ShowAudioRendererFilter(audioRendererFilter);
        CallSelectOutputDevice(opt,audioDeviceDescriptorsVector,audioRendererFilter);
    }
    else
    {
        cout << "------------Please input right arg Num------------" << endl;
        cout << "The arg order: " << endl;
        cout << "audioDeviceDescriptorsVector(deviceRole, deviceType, networkId, interruptGroupId, volumeGroupId)" << endl;
        cout << "audioRendererFilter(uid,contentType,streamUsage,rendererFlags,streamId)" << endl;
    }
}

static void CallSelectInputDevice(char option,std::vector<sptr<AudioDeviceDescriptor>> &audioDeviceDescriptorsVector,sptr<AudioCapturerFilter> audioCapturerFilter)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    if (option == 'i')
    {
        int result = audioSystemMgr->SelectIntputDevice(audioDeviceDescriptorsVector);
        cout << "SelectInputDevice Result: " << result << endl;
    }
    else
    {
        int result = audioSystemMgr->SelectIntputDevice(audioCapturerFilter,audioDeviceDescriptorsVector);
        cout << "SelectInputDevice by filter Result: " << result << endl;
    }
}

static void HandleSelectInputDevice(int argc, char* argv[], char opt)
{
    if (argc == AudioPolicyTest::SEVENTH_ARG)
    {
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptorsVector;
        sptr<AudioDeviceDescriptor> audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();

        int32_t intValue;
        intValue = atoi(argv[AudioPolicyTest::SECOND_ARG]);
        audioDeviceDescriptor->deviceRole_ = static_cast<DeviceRole>(intValue);
        intValue = atoi(argv[AudioPolicyTest::THIRD_ARG]);
        audioDeviceDescriptor->deviceType_  = static_cast<DeviceType>(intValue);
        audioDeviceDescriptor->networkId_   = std::string(argv[AudioPolicyTest::FOURTH_ARG]);
        intValue = atoi(argv[AudioPolicyTest::FIFTH_ARG]);
        audioDeviceDescriptor->interruptGroupId_ = intValue;
        intValue = atoi(argv[AudioPolicyTest::SIXTH_ARG]);
        audioDeviceDescriptor->volumeGroupId_   = intValue;
        audioDeviceDescriptorsVector.push_back(audioDeviceDescriptor);
        ShowAudioDeviceDescriptorsVector(audioDeviceDescriptorsVector);
        CallSelectInputDevice(opt,audioDeviceDescriptorsVector,nullptr);
    }
    else if (argc == AudioPolicyTest::EIGHTH_ARG)
    {
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptorsVector;
        sptr<AudioDeviceDescriptor> audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();

        int32_t intValue;
        intValue = atoi(argv[AudioPolicyTest::SECOND_ARG]);
        audioDeviceDescriptor->deviceRole_ = static_cast<DeviceRole>(intValue);
        intValue = atoi(argv[AudioPolicyTest::THIRD_ARG]);
        audioDeviceDescriptor->deviceType_  = static_cast<DeviceType>(intValue);
        audioDeviceDescriptor->networkId_   = std::string(argv[AudioPolicyTest::FOURTH_ARG]);
        intValue = atoi(argv[AudioPolicyTest::FIFTH_ARG]);
        audioDeviceDescriptor->interruptGroupId_ = intValue;
        intValue = atoi(argv[AudioPolicyTest::SIXTH_ARG]);
        audioDeviceDescriptor->volumeGroupId_   = intValue;
        audioDeviceDescriptorsVector.push_back(audioDeviceDescriptor);
        ShowAudioDeviceDescriptorsVector(audioDeviceDescriptorsVector);

        sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
        intValue = atoi(argv[AudioPolicyTest::SEVENTH_ARG]);
        audioCapturerFilter->uid = intValue;
        cout << "------------show Audio Capturer Filter------------" << endl;
        cout << "uid: " << audioCapturerFilter->uid << endl;
        CallSelectInputDevice(opt,audioDeviceDescriptorsVector,audioCapturerFilter);
    }
    else
    {
        cout << "------------Please input right arg Num------------" << endl;
        cout << "The arg order: " << endl;
        cout << "audioDeviceDescriptorsVector(deviceRole, deviceType, networkId, interruptGroupId, volumeGroupId)" << endl;
        cout << "audioCapturerFilter(uid)" << endl;
    }
}

static void HandleVolume(int streamType, char option)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    if (option == 'v') {
        float volume = audioSystemMgr->GetVolume(static_cast<AudioVolumeType>(streamType));
        cout << "Get Volume : " << volume << endl;
    } else {
        float volume = strtof(optarg, nullptr);
        cout << "Set Volume : " << volume << endl;
        int32_t result = audioSystemMgr->SetVolume(static_cast<AudioVolumeType>(streamType),
                                                   volume);
        cout << "Set Volume Result: " << result << endl;
    }
}

static void HandleMute(int streamType, char option)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    if (option == 'm') {
        bool muteStatus = audioSystemMgr->IsStreamMute(static_cast<AudioVolumeType>(streamType));
        cout << "Get Mute : " << muteStatus << endl;
    } else {
        int mute = strtol(optarg, nullptr, AudioPolicyTest::OPT_ARG_BASE);
        cout << "Set Mute : " << mute << endl;
        int32_t result = audioSystemMgr->SetMute(static_cast<AudioVolumeType>(streamType),
            (mute) ? true : false);
        cout << "Set Mute Result: " << result << endl;
    }
}

static void HandleMicMute(char option)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    if (option == 'u') {
        bool muteStatus = audioSystemMgr->IsMicrophoneMute();
        cout << "Is Mic Mute : " << muteStatus << endl;
    } else {
        int mute = strtol(optarg, nullptr, AudioPolicyTest::OPT_ARG_BASE);
        cout << "Set Mic Mute : " << mute << endl;
        int32_t result = audioSystemMgr->SetMicrophoneMute((mute) ? true : false);
        cout << "Set Mic Mute Result: " << result << endl;
    }
}

static void SetStreamType(int &streamType)
{
    streamType = strtol(optarg, nullptr, AudioPolicyTest::OPT_ARG_BASE);
    cout << "Set Stream : " << streamType << endl;
}

static void IsStreamActive()
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    int streamType = strtol(optarg, nullptr, AudioPolicyTest::OPT_ARG_BASE);
    cout << "Stream Active: " << audioSystemMgr->IsStreamActive(
        static_cast<AudioVolumeType>(streamType)) << endl;
}

static void SetDeviceActive(int argc, char *argv[])
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    int active = -1;
    int device = strtol(optarg, nullptr, AudioPolicyTest::OPT_ARG_BASE);
    cout << "Set Device : " << device << endl;

    if (optind < argc && *argv[optind] != '-') {
        active = strtol(argv[optind], nullptr, AudioPolicyTest::OPT_ARG_BASE);
        optind++;
    }
    cout << "Active : " << active << endl << endl;

    int32_t result = audioSystemMgr->SetDeviceActive(ActiveDeviceType(device),
        (active) ? true : false);
    cout << "Set DeviceActive Result: " << result << endl;
}

static void IsDeviceActive()
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    int device = strtol(optarg, nullptr, AudioPolicyTest::OPT_ARG_BASE);
    bool devActiveStatus = audioSystemMgr->IsDeviceActive(ActiveDeviceType(device));
    cout << "GetDevice Active : " << devActiveStatus << endl;
}

static void HandleRingerMode(char option)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    if (option == 'r') {
        int ringMode = static_cast<int32_t>(audioSystemMgr->GetRingerMode());
        cout << "Get Ringer Mode : " << ringMode << endl;
    } else {
        int ringMode = strtol(optarg, nullptr, AudioPolicyTest::OPT_ARG_BASE);
        cout << "Set Ringer Mode : " << ringMode << endl;
        audioSystemMgr->SetRingerMode(static_cast<AudioRingerMode>(ringMode));
    }
}

static void HandleAudioScene(char option)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    if (option == 'c') {
        int scene = static_cast<int32_t>(audioSystemMgr->GetAudioScene());
        cout << "Get Audio Scene : " << scene << endl;
    } else {
        int scene = strtol(optarg, nullptr, AudioPolicyTest::OPT_ARG_BASE);
        cout << "Set Audio Scene : " << scene << endl;
        audioSystemMgr->SetAudioScene(static_cast<AudioScene>(scene));
    }
}

static void NoValueError()
{
    char option[AudioPolicyTest::OPT_SHORT_LEN];
    cout << "option ";
    int len = snprintf_s(option, sizeof(option), sizeof(option) - 1, "-%c", optopt);
    if (len <= 0) {
        cout << "NoValueError: snprintf_s error : buffer allocation fails";
        return;
    }

    cout << option << " needs a value" << endl << endl;
    PrintUsage();
}

static void UnknownOptionError()
{
    char option[AudioPolicyTest::OPT_SHORT_LEN];
    int len = snprintf_s(option, sizeof(option), sizeof(option) - 1, "-%c", optopt);
    if (len <= 0) {
        cout << "unknown option: snprintf_s error : buffer allocation fails";
        return;
    }
    cout << "unknown option: " << option << endl << endl;
    PrintUsage();
}

int main(int argc, char* argv[])
{
    int opt = 0;
    if (((argc >= AudioPolicyTest::SECOND_ARG) && !strcmp(argv[AudioPolicyTest::FIRST_ARG], "--help")) ||
        (argc == AudioPolicyTest::FIRST_ARG)) {
        PrintUsage();
        return ERR_INVALID_PARAM;
    }

    int streamType = static_cast<int32_t>(AudioVolumeType::STREAM_MUSIC);
    while ((opt = getopt(argc, argv, ":V:U:S:D:M:R:C:d:s:vmrucOoIiGg")) != -1) {
        switch (opt) {
            case 'G':
            case 'g':
                HandleGetDevices(opt);
                break;
            case 'O':
            case 'o':
                HandleSelectOutputDevice(argc,argv,opt);
                break;
            case 'I':
            case 'i':
                HandleSelectInputDevice(argc,argv,opt);
                break;
            case 'V':
            case 'v':
                HandleVolume(streamType, opt);
                break;
            case 'M':
            case 'm':
                HandleMute(streamType, opt);
                break;
            case 'U':
            case 'u':
                HandleMicMute(opt);
                break;
            case 'S':
                SetStreamType(streamType);
                break;
            case 's':
                IsStreamActive();
                break;
            case 'D':
                SetDeviceActive(argc, argv);
                break;
            case 'd':
                IsDeviceActive();
                break;
            case 'R':
            case 'r':
                HandleRingerMode(opt);
                break;
            case 'C':
            case 'c':
                HandleAudioScene(opt);
                break;
            case ':':
                NoValueError();
                break;
            case '?':
                UnknownOptionError();
                break;
            default:
                break;
        }
    }

    return 0;
}
