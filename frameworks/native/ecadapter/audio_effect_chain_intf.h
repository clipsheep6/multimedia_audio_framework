// /*
//  * Copyright (c) 2022 Huawei Device Co., Ltd.
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *     http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */

// #ifndef AUDIO_EffectChain_SINK_INTF_H
// #define AUDIO_EffectChain_SINK_INTF_H

// #ifdef __cplusplus
// extern "C" {
// #endif
// // should be same with AudioSampleFormat in audio_info.h
// enum SampleFormat {
//     SAMPLE_U8 = 0,
//     SAMPLE_S16LE = 1,
//     SAMPLE_S24LE = 2,
//     SAMPLE_S32LE = 3,
//     SAMPLE_F32LE = 4,
//     INVALID_WIDTH = -1
// };

// // should be same with AudioSinkAttr in i_audio_EffectChain_sink.h
// typedef struct {
//     const char *adapterName;
//     uint32_t openMicSpeaker;
//     enum SampleFormat format;
//     uint32_t sampleFmt;
//     uint32_t sampleRate;
//     uint32_t channel;
//     float volume;
//     const char *filePath;
//     const char *deviceNetworkId;
//     int32_t device_type;
// } SinkAttr;

// struct EffectChainAdapter {
//     int32_t deviceClass;
//     void* wapper;
//     int32_t (*EffectChainSinkInit)(struct EffectChainAdapter *adapter, const SinkAttr *attr);
//     void (*EffectChainSinkDeInit)(struct EffectChainAdapter *adapter);
//     int32_t (*EffectChainSinkStart)(struct EffectChainAdapter *adapter);
//     int32_t (*EffectChainSinkPause)(struct EffectChainAdapter *adapter);
//     int32_t (*EffectChainSinkResume)(struct EffectChainAdapter *adapter);
//     int32_t (*EffectChainSinkStop)(struct EffectChainAdapter *adapter);
//     int32_t (*EffectChainRenderFrame)(struct EffectChainAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen);
//     int32_t (*EffectChainSinkSetVolume)(struct EffectChainAdapter *adapter, float left, float right);
//     int32_t (*EffectChainSinkGetLatency)(struct EffectChainAdapter *adapter, uint32_t *latency);
// };

// int32_t FillinSinkWapper(const char *device, const char *deviceNetworkId, struct EffectChainAdapter *adapter);
// int32_t AudioEffectChainSinkInit(struct EffectChainAdapter *adapter, const SinkAttr *attr);
// void AudioEffectChainSinkDeInit(struct EffectChainAdapter *adapter);
// int32_t AudioEffectChainSinkStart(struct EffectChainAdapter *adapter);
// int32_t AudioEffectChainSinkStop(struct EffectChainAdapter *adapter);
// int32_t AudioEffectChainSinkPause(struct EffectChainAdapter *adapter);
// int32_t AudioEffectChainSinkResume(struct EffectChainAdapter *adapter);
// int32_t AudioEffectChainSinkRenderFrame(struct EffectChainAdapter *adapter, char *data, uint64_t len,
//     uint64_t *writeLen);
// int32_t AudioEffectChainSinkSetVolume(struct EffectChainAdapter *adapter, float left, float right);
// int32_t AudioEffectChainSinkGetLatency(struct EffectChainAdapter *adapter, uint32_t *latency);
// #ifdef __cplusplus
// }
// #endif
// #endif // AUDIO_EffectChain_SINK_INTF_H
