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

#include "bluetooth_renderer_sink.h"
#include <dlfcn.h>
#include <unistd.h>
#include <string>
#include "cstring"
#include "iosfwd"
#include "__config"
#include "audio_control.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_log.h"
#include "audio_volume.h"


using namespace std;
using namespace OHOS::HDI::Audio_Bluetooth;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t HALF_FACTOR = 2;
const int32_t MAX_AUDIO_ADAPTER_NUM = 5;
const float DEFAULT_VOLUME_LEVEL = 1.0f;
const uint32_t AUDIO_CHANNELCOUNT = 2;
const uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
const uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
const uint32_t RENDER_FRAME_INTERVAL_IN_MICROSECONDS = 10000;
const uint32_t INT_32_MAX = 0x7fffffff;
const uint32_t PCM_8_BIT = 8;
const uint32_t PCM_16_BIT = 16;
const uint32_t PCM_24_BIT = 24;
const uint32_t PCM_32_BIT = 32;
}

#ifdef BT_DUMPFILE
const char *g_audioOutTestFilePath = "/data/local/tmp/audioout_bt.pcm";
#endif // BT_DUMPFILE

BluetoothRendererSink::BluetoothRendererSink()
    : rendererInited_(false), started_(false), paused_(false), leftVolume_(DEFAULT_VOLUME_LEVEL),
      rightVolume_(DEFAULT_VOLUME_LEVEL), audioManager_(nullptr), audioAdapter_(nullptr),
      audioRender_(nullptr), handle_(nullptr)
{
    attr_ = {};
#ifdef BT_DUMPFILE
    pfd = nullptr;
#endif // BT_DUMPFILE
}

BluetoothRendererSink::~BluetoothRendererSink()
{
    DeInit();
}

BluetoothRendererSink *BluetoothRendererSink::GetInstance()
{
    static BluetoothRendererSink audioRenderer_;

    return &audioRenderer_;
}

void BluetoothRendererSink::DeInit()
{
    started_ = false;
    rendererInited_ = false;
    if ((audioRender_ != nullptr) && (audioAdapter_ != nullptr)) {
        audioAdapter_->DestroyRender(audioAdapter_, audioRender_);
    }
    audioRender_ = nullptr;

    if ((audioManager_ != nullptr) && (audioAdapter_ != nullptr)) {
        audioManager_->UnloadAdapter(audioManager_, audioAdapter_);
    }
    audioAdapter_ = nullptr;
    audioManager_ = nullptr;

    dlclose(handle_);

#ifdef BT_DUMPFILE
    if (pfd) {
        fclose(pfd);
        pfd = nullptr;
    }
#endif // BT_DUMPFILE
}

void InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.frameSize = PCM_16_BIT * attrs.channelCount / PCM_8_BIT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = 0;
    attrs.type = AUDIO_IN_MEDIA;
    attrs.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (attrs.frameSize);
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = 0;
}

static int32_t SwitchAdapter(struct AudioAdapterDescriptor *descs, string adapterNameCase,
    enum AudioPortDirection portFlag, struct AudioPort &renderPort, int32_t size)
{
    AUDIO_INFO_LOG("BluetoothRendererSink: adapterNameCase: %{public}s", adapterNameCase.c_str());
    if (descs == nullptr) {
        return ERROR;
    }

    for (int32_t index = 0; index < size; index++) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr) {
            continue;
        }
        AUDIO_INFO_LOG("BluetoothRendererSink: adapter name for %{public}d: %{public}s", index, desc->adapterName);
        if (!strcmp(desc->adapterName, adapterNameCase.c_str())) {
            for (uint32_t port = 0; port < desc->portNum; port++) {
                // Only find out the port of out in the sound card
                if (desc->ports[port].dir == portFlag) {
                    renderPort = desc->ports[port];
                    AUDIO_INFO_LOG("BluetoothRendererSink: index found %{public}d", index);
                    return index;
                }
            }
        }
    }
    AUDIO_ERR_LOG("SwitchAdapter Fail");

    return ERR_INVALID_INDEX;
}

int32_t BluetoothRendererSink::InitAudioManager()
{
    AUDIO_INFO_LOG("BluetoothRendererSink: Initialize audio proxy manager");

#ifdef __aarch64__
    char resolvedPath[100] = "/vendor/lib64/libaudio_bluetooth_hdi_proxy_server.z.so";
#else
    char resolvedPath[100] = "/vendor/lib/libaudio_bluetooth_hdi_proxy_server.z.so";
#endif
    struct AudioProxyManager *(*getAudioManager)() = nullptr;

    handle_ = dlopen(resolvedPath, 1);
    if (handle_ == nullptr) {
        AUDIO_ERR_LOG("Open so Fail");
        return ERR_INVALID_HANDLE;
    }
    AUDIO_INFO_LOG("dlopen successful");

    getAudioManager = (struct AudioProxyManager *(*)())(dlsym(handle_, "GetAudioProxyManagerFuncs"));
    if (getAudioManager == nullptr) {
        return ERR_INVALID_HANDLE;
    }
    AUDIO_INFO_LOG("getaudiomanager done");

    audioManager_ = getAudioManager();
    if (audioManager_ == nullptr) {
        return ERR_INVALID_HANDLE;
    }
    AUDIO_INFO_LOG("audio manager created");

    return 0;
}

uint32_t PcmFormatToBits(AudioFormat format)
{
    switch (format) {
        case AUDIO_FORMAT_PCM_8_BIT:
            return PCM_8_BIT;
        case AUDIO_FORMAT_PCM_16_BIT:
            return PCM_16_BIT;
        case AUDIO_FORMAT_PCM_24_BIT:
            return PCM_24_BIT;
        case AUDIO_FORMAT_PCM_32_BIT:
            return PCM_32_BIT;
        default:
            return PCM_24_BIT;
    };
}

int32_t BluetoothRendererSink::CreateRender(struct AudioPort &renderPort)
{
    AUDIO_DEBUG_LOG("Create render in");
    int32_t ret;
    struct AudioSampleAttributes param;
    InitAttrs(param);
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.format = attr_.format;
    param.frameSize = PcmFormatToBits(param.format) * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    AUDIO_DEBUG_LOG("BluetoothRendererSink Create render format: %{public}d", param.format);
    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = renderPort.portId;
    deviceDesc.pins = PIN_OUT_SPEAKER;
    deviceDesc.desc = nullptr;
    ret = audioAdapter_->CreateRender(audioAdapter_, &deviceDesc, &param, &audioRender_);
    if (ret != 0 || audioRender_ == nullptr) {
        AUDIO_ERR_LOG("AudioDeviceCreateRender failed");
        audioManager_->UnloadAdapter(audioManager_, audioAdapter_);
        return ERR_NOT_STARTED;
    }
    AUDIO_DEBUG_LOG("create render done");

    return 0;
}

int32_t BluetoothRendererSink::Init(const BluetoothSinkAttr &attr)
{
    AUDIO_DEBUG_LOG("BluetoothRendererSink Init: %{public}d", attr_.format);
    attr_ = attr;

    string adapterNameCase = "bt_a2dp";  // Set sound card information
    enum AudioPortDirection port = PORT_OUT; // Set port information

    if (InitAudioManager() != 0) {
        AUDIO_ERR_LOG("Init audio manager Fail");
        return ERR_NOT_STARTED;
    }

    int32_t size = 0;
    int32_t ret;
    struct AudioAdapterDescriptor *descs = nullptr;
    ret = audioManager_->GetAllAdapters(audioManager_, &descs, &size);
    if (size > MAX_AUDIO_ADAPTER_NUM || size == 0 || descs == nullptr || ret != 0) {
        AUDIO_ERR_LOG("Get adapters Fail");
        return ERR_NOT_STARTED;
    }

    // Get qualified sound card and port
    int32_t index = SwitchAdapter(descs, adapterNameCase, port, audioPort, size);
    if (index < 0) {
        AUDIO_ERR_LOG("Switch Adapter Fail");
        return ERR_NOT_STARTED;
    }

    struct AudioAdapterDescriptor *desc = &descs[index];
    if (audioManager_->LoadAdapter(audioManager_, desc, &audioAdapter_) != 0) {
        AUDIO_ERR_LOG("Load Adapter Fail");
        return ERR_NOT_STARTED;
    }
    if (audioAdapter_ == nullptr) {
        AUDIO_ERR_LOG("Load audio device failed");
        return ERR_NOT_STARTED;
    }

    // Initialization port information, can fill through mode and other parameters
    ret = audioAdapter_->InitAllPorts(audioAdapter_);
    if (ret != 0) {
        AUDIO_ERR_LOG("InitAllPorts failed");
        return ERR_NOT_STARTED;
    }

    if (CreateRender(audioPort) != 0) {
        AUDIO_ERR_LOG("Create render failed");
        return ERR_NOT_STARTED;
    }

    rendererInited_ = true;

#ifdef BT_DUMPFILE
    pfd = fopen(g_audioOutTestFilePath, "wb+");
    if (pfd == nullptr) {
        AUDIO_ERR_LOG("Error opening pcm test file!");
    }
#endif // BT_DUMPFILE

    return SUCCESS;
}

int32_t BluetoothRendererSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int32_t ret = SUCCESS;
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("Bluetooth Render Handle is nullptr!");
        return ERR_INVALID_HANDLE;
    }

#ifdef BT_DUMPFILE
    size_t writeResult = fwrite((void*)&data, 1, len, pfd);
    if (writeResult != len) {
        AUDIO_ERR_LOG("Failed to write the file.");
    }
#endif // BT_DUMPFILE

    while (true) {
        ret = audioRender_->RenderFrame(audioRender_, (void*)&data, len, &writeLen);
        AUDIO_DEBUG_LOG("A2dp RenderFrame returns: %{public}x", ret);
        if (ret == -4) {
            AUDIO_ERR_LOG("retry render frame...");
            usleep(RENDER_FRAME_INTERVAL_IN_MICROSECONDS);
            continue;
        }

        if (ret != 0) {
            AUDIO_ERR_LOG("A2dp RenderFrame failed ret: %{public}x", ret);
            ret = ERR_WRITE_FAILED;
        }

        break;
    }
    usleep(RENDER_FRAME_INTERVAL_IN_MICROSECONDS);

    return ret;
}

int32_t BluetoothRendererSink::Start(void)
{
    int32_t ret;

    if (!started_) {
        ret = audioRender_->control.Start(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            started_ = true;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("BluetoothRendererSink::Start failed!");
            return ERR_NOT_STARTED;
        }
    }

    return SUCCESS;
}

int32_t BluetoothRendererSink::SetVolume(float left, float right)
{
    int32_t ret;
    float volume;

    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("BluetoothRendererSink::SetVolume failed audioRender_ null");
        return ERR_INVALID_HANDLE;
    }

    leftVolume_ = left;
    rightVolume_ = right;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    ret = audioRender_->volume.SetVolume(reinterpret_cast<AudioHandle>(audioRender_), volume);
    if (ret) {
        AUDIO_ERR_LOG("BluetoothRendererSink::Set volume failed!");
    }

    return ret;
}

int32_t BluetoothRendererSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t BluetoothRendererSink::GetLatency(uint32_t *latency)
{
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("BluetoothRendererSink: GetLatency failed audio render null");
        return ERR_INVALID_HANDLE;
    }

    if (!latency) {
        AUDIO_ERR_LOG("BluetoothRendererSink: GetLatency failed latency null");
        return ERR_INVALID_PARAM;
    }

    uint32_t hdiLatency;
    if (audioRender_->GetLatency(audioRender_, &hdiLatency) == 0) {
        *latency = hdiLatency;
        return SUCCESS;
    } else {
        return ERR_OPERATION_FAILED;
    }
}

int32_t BluetoothRendererSink::GetTransactionId(uint64_t *transactionId)
{
    AUDIO_INFO_LOG("BluetoothRendererSink::GetTransactionId in");

    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("BluetoothRendererSink: GetTransactionId failed audio render null");
        return ERR_INVALID_HANDLE;
    }

    if (!transactionId) {
        AUDIO_ERR_LOG("BluetoothRendererSink: GetTransactionId failed transactionId null");
        return ERR_INVALID_PARAM;
    }

    *transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

int32_t BluetoothRendererSink::Stop(void)
{
    AUDIO_INFO_LOG("BluetoothRendererSink::Stop in");
    int32_t ret;

    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("BluetoothRendererSink::Stop failed audioRender_ null");
        return ERR_INVALID_HANDLE;
    }

    if (started_) {
        AUDIO_INFO_LOG("BluetoothRendererSink::Stop control before");
        ret = audioRender_->control.Stop(reinterpret_cast<AudioHandle>(audioRender_));
        AUDIO_INFO_LOG("BluetoothRendererSink::Stop control after");
        if (!ret) {
            started_ = false;
            paused_ = false;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("BluetoothRendererSink::Stop failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return SUCCESS;
}

int32_t BluetoothRendererSink::Pause(void)
{
    int32_t ret;

    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("BluetoothRendererSink::Pause failed audioRender_ null");
        return ERR_INVALID_HANDLE;
    }

    if (!started_) {
        AUDIO_ERR_LOG("BluetoothRendererSink::Pause invalid state!");
        return ERR_OPERATION_FAILED;
    }

    if (!paused_) {
        ret = audioRender_->control.Pause(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            paused_ = true;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("BluetoothRendererSink::Pause failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return SUCCESS;
}

int32_t BluetoothRendererSink::Resume(void)
{
    int32_t ret;

    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("BluetoothRendererSink::Resume failed audioRender_ null");
        return ERR_INVALID_HANDLE;
    }

    if (!started_) {
        AUDIO_ERR_LOG("BluetoothRendererSink::Resume invalid state!");
        return ERR_OPERATION_FAILED;
    }

    if (paused_) {
        ret = audioRender_->control.Resume(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            paused_ = false;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("BluetoothRendererSink::Resume failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return SUCCESS;
}

int32_t BluetoothRendererSink::Reset(void)
{
    int32_t ret;

    if (started_ && audioRender_ != nullptr) {
        ret = audioRender_->control.Flush(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("BluetoothRendererSink::Reset failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}

int32_t BluetoothRendererSink::Flush(void)
{
    int32_t ret;

    if (started_ && audioRender_ != nullptr) {
        ret = audioRender_->control.Flush(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("BluetoothRendererSink::Flush failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}
} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

BluetoothRendererSink *g_bluetoothRendrSinkInstance = BluetoothRendererSink::GetInstance();
int32_t BluetoothFillinAudioRenderSinkWapper(char *deviceNetworkId, void **wapper)
{
    (void)deviceNetworkId;
    *wapper = static_cast<void *>(BluetoothRendererSink::GetInstance());
    return SUCCESS;
}

int32_t BluetoothRendererSinkInit(void *wapper, BluetoothSinkAttr *attr)
{
    int32_t ret;
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);
    if (bluetoothRendererSinkWapper->rendererInited_)
        return SUCCESS;

    ret = bluetoothRendererSinkWapper->Init(*attr);
    return ret;
}

void BluetoothRendererSinkDeInit(void *wapper)
{
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);
    if (bluetoothRendererSinkWapper->rendererInited_)
        bluetoothRendererSinkWapper->DeInit();
}

int32_t BluetoothRendererSinkStop(void *wapper)
{
    int32_t ret;
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);

    if (!bluetoothRendererSinkWapper->rendererInited_)
        return SUCCESS;

    ret = bluetoothRendererSinkWapper->Stop();
    return ret;
}

int32_t BluetoothRendererSinkStart(void *wapper)
{
    int32_t ret;
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);

    if (!bluetoothRendererSinkWapper->rendererInited_) {
        AUDIO_ERR_LOG("audioRenderer Not Inited! Init the renderer first\n");
        return ERR_NOT_STARTED;
    }

    ret = bluetoothRendererSinkWapper->Start();
    return ret;
}

int32_t BluetoothRendererSinkPause(void *wapper)
{
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);
    if (!bluetoothRendererSinkWapper->rendererInited_) {
        AUDIO_ERR_LOG("BT renderer sink pause failed");
        return ERR_NOT_STARTED;
    }

    return bluetoothRendererSinkWapper->Pause();
}

int32_t BluetoothRendererSinkResume(void *wapper)
{
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);
    if (!bluetoothRendererSinkWapper->rendererInited_) {
        AUDIO_ERR_LOG("BT renderer sink resume failed");
        return ERR_NOT_STARTED;
    }

    return bluetoothRendererSinkWapper->Resume();
}

int32_t BluetoothRendererRenderFrame(void *wapper, char &data, uint64_t len, uint64_t &writeLen)
{
    int32_t ret;
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);

    if (!bluetoothRendererSinkWapper->rendererInited_) {
        AUDIO_ERR_LOG("audioRenderer Not Inited! Init the renderer first\n");
        return ERR_NOT_STARTED;
    }

    ret = bluetoothRendererSinkWapper->RenderFrame(data, len, writeLen);
    return ret;
}

int32_t BluetoothRendererSinkSetVolume(void *wapper, float left, float right)
{
    int32_t ret;
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);

    if (!bluetoothRendererSinkWapper->rendererInited_) {
        AUDIO_ERR_LOG("audioRenderer Not Inited! Init the renderer first\n");
        return ERR_NOT_STARTED;
    }

    ret = bluetoothRendererSinkWapper->SetVolume(left, right);
    return ret;
}

int32_t BluetoothRendererSinkGetLatency(void *wapper, uint32_t *latency)
{
    int32_t ret;
    BluetoothRendererSink *bluetoothRendererSinkWapper = static_cast<BluetoothRendererSink *>(wapper);

    if (!bluetoothRendererSinkWapper->rendererInited_) {
        AUDIO_ERR_LOG("audioRenderer Not Inited! Init the renderer first\n");
        return ERR_NOT_STARTED;
    }

    if (!latency) {
        AUDIO_ERR_LOG("BluetoothRendererSinkGetLatency failed latency null");
        return ERR_INVALID_PARAM;
    }

    ret = bluetoothRendererSinkWapper->GetLatency(latency);
    return ret;
}

int32_t BluetoothRendererSinkGetTransactionId(uint64_t *transactionId)
{
    if (!g_bluetoothRendrSinkInstance->rendererInited_) {
        AUDIO_ERR_LOG("audioRenderer Not Inited! Init the renderer first");
        return ERR_NOT_STARTED;
    }

    if (!transactionId) {
        AUDIO_ERR_LOG("BluetoothRendererSinkGetTransactionId failed transaction id null");
        return ERR_INVALID_PARAM;
    }

    return g_bluetoothRendrSinkInstance->GetTransactionId(transactionId);
}
#ifdef __cplusplus
}
#endif
