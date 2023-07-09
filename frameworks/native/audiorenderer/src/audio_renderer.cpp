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

#include <sstream>

#include "audio_renderer.h"

#include "audio_log.h"
#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_stream.h"
#include "audio_renderer_private.h"
#include "audio_utils.h"
#ifdef OHCORE
#include "audio_renderer_gateway.h"
#endif

namespace OHOS {
namespace AudioStandard {

static const int32_t MAX_VOLUME_LEVEL = 15;
static const int32_t CONST_FACTOR = 100;

static float VolumeToDb(int32_t volumeLevel)
{
    float value = static_cast<float>(volumeLevel) / MAX_VOLUME_LEVEL;
    float roundValue = static_cast<int>(value * CONST_FACTOR);

    return static_cast<float>(roundValue) / CONST_FACTOR;
}

std::mutex AudioRenderer::createRendererMutex_;

AudioRenderer::~AudioRenderer() = default;
AudioRendererPrivate::~AudioRendererPrivate()
{
    RendererState state = GetStatus();
    if (state != RENDERER_RELEASED && state != RENDERER_NEW) {
        Release();
    }
#ifdef DUMP_CLIENT_PCM
    if (dcp_) {
        fclose(dcp_);
        dcp_ = nullptr;
    }
#endif
}

int32_t AudioRenderer::CheckMaxRendererInstances()
{
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    AUDIO_INFO_LOG("Audio current renderer change infos size: %{public}zu", audioRendererChangeInfos.size());
    int32_t maxRendererInstances = AudioPolicyManager::GetInstance().GetMaxRendererInstances();
    CHECK_AND_RETURN_RET_LOG(audioRendererChangeInfos.size() < static_cast<size_t>(maxRendererInstances), ERR_OVERFLOW,
        "The current number of audio renderer streams is greater than the maximum number of configured instances");

    return SUCCESS;
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(AudioStreamType audioStreamType)
{
    AppInfo appInfo = {};
    return Create(audioStreamType, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(AudioStreamType audioStreamType, const AppInfo &appInfo)
{
    if (audioStreamType == STREAM_MEDIA) {
        audioStreamType = STREAM_MUSIC;
    }
#ifdef OHCORE
    return std::make_unique<AudioRendererGateway>(audioStreamType);
#else
    return std::make_unique<AudioRendererPrivate>(audioStreamType, appInfo);
#endif
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const AudioRendererOptions &rendererOptions)
{
    AppInfo appInfo = {};
    return Create("", rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const AudioRendererOptions &rendererOptions,
    const AppInfo &appInfo)
{
    return Create("", rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const std::string cachePath,
    const AudioRendererOptions &rendererOptions)
{
    AppInfo appInfo = {};
    return Create(cachePath, rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const std::string cachePath,
    const AudioRendererOptions &rendererOptions, const AppInfo &appInfo)
{
    Trace trace("AudioRenderer::Create");
    std::lock_guard<std::mutex> lock(createRendererMutex_);
    CHECK_AND_RETURN_RET_LOG(AudioRenderer::CheckMaxRendererInstances() == SUCCESS, nullptr,
        "Too many renderer instances");
    ContentType contentType = rendererOptions.rendererInfo.contentType;
    CHECK_AND_RETURN_RET_LOG(contentType >= CONTENT_TYPE_UNKNOWN && contentType <= CONTENT_TYPE_ULTRASONIC, nullptr,
                             "Invalid content type");

    StreamUsage streamUsage = rendererOptions.rendererInfo.streamUsage;
    CHECK_AND_RETURN_RET_LOG(streamUsage >= STREAM_USAGE_UNKNOWN &&
        streamUsage <= STREAM_USAGE_VOICE_MODEM_COMMUNICATION, nullptr, "Invalid stream usage");
    if (contentType == CONTENT_TYPE_ULTRASONIC || streamUsage == STREAM_USAGE_SYSTEM ||
        streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        if (!PermissionUtil::VerifySelfPermission()) {
            AUDIO_ERR_LOG("CreateAudioRenderer failed! CONTENT_TYPE_ULTRASONIC or STREAM_USAGE_SYSTEM or "\
                "STREAM_USAGE_VOICE_MODEM_COMMUNICATION: No system permission");
            return nullptr;
        }
    }

    AudioStreamType audioStreamType = AudioStream::GetStreamType(contentType, streamUsage);
#ifdef OHCORE
    auto audioRenderer = std::make_unique<AudioRendererGateway>(audioStreamType);
#else
    auto audioRenderer = std::make_unique<AudioRendererPrivate>(audioStreamType, appInfo);
#endif
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, nullptr, "Failed to create renderer object");
    if (!cachePath.empty()) {
        AUDIO_DEBUG_LOG("Set application cache path");
        audioRenderer->SetApplicationCachePath(cachePath);
    }

    int32_t rendererFlags = rendererOptions.rendererInfo.rendererFlags;
    AUDIO_INFO_LOG("create audiorenderer with usage: %{public}d, content: %{public}d, flags: %{public}d",
        streamUsage, contentType, rendererFlags);

    audioRenderer->rendererInfo_.contentType = contentType;
    audioRenderer->rendererInfo_.streamUsage = streamUsage;
    audioRenderer->rendererInfo_.rendererFlags = rendererFlags;

    AudioPrivacyType privacyType = rendererOptions.privacyType;
    audioRenderer->SetAudioPrivacyType(privacyType);

    AudioRendererParams params;
    params.sampleFormat = rendererOptions.streamInfo.format;
    params.sampleRate = rendererOptions.streamInfo.samplingRate;
    params.channelCount = rendererOptions.streamInfo.channels;
    params.encodingType = rendererOptions.streamInfo.encoding;

    if (audioRenderer->SetParams(params) != SUCCESS) {
        AUDIO_ERR_LOG("SetParams failed in renderer");
        audioRenderer = nullptr;
        return nullptr;
    }

    return audioRenderer;
}

AudioRendererPrivate::AudioRendererPrivate(AudioStreamType audioStreamType, const AppInfo &appInfo)
{
    appInfo_ = appInfo;
    if (!(appInfo_.appPid)) {
        appInfo_.appPid = getpid();
    }

    if (appInfo_.appUid < 0) {
        appInfo_.appUid = static_cast<int32_t>(getuid());
    }

    audioStream_ = std::make_shared<AudioStream>(audioStreamType, AUDIO_MODE_PLAYBACK, appInfo_.appUid);
    if (audioStream_) {
        AUDIO_DEBUG_LOG("AudioRendererPrivate::Audio stream created");
        // Initializing with default values
        rendererInfo_.contentType = CONTENT_TYPE_MUSIC;
        rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    }

    rendererProxyObj_ = std::make_shared<AudioRendererProxyObj>();
    if (!rendererProxyObj_) {
        AUDIO_ERR_LOG("AudioRendererProxyObj Memory Allocation Failed !!");
    }

    audioInterrupt_.audioFocusType.streamType = audioStreamType;
    audioInterrupt_.pid = appInfo_.appPid;
    audioInterrupt_.mode = SHARE_MODE;

#ifdef DUMP_CLIENT_PCM
    std::stringstream strStream;
    std::string dumpPatch;
    strStream << "/data/local/tmp/";
    strStream << appInfo_.appPid << ".pcm";
    strStream >> dumpPatch;
    AUDIO_INFO_LOG("Client dump using path: %{public}s with pid:%{public}d", dumpPatch.c_str(), getpid());

    dcp_ = fopen(dumpPatch.c_str(), "a+");
    if (dcp_ == nullptr) {
        AUDIO_ERR_LOG("Error opening pcm test file!");
    }
#endif
}

int32_t AudioRendererPrivate::InitAudioInterruptCallback()
{
    AUDIO_INFO_LOG("AudioRendererPrivate::InitAudioInterruptCallback in");
    if (audioInterrupt_.mode != SHARE_MODE && audioInterrupt_.mode != INDEPENDENT_MODE) {
        AUDIO_ERR_LOG("InitAudioInterruptCallback::Invalid interrupt mode!");
        return ERR_INVALID_PARAM;
    }
    if (audioStream_->GetAudioSessionID(audioInterrupt_.sessionID) != 0) {
        AUDIO_ERR_LOG("InitAudioInterruptCallback::GetAudioSessionID failed");
        return ERR_INVALID_INDEX;
    }
    sessionID_ = audioInterrupt_.sessionID;
    audioInterrupt_.streamUsage = rendererInfo_.streamUsage;
    audioInterrupt_.contentType = rendererInfo_.contentType;

    AUDIO_INFO_LOG("InitAudioInterruptCallback::interruptMode %{public}d, streamType %{public}d, sessionID %{public}d",
        audioInterrupt_.mode, audioInterrupt_.audioFocusType.streamType, audioInterrupt_.sessionID);

    if (audioInterruptCallback_ == nullptr) {
        audioInterruptCallback_ = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream_, audioInterrupt_);
        if (audioInterruptCallback_ == nullptr) {
            AUDIO_ERR_LOG("InitAudioInterruptCallback::Failed to allocate memory for audioInterruptCallback_");
            return ERROR;
        }
    }
    return AudioPolicyManager::GetInstance().SetAudioInterruptCallback(sessionID_, audioInterruptCallback_);
}

int32_t AudioRendererPrivate::GetFrameCount(uint32_t &frameCount) const
{
    return audioStream_->GetFrameCount(frameCount);
}

int32_t AudioRendererPrivate::GetLatency(uint64_t &latency) const
{
    return audioStream_->GetLatency(latency);
}

void AudioRendererPrivate::SetAudioPrivacyType(AudioPrivacyType privacyType)
{
    audioStream_->SetPrivacyType(privacyType);
}

int32_t AudioRendererPrivate::SetParams(const AudioRendererParams params)
{
    Trace trace("AudioRenderer::SetParams");
    AudioStreamParams audioStreamParams;
    AudioRenderer *renderer = this;
    rendererProxyObj_->SaveRendererObj(renderer);
    audioStream_->SetRendererInfo(rendererInfo_);

    audioStreamParams.format = params.sampleFormat;
    audioStreamParams.samplingRate = params.sampleRate;
    audioStreamParams.channels = params.channelCount;
    audioStreamParams.encoding = params.encodingType;

    audioStream_->SetClientID(appInfo_.appPid, appInfo_.appUid);

    int32_t ret = audioStream_->SetAudioStreamInfo(audioStreamParams, rendererProxyObj_);
    if (ret) {
        AUDIO_ERR_LOG("AudioRendererPrivate::SetParams SetAudioStreamInfo Failed");
        return ret;
    }
    AUDIO_INFO_LOG("AudioRendererPrivate::SetParams SetAudioStreamInfo Succeeded");

    return InitAudioInterruptCallback();
}

int32_t AudioRendererPrivate::GetParams(AudioRendererParams &params) const
{
    AudioStreamParams audioStreamParams;
    int32_t result = audioStream_->GetAudioStreamInfo(audioStreamParams);
    if (!result) {
        params.sampleFormat = static_cast<AudioSampleFormat>(audioStreamParams.format);
        params.sampleRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        params.channelCount = static_cast<AudioChannel>(audioStreamParams.channels);
        params.encodingType = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    }

    return result;
}

int32_t AudioRendererPrivate::GetRendererInfo(AudioRendererInfo &rendererInfo) const
{
    rendererInfo = rendererInfo_;

    return SUCCESS;
}

int32_t AudioRendererPrivate::GetStreamInfo(AudioStreamInfo &streamInfo) const
{
    AudioStreamParams audioStreamParams;
    int32_t result = audioStream_->GetAudioStreamInfo(audioStreamParams);
    if (!result) {
        streamInfo.format = static_cast<AudioSampleFormat>(audioStreamParams.format);
        streamInfo.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        streamInfo.channels = static_cast<AudioChannel>(audioStreamParams.channels);
        streamInfo.encoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    }

    return result;
}

int32_t AudioRendererPrivate::SetRendererCallback(const std::shared_ptr<AudioRendererCallback> &callback)
{
    // If the client is using the deprecated SetParams API. SetRendererCallback must be invoked, after SetParams.
    // In general, callbacks can only be set after the renderer state is PREPARED.
    RendererState state = GetStatus();
    if (state == RENDERER_NEW || state == RENDERER_RELEASED) {
        AUDIO_DEBUG_LOG("AudioRendererPrivate::SetRendererCallback incorrect state:%{public}d to register cb", state);
        return ERR_ILLEGAL_STATE;
    }
    if (callback == nullptr) {
        AUDIO_ERR_LOG("AudioRendererPrivate::SetRendererCallback callback param is null");
        return ERR_INVALID_PARAM;
    }

    // Save reference for interrupt callback
    if (audioInterruptCallback_ == nullptr) {
        AUDIO_ERR_LOG("AudioRendererPrivate::SetRendererCallback audioInterruptCallback_ == nullptr");
        return ERROR;
    }
    std::shared_ptr<AudioRendererInterruptCallbackImpl> cbInterrupt =
        std::static_pointer_cast<AudioRendererInterruptCallbackImpl>(audioInterruptCallback_);
    cbInterrupt->SaveCallback(callback);

    // Save and Set reference for stream callback. Order is important here.
    if (audioStreamCallback_ == nullptr) {
        audioStreamCallback_ = std::make_shared<AudioStreamCallbackRenderer>();
        if (audioStreamCallback_ == nullptr) {
            AUDIO_ERR_LOG("AudioRendererPrivate::Failed to allocate memory for audioStreamCallback_");
            return ERROR;
        }
    }
    std::shared_ptr<AudioStreamCallbackRenderer> cbStream =
        std::static_pointer_cast<AudioStreamCallbackRenderer>(audioStreamCallback_);
    cbStream->SaveCallback(callback);
    (void)audioStream_->SetStreamCallback(audioStreamCallback_);

    return SUCCESS;
}

int32_t AudioRendererPrivate::SetRendererPositionCallback(int64_t markPosition,
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    if ((callback == nullptr) || (markPosition <= 0)) {
        AUDIO_ERR_LOG("AudioRendererPrivate::SetRendererPositionCallback input param is invalid");
        return ERR_INVALID_PARAM;
    }

    audioStream_->SetRendererPositionCallback(markPosition, callback);

    return SUCCESS;
}

void AudioRendererPrivate::UnsetRendererPositionCallback()
{
    audioStream_->UnsetRendererPositionCallback();
}

int32_t AudioRendererPrivate::SetRendererPeriodPositionCallback(int64_t frameNumber,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    if ((callback == nullptr) || (frameNumber <= 0)) {
        AUDIO_ERR_LOG("AudioRendererPrivate::SetRendererPeriodPositionCallback input param is invalid");
        return ERR_INVALID_PARAM;
    }

    audioStream_->SetRendererPeriodPositionCallback(frameNumber, callback);

    return SUCCESS;
}

void AudioRendererPrivate::UnsetRendererPeriodPositionCallback()
{
    audioStream_->UnsetRendererPeriodPositionCallback();
}

bool AudioRendererPrivate::Start(StateChangeCmdType cmdType) const
{
    AUDIO_INFO_LOG("AudioRenderer::Start");
    RendererState state = GetStatus();
    if ((state != RENDERER_PREPARED) && (state != RENDERER_STOPPED) && (state != RENDERER_PAUSED)) {
        AUDIO_ERR_LOG("AudioRenderer::Start failed. Illegal state:%{public}u", state);
        return false;
    }

    AUDIO_INFO_LOG("AudioRenderer::Start::interruptMode: %{public}d, streamType: %{public}d, sessionID: %{public}d",
        audioInterrupt_.mode, audioInterrupt_.audioFocusType.streamType, audioInterrupt_.sessionID);

    if (audioInterrupt_.audioFocusType.streamType == STREAM_DEFAULT ||
        audioInterrupt_.sessionID == INVALID_SESSION_ID) {
        return false;
    }

    int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioRenderer::ActivateAudioInterrupt Failed");
        return false;
    }

    return audioStream_->StartAudioStream(cmdType);
}

int32_t AudioRendererPrivate::Write(uint8_t *buffer, size_t bufferSize)
{
    Trace trace("Write");
#ifdef DUMP_CLIENT_PCM
    if (dcp_ != nullptr) {
        fwrite((void *)buffer, 1, bufferSize, dcp_);
    }
#endif
    return audioStream_->Write(buffer, bufferSize);
}

RendererState AudioRendererPrivate::GetStatus() const
{
    return static_cast<RendererState>(audioStream_->GetState());
}

bool AudioRendererPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return audioStream_->GetAudioTime(timestamp, base);
}

bool AudioRendererPrivate::Drain() const
{
    return audioStream_->DrainAudioStream();
}

bool AudioRendererPrivate::Flush() const
{
    return audioStream_->FlushAudioStream();
}

bool AudioRendererPrivate::Pause(StateChangeCmdType cmdType) const
{
    AUDIO_INFO_LOG("AudioRenderer::Pause");
    RendererState state = GetStatus();
    if (state != RENDERER_RUNNING) {
        // If the stream is not running, there is no need to pause and deactive audio interrupt
        AUDIO_ERR_LOG("AudioRenderer::Pause: State of stream is not running. Illegal state:%{public}u", state);
        return false;
    }
    bool result = audioStream_->PauseAudioStream(cmdType);

    // When user is intentionally pausing, deactivate to remove from audioFocusInfoList_
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioRenderer::DeactivateAudioInterrupt Failed");
    }

    return result;
}

bool AudioRendererPrivate::Stop() const
{
    AUDIO_INFO_LOG("AudioRenderer::Stop");
    bool result = audioStream_->StopAudioStream();
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioRenderer: DeactivateAudioInterrupt Failed");
    }

    return result;
}

bool AudioRendererPrivate::Release() const
{
    AUDIO_INFO_LOG("AudioRenderer::Release");
    // If Stop call was skipped, Release to take care of Deactivation
    (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);

    // Unregister the callaback in policy server
    (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(sessionID_);

    return audioStream_->ReleaseAudioStream();
}

int32_t AudioRendererPrivate::GetBufferSize(size_t &bufferSize) const
{
    return audioStream_->GetBufferSize(bufferSize);
}

int32_t AudioRendererPrivate::GetAudioStreamId(uint32_t &sessionID) const
{
    return audioStream_->GetAudioSessionID(sessionID);
}

int32_t AudioRendererPrivate::SetAudioRendererDesc(AudioRendererDesc audioRendererDesc)
{
    ContentType contentType = audioRendererDesc.contentType;
    StreamUsage streamUsage = audioRendererDesc.streamUsage;
    AudioStreamType audioStreamType = audioStream_->GetStreamType(contentType, streamUsage);
    audioInterrupt_.audioFocusType.streamType = audioStreamType;
    return audioStream_->SetAudioStreamType(audioStreamType);
}

int32_t AudioRendererPrivate::SetStreamType(AudioStreamType audioStreamType)
{
    audioInterrupt_.audioFocusType.streamType = audioStreamType;
    return audioStream_->SetAudioStreamType(audioStreamType);
}

int32_t AudioRendererPrivate::SetVolume(float volume) const
{
    return audioStream_->SetVolume(volume);
}

float AudioRendererPrivate::GetVolume() const
{
    return audioStream_->GetVolume();
}

int32_t AudioRendererPrivate::SetRenderRate(AudioRendererRate renderRate) const
{
    return audioStream_->SetRenderRate(renderRate);
}

AudioRendererRate AudioRendererPrivate::GetRenderRate() const
{
    return audioStream_->GetRenderRate();
}

int32_t AudioRendererPrivate::SetRendererSamplingRate(uint32_t sampleRate) const
{
    return audioStream_->SetRendererSamplingRate(sampleRate);
}

uint32_t AudioRendererPrivate::GetRendererSamplingRate() const
{
    return audioStream_->GetRendererSamplingRate();
}

int32_t AudioRendererPrivate::SetBufferDuration(uint64_t bufferDuration) const
{
    if (bufferDuration < MINIMUM_BUFFER_SIZE_MSEC || bufferDuration > MAXIMUM_BUFFER_SIZE_MSEC) {
        AUDIO_ERR_LOG("Error: Please set the buffer duration between 5ms ~ 20ms");
        return ERR_INVALID_PARAM;
    }

    return audioStream_->SetBufferSizeInMsec(bufferDuration);
}

AudioRendererInterruptCallbackImpl::AudioRendererInterruptCallbackImpl(const std::shared_ptr<AudioStream> &audioStream,
    const AudioInterrupt &audioInterrupt)
    : audioStream_(audioStream), audioInterrupt_(audioInterrupt)
{
    AUDIO_INFO_LOG("AudioRendererInterruptCallbackImpl constructor");
}

AudioRendererInterruptCallbackImpl::~AudioRendererInterruptCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioRendererInterruptCallbackImpl: instance destroy");
}

void AudioRendererInterruptCallbackImpl::SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback)
{
    callback_ = callback;
}

void AudioRendererInterruptCallbackImpl::NotifyEvent(const InterruptEvent &interruptEvent)
{
    if (cb_ != nullptr) {
        cb_->OnInterrupt(interruptEvent);
        AUDIO_INFO_LOG("AudioRendererInterruptCallbackImpl::NotifyEvent: Send interruptEvent to app successfully");
    } else {
        AUDIO_ERR_LOG("AudioRendererInterruptCallbackImpl::NotifyEvent: cb_==nullptr, failed to send interruptEvent");
    }
}

bool AudioRendererInterruptCallbackImpl::HandleForceDucking(const InterruptEventInternal &interruptEvent)
{
    int32_t systemVolumeLevel =
        AudioPolicyManager::GetInstance().GetSystemVolumeLevel(audioInterrupt_.audioFocusType.streamType);
    float systemVolumeDb = VolumeToDb(systemVolumeLevel);
    float duckVolumeDb = interruptEvent.duckVolume;
    int32_t ret = 0;

    if (systemVolumeDb <= duckVolumeDb || FLOAT_COMPARE_EQ(systemVolumeDb, 0.0f)) {
        AUDIO_INFO_LOG("HandleForceDucking: StreamVolume %{public}f <= duckVolumeDb %{public}f. "
            "No need to duck further", systemVolumeDb, duckVolumeDb);
        return false;
    }

    instanceVolBeforeDucking_ = audioStream_->GetVolume();
    float duckInstanceVolume = duckVolumeDb / systemVolumeDb;
    if (FLOAT_COMPARE_EQ(instanceVolBeforeDucking_, 0.0f) || instanceVolBeforeDucking_ < duckInstanceVolume) {
        AUDIO_INFO_LOG("HandleForceDucking: No need to duck further");
        return false;
    }

    ret = audioStream_->SetVolume(duckInstanceVolume);
    if (ret) {
        AUDIO_ERR_LOG("HandleForceDucking: Failed to set duckVolumeDb(instance) %{pubic}f", duckInstanceVolume);
        return false;
    }

    AUDIO_INFO_LOG("HandleForceDucking: Set duckVolumeDb(instance) %{pubic}f successfully", duckInstanceVolume);
    return true;
}

void AudioRendererInterruptCallbackImpl::NotifyForcePausedToResume(const InterruptEventInternal &interruptEvent)
{
    // Change InterruptForceType to Share, Since app will take care of resuming
    InterruptEvent interruptEventResume {interruptEvent.eventType, INTERRUPT_SHARE,
                                         interruptEvent.hintType};
    NotifyEvent(interruptEventResume);
}

void AudioRendererInterruptCallbackImpl::HandleAndNotifyForcedEvent(const InterruptEventInternal &interruptEvent)
{
    // ForceType: INTERRUPT_FORCE. Handle the event forcely and notify the app.
    AUDIO_DEBUG_LOG("HandleAndNotifyForcedEvent in");
    InterruptHint hintType = interruptEvent.hintType;
    switch (hintType) {
        case INTERRUPT_HINT_PAUSE:
            if (audioStream_->GetState() == PREPARED) {
                AUDIO_INFO_LOG("HandleAndNotifyForcedEvent: To pause incoming, no need to pause");
            } else if (audioStream_->GetState() == RUNNING) {
                (void)audioStream_->PauseAudioStream(); // Just Pause, do not deactivate here
            } else {
                AUDIO_WARNING_LOG("HandleAndNotifyForcedEvent: State of stream is not running.No need to pause");
                return;
            }
            isForcePaused_ = true;
            break;
        case INTERRUPT_HINT_RESUME:
            if ((audioStream_->GetState() != PAUSED && audioStream_->GetState() != PREPARED) || !isForcePaused_) {
                AUDIO_WARNING_LOG("HandleAndNotifyForcedEvent: State of stream is not paused or pause is not forced");
                return;
            }
            isForcePaused_ = false;
            NotifyForcePausedToResume(interruptEvent);
            return; // return, sending callback is taken care in NotifyForcePausedToResume
        case INTERRUPT_HINT_STOP:
            (void)audioStream_->StopAudioStream();
            break;
        case INTERRUPT_HINT_DUCK:
            if (!HandleForceDucking(interruptEvent)) {
                AUDIO_WARNING_LOG("HandleAndNotifyForcedEvent: Failed to duck forcely, don't notify app");
                return;
            }
            isForceDucked_ = true;
            break;
        case INTERRUPT_HINT_UNDUCK:
            if (!isForceDucked_) {
                AUDIO_WARNING_LOG("HandleAndNotifyForcedEvent: It is not forced ducked, don't unduck or notify app");
                return;
            }
            (void)audioStream_->SetVolume(instanceVolBeforeDucking_);
            AUDIO_INFO_LOG("HandleAndNotifyForcedEvent: Unduck Volume(instance) successfully: %{public}f",
                instanceVolBeforeDucking_);
            isForceDucked_ = false;
            break;
        default: // If the hintType is NONE, don't need to send callbacks
            return;
    }
    // Notify valid forced event callbacks to app
    InterruptEvent interruptEventForced {interruptEvent.eventType, interruptEvent.forceType, interruptEvent.hintType};
    NotifyEvent(interruptEventForced);
}

void AudioRendererInterruptCallbackImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    cb_ = callback_.lock();
    InterruptForceType forceType = interruptEvent.forceType;
    AUDIO_INFO_LOG("AudioRendererInterruptCallbackImpl::OnInterrupt: forceType %{public}d, hintType: %{public}d",
        forceType, interruptEvent.hintType);

    if (forceType != INTERRUPT_FORCE) { // INTERRUPT_SHARE
        AUDIO_DEBUG_LOG("AudioRendererPrivate ForceType: INTERRUPT_SHARE. Let app handle the event");
        InterruptEvent interruptEventShared {interruptEvent.eventType, interruptEvent.forceType,
            interruptEvent.hintType};
        NotifyEvent(interruptEventShared);
        return;
    }

    if (audioStream_ == nullptr) {
        AUDIO_ERR_LOG("AudioRendererInterruptCallbackImpl::Stream is not alive. No need to take forced action");
        return;
    }

    HandleAndNotifyForcedEvent(interruptEvent);
}

void AudioStreamCallbackRenderer::SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback)
{
    callback_ = callback;
}

void AudioStreamCallbackRenderer::OnStateChange(const State state, const StateChangeCmdType cmdType)
{
    std::shared_ptr<AudioRendererCallback> cb = callback_.lock();
    if (cb == nullptr) {
        AUDIO_ERR_LOG("AudioStreamCallbackRenderer::OnStateChange cb == nullptr.");
        return;
    }

    cb->OnStateChange(static_cast<RendererState>(state), cmdType);
}

std::vector<AudioSampleFormat> AudioRenderer::GetSupportedFormats()
{
    return AUDIO_SUPPORTED_FORMATS;
}

std::vector<AudioSamplingRate> AudioRenderer::GetSupportedSamplingRates()
{
    return AUDIO_SUPPORTED_SAMPLING_RATES;
}

std::vector<AudioChannel> AudioRenderer::GetSupportedChannels()
{
    return RENDERER_SUPPORTED_CHANNELS;
}

std::vector<AudioEncodingType> AudioRenderer::GetSupportedEncodingTypes()
{
    return AUDIO_SUPPORTED_ENCODING_TYPES;
}

int32_t AudioRendererPrivate::SetRenderMode(AudioRenderMode renderMode) const
{
    return audioStream_->SetRenderMode(renderMode);
}

AudioRenderMode AudioRendererPrivate::GetRenderMode() const
{
    return audioStream_->GetRenderMode();
}

int32_t AudioRendererPrivate::GetBufferDesc(BufferDesc &bufDesc) const
{
    return audioStream_->GetBufferDesc(bufDesc);
}

int32_t AudioRendererPrivate::Enqueue(const BufferDesc &bufDesc) const
{
    return audioStream_->Enqueue(bufDesc);
}

int32_t AudioRendererPrivate::Clear() const
{
    return audioStream_->Clear();
}

int32_t AudioRendererPrivate::GetBufQueueState(BufferQueueState &bufState) const
{
    return audioStream_->GetBufQueueState(bufState);
}

void AudioRendererPrivate::SetApplicationCachePath(const std::string cachePath)
{
    audioStream_->SetApplicationCachePath(cachePath);
}

int32_t AudioRendererPrivate::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    return audioStream_->SetRendererWriteCallback(callback);
}

void AudioRendererPrivate::SetInterruptMode(InterruptMode mode)
{
    AUDIO_INFO_LOG("AudioRendererPrivate::SetInterruptMode: InterruptMode %{pubilc}d", mode);
    if (audioInterrupt_.mode == mode) {
        return;
    } else if (mode != SHARE_MODE && mode != INDEPENDENT_MODE) {
        AUDIO_ERR_LOG("AudioRendererPrivate::SetInterruptMode: Invalid interrupt mode!");
        return;
    }
    audioInterrupt_.mode = mode;
}

int32_t AudioRendererPrivate::SetLowPowerVolume(float volume) const
{
    return audioStream_->SetLowPowerVolume(volume);
}

float AudioRendererPrivate::GetLowPowerVolume() const
{
    return audioStream_->GetLowPowerVolume();
}

float AudioRendererPrivate::GetSingleStreamVolume() const
{
    return audioStream_->GetSingleStreamVolume();
}

float AudioRendererPrivate::GetMinStreamVolume() const
{
    return AudioPolicyManager::GetInstance().GetMinStreamVolume();
}

float AudioRendererPrivate::GetMaxStreamVolume() const
{
    return AudioPolicyManager::GetInstance().GetMaxStreamVolume();
}

int32_t AudioRendererPrivate::GetCurrentOutputDevices(DeviceInfo &deviceInfo) const
{
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamId(sessionId);
    if (ret) {
        AUDIO_ERR_LOG("AudioRendererPrivate::GetCurrentOutputDevices Get sessionId failed");
        return ret;
    }

    ret = AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    if (ret) {
        AUDIO_ERR_LOG("AudioRendererPrivate::GetCurrentOutputDevices Get Current Renderer devices failed");
        return ret;
    }

    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            deviceInfo = (*it)->outputDeviceInfo;
        }
    }
    return SUCCESS;
}

uint32_t AudioRendererPrivate::GetUnderflowCount() const
{
    return audioStream_->GetUnderflowCount();
}

int32_t AudioRendererPrivate::RegisterAudioRendererEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioRendererDeviceChangeCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterAudioRendererEventListener client id: %{public}d", clientPid);
    if (callback == nullptr) {
        AUDIO_ERR_LOG("callback is null");
        return ERR_INVALID_PARAM;
    }

    if (GetCurrentOutputDevices(currentDeviceInfo) != SUCCESS) {
        AUDIO_ERR_LOG("get current device info failed");
        return ERROR;
    }

    if (!audioDeviceChangeCallback_) {
        audioDeviceChangeCallback_ = std::make_shared<AudioRendererStateChangeCallbackImpl>();
        if (!audioDeviceChangeCallback_) {
            AUDIO_ERR_LOG("AudioRendererPrivate: Memory Allocation Failed !!");
            return ERROR;
        }
    }

    int32_t ret =
        AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(clientPid, audioDeviceChangeCallback_);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioRendererPrivate::RegisterAudioRendererEventListener failed");
        return ERROR;
    }

    audioDeviceChangeCallback_->setAudioRendererObj(this);
    audioDeviceChangeCallback_->SaveCallback(callback);
    AUDIO_INFO_LOG("AudioRendererPrivate::RegisterAudioRendererEventListener successful!");
    return SUCCESS;
}

int32_t AudioRendererPrivate::RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
    const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterAudioPolicyServerDiedCb client id: %{public}d", clientPid);
    if (callback == nullptr) {
        AUDIO_ERR_LOG("callback is null");
        return ERR_INVALID_PARAM;
    }

    return AudioPolicyManager::GetInstance().RegisterAudioPolicyServerDiedCb(clientPid, callback);
}

int32_t AudioRendererPrivate::UnregisterAudioPolicyServerDiedCb(const int32_t clientPid)
{
    AUDIO_INFO_LOG("AudioRendererPrivate:: UnregisterAudioPolicyServerDiedCb client id: %{public}d", clientPid);
    return AudioPolicyManager::GetInstance().UnregisterAudioPolicyServerDiedCb(clientPid);
}

void AudioRendererPrivate::DestroyAudioRendererStateCallback()
{
    if (audioDeviceChangeCallback_ != nullptr) {
        audioDeviceChangeCallback_.reset();
        audioDeviceChangeCallback_ = nullptr;
    }
}

int32_t AudioRendererPrivate::UnregisterAudioRendererEventListener(const int32_t clientPid)
{
    AUDIO_INFO_LOG("AudioRendererPrivate::UnregisterAudioCapturerEventListener client id: %{public}d", clientPid);
    int32_t ret = AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(clientPid);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioRendererPrivate::UnregisterAudioRendererEventListener failed");
        return ERROR;
    }

    DestroyAudioRendererStateCallback();
    return SUCCESS;
}

AudioRendererStateChangeCallbackImpl::AudioRendererStateChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioRendererStateChangeCallbackImpl instance create");
}

AudioRendererStateChangeCallbackImpl::~AudioRendererStateChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioRendererStateChangeCallbackImpl instance destory");
}

void AudioRendererStateChangeCallbackImpl::SaveCallback(
    const std::weak_ptr<AudioRendererDeviceChangeCallback> &callback)
{
    callback_ = callback;
}

void AudioRendererStateChangeCallbackImpl::setAudioRendererObj(AudioRendererPrivate *rendererObj)
{
    renderer = rendererObj;
}

bool AudioRendererPrivate::IsDeviceChanged(DeviceInfo &newDeviceInfo)
{
    bool deviceUpdated = false;
    DeviceInfo deviceInfo = {};

    if (GetCurrentOutputDevices(deviceInfo) != SUCCESS) {
        AUDIO_ERR_LOG("AudioRendererPrivate::GetCurrentOutputDevices failed");
        return deviceUpdated;
    }

    AUDIO_INFO_LOG("newDeviceInfo type: %{public}d, currentDeviceInfo type: %{public}d ",
        deviceInfo.deviceType, currentDeviceInfo.deviceType);
    if (currentDeviceInfo.deviceType != deviceInfo.deviceType) {
        currentDeviceInfo = deviceInfo;
        newDeviceInfo = currentDeviceInfo;
        deviceUpdated = true;
    }
    return deviceUpdated;
}

void AudioRendererStateChangeCallbackImpl::OnRendererStateChange(
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    std::shared_ptr<AudioRendererDeviceChangeCallback> cb = callback_.lock();
    if (cb == nullptr) {
        AUDIO_ERR_LOG("AudioRendererStateChangeCallbackImpl::OnStateChange cb == nullptr.");
        return;
    }
    AUDIO_INFO_LOG("AudioRendererStateChangeCallbackImpl OnRendererStateChange");
    DeviceInfo deviceInfo = {};
    if (renderer->IsDeviceChanged(deviceInfo)) {
        cb->OnStateChange(deviceInfo);
    }
}

AudioEffectMode AudioRendererPrivate::GetAudioEffectMode() const
{
    return audioStream_->GetAudioEffectMode();
}

int64_t AudioRendererPrivate::GetFramesWritten() const
{
    return audioStream_->GetFramesWritten();
}

int32_t AudioRendererPrivate::SetAudioEffectMode(AudioEffectMode effectMode) const
{
    return audioStream_->SetAudioEffectMode(effectMode);
}
}  // namespace AudioStandard
}  // namespace OHOS
