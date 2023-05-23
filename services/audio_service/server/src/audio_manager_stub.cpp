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

#include "audio_manager_base.h"
#include "audio_system_manager.h"
#include "audio_log.h"
#include "i_audio_process.h"
#include "audio_effect_server.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
static void LoadEffectLibrariesReadData(vector<Library>& libList, vector<Effect>& effectList, MessageParcel &data,
                                        int32_t countLib, int32_t countEff)
{
    int32_t i;
    for (i = 0; i < countLib; i++) {
        string libName = data.ReadString();
        string libPath = data.ReadString();
        libList.push_back({libName, libPath});
    }
    for (i = 0; i < countEff; i++) {
        string effectName = data.ReadString();
        string libName = data.ReadString();
        effectList.push_back({effectName, libName});
    }
}

static void LoadEffectLibrariesWriteReply(vector<Effect>& successEffectList, MessageParcel &reply)
{
    reply.WriteInt32(successEffectList.size());
    for (Effect effect: successEffectList) {
        reply.WriteString(effect.name);
        reply.WriteString(effect.libraryName);
    }
}

int AudioManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    AUDIO_DEBUG_LOG("OnRemoteRequest, cmd = %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        AUDIO_ERR_LOG("AudioManagerStub: ReadInterfaceToken failed");
        return -1;
    }

    switch (code) {
        case SET_AUDIO_PARAMETER: {
            AUDIO_DEBUG_LOG("SET_AUDIO_PARAMETER AudioManagerStub");
            const std::string key = data.ReadString();
            const std::string value = data.ReadString();
            AUDIO_DEBUG_LOG("SET_AUDIO_PARAMETER key-value pair from client= %{public}s, %{public}s",
                            key.c_str(), value.c_str());
            SetAudioParameter(key, value);
            return AUDIO_OK;
        }
        case GET_AUDIO_PARAMETER: {
            AUDIO_DEBUG_LOG("GET_AUDIO_PARAMETER AudioManagerStub");
            const std::string key = data.ReadString();
            AUDIO_DEBUG_LOG("GET_AUDIO_PARAMETER key received from client= %{public}s", key.c_str());
            const std::string value = GetAudioParameter(key);
            reply.WriteString(value);
            return AUDIO_OK;
        }
        case RETRIEVE_COOKIE: {
            AUDIO_DEBUG_LOG("RETRIEVE_COOKIE AudioManagerStub");
            int32_t size = 0;
            const char *cookieInfo = RetrieveCookie(size);
            reply.WriteInt32(size);
            if (size > 0) {
                AUDIO_DEBUG_LOG("cookie received from server");
                reply.WriteRawData(static_cast<const void *>(cookieInfo), size);
                free((void *)cookieInfo);
                cookieInfo = nullptr;
            }
            return AUDIO_OK;
        }
        case GET_TRANSACTION_ID: {
            AUDIO_DEBUG_LOG("GET_TRANSACTION_ID AudioManagerStub");
            DeviceType deviceType = (static_cast<DeviceType>(data.ReadInt32()));
            DeviceRole deviceRole = (static_cast<DeviceRole>(data.ReadInt32()));
            uint64_t transactionId = GetTransactionId(deviceType, deviceRole);

            reply.WriteUint64(transactionId);

            return AUDIO_OK;
        }
        case SET_MICROPHONE_MUTE: {
            AUDIO_DEBUG_LOG("SET_MICROPHONE_MUTE AudioManagerStub");
            bool isMute = data.ReadBool();
            AUDIO_DEBUG_LOG("SET_MICROPHONE_MUTE isMute value from client= %{public}d", isMute);
            int32_t result = SetMicrophoneMute(isMute);
            reply.WriteInt32(result);
            return AUDIO_OK;
        }
        case IS_MICROPHONE_MUTE: {
            AUDIO_DEBUG_LOG("IS_MICROPHONE_MUTE AudioManagerStub");
            bool isMute = IsMicrophoneMute();
            reply.WriteBool(isMute);
            return AUDIO_OK;
        }
        case SET_AUDIO_SCENE: {
            AUDIO_DEBUG_LOG("SET_AUDIO_SCENE AudioManagerStub");
            AudioScene audioScene = (static_cast<AudioScene>(data.ReadInt32()));
            DeviceType activeDevice = (static_cast<DeviceType>(data.ReadInt32()));
            int32_t result = SetAudioScene(audioScene, activeDevice);
            reply.WriteInt32(result);
            return AUDIO_OK;
        }
        case UPDATE_ROUTE_REQ: {
            AUDIO_DEBUG_LOG("UPDATE_ROUTE_REQ AudioManagerStub");
            DeviceType type = static_cast<DeviceType>(data.ReadInt32());
            DeviceFlag flag = static_cast<DeviceFlag>(data.ReadInt32());
            int32_t ret = UpdateActiveDeviceRoute(type, flag);
            reply.WriteInt32(ret);
            return AUDIO_OK;
        }
        case SET_PARAMETER_CALLBACK: {
            AUDIO_DEBUG_LOG("SET_PARAMETER_CALLBACK AudioManagerStub");
            sptr<IRemoteObject> object = data.ReadRemoteObject();
            if (object == nullptr) {
                AUDIO_ERR_LOG("AudioManagerStub: SET_PARAMETER_CALLBACK obj is null");
                return AUDIO_ERR;
            }
            int32_t result = SetParameterCallback(object);
            reply.WriteInt32(result);
            return AUDIO_OK;
        }
        case SET_REMOTE_AUDIO_PARAMETER: {
            AUDIO_DEBUG_LOG("SET_AUDIO_PARAMETER AudioManagerStub");
            const std::string networkId = data.ReadString();
            AudioParamKey key = static_cast<AudioParamKey>(data.ReadInt32());
            const std::string condtion = data.ReadString();
            const std::string value = data.ReadString();
            AUDIO_DEBUG_LOG("SET_AUDIO_PARAMETER key-value pair from client= %{public}d, %{public}s",
                key, value.c_str());
            SetAudioParameter(networkId, key, condtion, value);
            return AUDIO_OK;
        }
        case GET_REMOTE_AUDIO_PARAMETER: {
            AUDIO_DEBUG_LOG("GET_AUDIO_PARAMETER AudioManagerStub");
            const std::string networkId = data.ReadString();
            AudioParamKey key = static_cast<AudioParamKey>(data.ReadInt32());
            const std::string condition = data.ReadString();
            AUDIO_DEBUG_LOG("GET_AUDIO_PARAMETER key received from client= %{public}d", key);
            const std::string value = GetAudioParameter(networkId, key, condition);
            reply.WriteString(value);
            return AUDIO_OK;
        }
        case NOTIFY_DEVICE_INFO: {
            AUDIO_DEBUG_LOG("NOTIFY_DEVICE_INFO AudioManagerStub");
            const std::string networkId = data.ReadString();
            const bool connected = data.ReadBool();
            NotifyDeviceInfo(networkId, connected);
            return AUDIO_OK;
        }
        case CHECK_REMOTE_DEVICE_STATE: {
            AUDIO_DEBUG_LOG("CHECK_REMOTE_DEVICE_STATE AudioManagerStub");
            std::string networkId = data.ReadString();
            DeviceRole deviceRole = static_cast<DeviceRole>(data.ReadInt32());
            bool isStartDevice = data.ReadBool();
            int32_t result = CheckRemoteDeviceState(networkId, deviceRole, isStartDevice);
            reply.WriteInt32(result);
            return AUDIO_OK;
        }
        case SET_VOICE_VOLUME: {
            AUDIO_DEBUG_LOG("SET_VOICE_VOLUME AudioManagerStub");
            const float volume = data.ReadFloat();
            int32_t result = SetVoiceVolume(volume);
            reply.WriteInt32(result);
            return AUDIO_OK;
        }
        case SET_AUDIO_MONO_STATE: {
            AUDIO_DEBUG_LOG("SET_AUDIO_MONO_STATE AudioManagerStub");
            bool audioMonoState = data.ReadBool();
            SetAudioMonoState(audioMonoState);
            return AUDIO_OK;
        }
        case SET_AUDIO_BALANCE_VALUE: {
            AUDIO_DEBUG_LOG("SET_AUDIO_BALANCE_VALUE AudioManagerStub");
            float audioBalanceValue = data.ReadFloat();
            SetAudioBalanceValue(audioBalanceValue);
            return AUDIO_OK;
        }
        case CREATE_AUDIOPROCESS: {
            AUDIO_INFO_LOG("CREATE_AUDIOPROCESS AudioManagerStub");
            AudioProcessConfig config;
            IAudioProcess::ReadConfigFromParcel(config, data);
            sptr<IRemoteObject> process = CreateAudioProcess(config);
            if (process == nullptr) {
                AUDIO_ERR_LOG("CREATE_AUDIOPROCESS AudioManagerStub CreateAudioProcess failed");
                return AUDIO_ERR;
            }
            reply.WriteRemoteObject(process);
            return AUDIO_OK;
        }
        case LOAD_AUDIO_EFFECT_LIBRARIES: {
            AUDIO_ERR_LOG("cjw: LOAD_AUDIO_EFFECT_LIBRARIES");
            vector<Library> libList = {};
            vector<Effect> effectList = {};
            int32_t countLib = data.ReadInt32();
            int32_t countEff = data.ReadInt32();
            if ((countLib < 0) || (countLib > AUDIO_EFFECT_COUNT_UPPER_LIMIT) ||
                (countEff < 0) || (countEff > AUDIO_EFFECT_COUNT_UPPER_LIMIT)) {
                AUDIO_ERR_LOG("LOAD_AUDIO_EFFECT_LIBRARIES read data failed");
                return AUDIO_ERR;
            }
            LoadEffectLibrariesReadData(libList, effectList, data, countLib, countEff);
            if (countLib > 0) {
                //load lib and reply success list
                vector<Effect> successEffectList = {};
                bool loadSuccess = LoadAudioEffectLibraries(libList, effectList, successEffectList);
                if (!loadSuccess) {
                    AUDIO_ERR_LOG("Load audio effect libraries failed, please check log");
                    return AUDIO_ERR;
                }
                LoadEffectLibrariesWriteReply(successEffectList, reply);
            }
            return AUDIO_OK;
        }
        case REQUEST_THREAD_PRIORITY: {
            uint32_t tid = data.ReadUint32();
            string bundleName = data.ReadString();
            RequestThreadPriority(tid, bundleName);
            return AUDIO_OK;
        }
		case CREATE_AUDIO_EFFECT_CHAIN_MANAGER: {
            AUDIO_ERR_LOG("cjw: CREATE_AUDIO_EFFECT_CHAIN_MANAGER");
            int i;
            vector<EffectChain> effectChains = {};
            vector<int32_t> countEffect = {};
            int32_t countEffectChains = data.ReadInt32();
            for (i = 0; i < countEffectChains; i++) {
                countEffect.emplace_back(data.ReadInt32());
            }

            for (int32_t count: countEffect) {
                EffectChain effectChain;
                effectChain.name = data.ReadString();
                for (i = 0; i < count; i++) {
                    effectChain.apply.emplace_back(data.ReadString());
                }
                effectChains.emplace_back(effectChain);
            }

            unordered_map<string, string> sceneTypeToEffectChainNameMap;
            string key, value;
            int32_t mapSize = data.ReadInt32();
            AUDIO_INFO_LOG("cjw: mapSize: %{public}d", mapSize);
            for (i = 0; i < mapSize; i++) {
                key = data.ReadString();
                value = data.ReadString();
                sceneTypeToEffectChainNameMap[key] = value;
            }

            bool createSuccess = CreateEffectChainManager(effectChains, sceneTypeToEffectChainNameMap);
            if(!createSuccess){
                AUDIO_ERR_LOG("create audio effect chain manager failed, please check log");
                return AUDIO_ERR;
            }

            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioManagerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS
