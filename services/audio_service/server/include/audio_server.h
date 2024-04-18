/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_SERVER_H
#define ST_AUDIO_SERVER_H

#include <mutex>
#include <pthread.h>
#include <unordered_map>
#include <queue>
#include <vector>
#include <list>

// #include <map>
#include <pwd.h>
#include <pulse/pulseaudio.h>
#include "securec.h"

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"
#include "system_ability.h"

#include "audio_manager_base.h"
#include "audio_server_death_recipient.h"
#include "audio_system_manager.h"
#include "i_audio_renderer_sink.h"
#include "i_audio_capturer_source.h"
#include "audio_effect_server.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

static const int32_t AUDIO_DUMP_SUCCESS = 0;
static const int32_t AUDIO_DUMP_INIT_ERR = -1;

typedef struct {
    std::string name;
    pa_sample_spec sampleSpec;
} SinkSourceInfo;

typedef struct {
    uint32_t userId;
    uint32_t corked;                   // status
    std::string sessionId;
    std::string sessionStartTime;
    std::string applicationName;
    std::string processId;
    std::string privacyType;
    pa_sample_spec sampleSpec;
}InputOutputInfo;

typedef struct {
    std::vector<SinkSourceInfo> sinkDevices;
    std::vector<SinkSourceInfo> sourceDevices;
    std::vector<InputOutputInfo> sinkInputs;
    std::vector<InputOutputInfo> sourceOutputs;
} StreamData;

class AudioServer : public SystemAbility, public AudioManagerStub, public IAudioSinkCallback, IAudioSourceCallback {
    DECLARE_SYSTEM_ABILITY(AudioServer);
public:
    DISALLOW_COPY_AND_MOVE(AudioServer);
    explicit AudioServer(int32_t systemAbilityId, bool runOnCreate = true);
    ~AudioServer();
    void OnDump() override;
    void OnStart() override;
    void OnStop() override;
    int32_t OffloadDrain() override;
                
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    bool LoadAudioEffectLibraries(std::vector<Library> libraries, std::vector<Effect> effects,
        std::vector<Effect>& successEffectList) override;
    bool CreatePlaybackCapturerManager() override;
    bool CreateEffectChainManager(std::vector<EffectChain> &effectChains,
        std::unordered_map<std::string, std::string> &effectMap,
        std::unordered_map<std::string, std::string> &enhanceMap) override;
    bool SetOutputDeviceSink(int32_t deviceType, std::string &sinkName) override;
    int32_t SetMicrophoneMute(bool isMute) override;
    int32_t SetVoiceVolume(float volume) override;
    int32_t OffloadSetVolume(float volume) override;
    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeOutputDevice,
        DeviceType activeInputDevice) override;
    static void *paDaemonThread(void *arg);
    int32_t SetExtraParameters(const std::string& key,
        const std::vector<std::pair<std::string, std::string>>& kvpairs) override;
    void SetAudioParameter(const std::string& key, const std::string& value) override;
    void SetAudioParameter(const std::string& networkId, const AudioParamKey key, const std::string& condition,
        const std::string& value) override;
    int32_t GetExtraParameters(const std::string &mainKey, const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result) override;
    const std::string GetAudioParameter(const std::string &key) override;
    const std::string GetAudioParameter(const std::string& networkId, const AudioParamKey key,
        const std::string& condition) override;
    uint64_t GetTransactionId(DeviceType deviceType, DeviceRole deviceRole) override;
    int32_t UpdateActiveDeviceRoute(DeviceType type, DeviceFlag flag) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;

    void NotifyDeviceInfo(std::string networkId, bool connected) override;

    int32_t CheckRemoteDeviceState(std::string networkId, DeviceRole deviceRole, bool isStartDevice) override;

    sptr<IRemoteObject> CreateAudioProcess(const AudioProcessConfig &config) override;

    // ISinkParameterCallback
    void OnAudioSinkParamChange(const std::string &netWorkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override;

    // IAudioSourceCallback
    void OnWakeupClose() override;
    void OnCapturerState(bool isActive) override;
    void OnAudioSourceParamChange(const std::string &netWorkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override;

    int32_t SetParameterCallback(const sptr<IRemoteObject>& object) override;

    int32_t RegiestPolicyProvider(const sptr<IRemoteObject> &object) override;

    int32_t SetWakeupSourceCallback(const sptr<IRemoteObject>& object) override;

    void RequestThreadPriority(uint32_t tid, std::string bundleName) override;

    int32_t SetSupportStreamUsage(std::vector<int32_t> usage) override;

    int32_t SetCaptureSilentState(bool state) override;
    
    int32_t GetCapturePresentationPosition(const std::string& deviceClass, uint64_t& frames, int64_t& timeSec,
        int64_t& timeNanoSec) override;

    int32_t GetRenderPresentationPosition(const std::string& deviceClass, uint64_t& frames, int64_t& timeSec,
        int64_t& timeNanoSec) override;

    int32_t OffloadGetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    
    int32_t OffloadSetBufferSize(uint32_t sizeMs) override;

    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState) override;

    int32_t NotifyStreamVolumeChanged(AudioStreamType streamType, float volume) override;

    int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType) override;

    int32_t ResetRouteForDisconnect(DeviceType type) override;

    uint32_t GetEffectLatency(const std::string &sessionId) override;

    float GetMaxAmplitude(bool isOutputDevice, int32_t deviceType) override;

    void UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer) override;

    // for hidump
    void ArgDataDump(std::string &dumpString, std::queue<std::u16string> argQue);
    void ServerDataDump(std::string &dumpString);
    void HelpInfoDump(std::string &dumpString);
    void InitDumpFuncMap();
    void PlaybackSinkDump(std::string &dumpString);
    void RecordSourceDump(std::string &dumpString);
    void HDFModulesDump(std::string &dumpString);
    void PolicyHandlerDump(std::string &dumpString);
    void GetPaInfo();
    int32_t PaInitialize();
    void ResetPAAudioDump();
    static bool IsEndWith(const std::string &mainStr, const std::string &toMatch);
    static bool IsValidModule (const std::string moduleName);
    static void PAContextStateCb(pa_context *context, void *userdata);
    static void PASinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void PASinkInputInfoCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
    static void PASourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata);
    static void PASourceOutputInfoCallback(pa_context *c, const pa_source_output_info *i, int eol, void *userdata);

protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    bool VerifyClientPermission(const std::string &permissionName,
        Security::AccessToken::AccessTokenID tokenId = Security::AccessToken::INVALID_TOKENID);
    bool PermissionChecker(const AudioProcessConfig &config);
    bool CheckPlaybackPermission(Security::AccessToken::AccessTokenID tokenId, const StreamUsage streamUsage);
    bool CheckRecorderPermission(Security::AccessToken::AccessTokenID tokenId, const SourceType sourceType,
        int32_t appUid);
    bool CheckVoiceCallRecorderPermission(Security::AccessToken::AccessTokenID tokenId);

    void AudioServerDied(pid_t pid);
    void RegisterPolicyServerDeathRecipient();
    void RegisterAudioCapturerSourceCallback();
    int32_t SetIORoute(DeviceType type, DeviceFlag flag);
    bool CheckAndPrintStacktrace(const std::string &key);
    const std::string GetDPParameter(const std::string &condition);
    const std::string GetUsbParameter();

private:
    static constexpr int32_t MEDIA_SERVICE_UID = 1013;
    static constexpr int32_t MAX_VOLUME = 15;
    static constexpr int32_t MIN_VOLUME = 0;
    static std::unordered_map<int, float> AudioStreamVolumeMap;
    static std::map<std::string, std::string> audioParameters;
    static std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;

    int32_t audioUid_ = 1041;
    pthread_t m_paDaemonThread;
    AudioScene audioScene_ = AUDIO_SCENE_DEFAULT;
    bool isAudioCapturerSourcePrimaryStarted_ = false;
    std::shared_ptr<AudioParameterCallback> audioParamCb_;
    std::shared_ptr<WakeUpSourceCallback> wakeupCallback_;
    std::mutex audioParamCbMtx_;
    std::mutex setWakeupCloseCallbackMutex_;
    std::mutex audioParameterMutex_;
    std::mutex audioSceneMutex_;
    bool isGetProcessEnabled_ = false;
    std::unique_ptr<AudioEffectServer> audioEffectServer_;
    //for dump
    using DumpFunc = void(AudioServer::*)(std::string &dumpString);
    std::map<std::u16string, DumpFunc> dumpFuncMap;
    pa_threaded_mainloop *mainLoop;
    pa_mainloop_api *api;
    pa_context *context;
    std::mutex ctrlMutex_;

    bool isMainLoopStarted_;
    bool isContextConnected_;
    StreamData streamData_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SERVER_H
