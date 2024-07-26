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
#undef LOG_TAG
#define LOG_TAG "ModuleHdiSource"

#include <config.h>
#include <pulsecore/log.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/source.h>
#include <stddef.h>
#include <stdbool.h>

#include <pulsecore/core.h>
#include <pulsecore/core-util.h>
#include <pulsecore/namereg.h>

#include "securec.h"
#include "audio_log.h"
#include "audio_hdiadapter_info.h"
#include "audio_enhance_chain_adapter.h"
#include "userdata.h"

const uint32_t BYTE_SIZE_SAMPLE_U8 = 1;
const uint32_t BYTE_SIZE_SAMPLE_S16 = 2;
const uint32_t BYTE_SIZE_SAMPLE_S24 = 3;
const uint32_t BYTE_SIZE_SAMPLE_S32 = 4;

pa_source *PaHdiSourceNew(pa_module *m, pa_modargs *ma, const char *driver);
void PaHdiSourceFree(pa_source *s);

PA_MODULE_AUTHOR("OpenHarmony");
PA_MODULE_DESCRIPTION("OpenHarmony HDI Source");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        "source_name=<name for the source> "
        "device_class=<name for the device class> "
        "source_properties=<properties for the source> "
        "format=<sample format> "
        "rate=<sample rate> "
        "channels=<number of channels> "
        "channel_map=<channel map>"
        "buffer_size=<custom buffer size>"
        "file_path=<file path for data reading>"
        "adapter_name=<primary>"
        "open_mic_speaker<open mic>"
        "network_id<device network id>"
        "device_type<device type or port>"
        "source_type<source type or port>"
    );

static const char * const VALID_MODARGS[] = {
    "source_name",
    "device_class",
    "source_properties",
    "format",
    "rate",
    "channels",
    "channel_map",
    "buffer_size",
    "file_path",
    "adapter_name",
    "open_mic_speaker",
    "network_id",
    "device_type",
    "source_type",
    "ec_type",
    "ec_adapter",
    "ec_sampling_rate",
    "ec_format",
    "ec_channels",
    "open_mic_ref",
    "mic_ref_rate",
    "mic_ref_format",
    "mic_ref_channels",
    NULL
};

static void IncreaseScenekeyCount(pa_hashmap *sceneMap, const char *key)
{
    if (sceneMap == NULL) {
        return;
    }
    char *sceneKey;
    uint32_t *num = NULL;
    if ((num = (uint32_t *)pa_hashmap_get(sceneMap, key)) != NULL) {
        (*num)++;
    } else {
        sceneKey = strdup(key);
        num = pa_xnew0(uint32_t, 1);
        *num = 1;
        pa_hashmap_put(sceneMap, sceneKey, num);
    }
}


static bool DecreaseScenekeyCount(pa_hashmap *sceneMap, const char *key)
{
    if (sceneMap == NULL) {
        return false;
    }
    uint32_t *num = NULL;
    if ((num = (uint32_t *)pa_hashmap_get(sceneMap, key)) != NULL) {
        (*num)--;
        if (*num == 0) {
            pa_hashmap_remove_and_free(sceneMap, key);
            return true;
        }
    }
    return false;
}

static void SetResampler(pa_source_output *so, const pa_sample_spec *algoConfig,
    const char *sceneKey, pa_hashmap *resamplerMap)
{
    AUDIO_INFO_LOG("SOURCE rate = %{public}d ALGO rate = %{public}d ",
        so->source->sample_spec.rate, algoConfig->rate);
    if (!pa_sample_spec_equal(&so->source->sample_spec, algoConfig)) {
        pa_resampler *preResampler = pa_resampler_new(so->source->core->mempool,
            &so->source->sample_spec, &so->source->channel_map,
            algoConfig, &so->source->channel_map,
            so->source->core->lfe_crossover_freq,
            PA_RESAMPLER_AUTO,
            PA_RESAMPLER_VARIABLE_RATE);
        pa_hashmap_put(resamplerMap, pa_xstrdup(sceneKey), preResampler);
        pa_resampler_set_input_rate(so->thread_info.resampler, algoConfig->rate);
    }
}

static uint32_t GetByteSizeByFormat(enum HdiAdapterFormat format)
{
    uint32_t byteSize = 0;
    switch (format) {
        case SAMPLE_U8:
            byteSize = BYTE_SIZE_SAMPLE_U8;
            break;
        case SAMPLE_S16:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
        case SAMPLE_S24:
            byteSize = BYTE_SIZE_SAMPLE_S24;
            break;
        case SAMPLE_S32:
            byteSize = BYTE_SIZE_SAMPLE_S32;
            break;
        default:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
    }
    return byteSize;
}

static void InitDeviceAttrAdapter(struct DeviceAttrAdapter *deviceAttrAdapter, struct Userdata *u)
{
    deviceAttrAdapter->micRate = u->attrs.sampleRate;
    deviceAttrAdapter->micChannels = u->attrs.channel;
    deviceAttrAdapter->micFormat = GetByteSizeByFormat(u->attrs.format);
    deviceAttrAdapter->needEc = false;
    deviceAttrAdapter->needMicRef = false;
    if (u->ecType != EC_NONE) {
        deviceAttrAdapter->needEc = true;
        deviceAttrAdapter->ecRate = u->ecSamplingRate;
        deviceAttrAdapter->ecChannels = u->ecChannels;
        deviceAttrAdapter->ecFormat = (uint32_t)u->ecFormat;
    } else {
        deviceAttrAdapter->ecRate = 0;
        deviceAttrAdapter->ecChannels = 0;
        deviceAttrAdapter->ecFormat = 0;
    }
    if (u->micRef == REF_ON) {
        deviceAttrAdapter->needMicRef = true;
        deviceAttrAdapter->micRefRate = u->micRefRate;
        deviceAttrAdapter->micRefChannels = u->micRefChannels;
        deviceAttrAdapter->micRefFormat = (uint32_t)u->micRefFormat;
    } else {
        deviceAttrAdapter->micRefRate = 0;
        deviceAttrAdapter->micRefChannels = 0;
        deviceAttrAdapter->micRefFormat = 0;
    }
}

static pa_hook_result_t SourceOutputPutCb(pa_core *c, pa_source_output *so)
{
    struct Userdata *u = (struct Userdata *)so->source->userdata;
    if (u == NULL) {
        AUDIO_ERR_LOG("Get Userdata failed! userdata is NULL");
        return PA_HOOK_OK;
    }
    AUDIO_INFO_LOG("Trigger SourceOutputPutCb");
    pa_assert(c);
    const char *sceneType = pa_proplist_gets(so->proplist, "scene.type");
    uint32_t captureId = u->captureId;
    uint32_t renderId = u->renderId;
    uint32_t sceneTypeCode = 0;
    if (GetSceneTypeCode(sceneType, &sceneTypeCode) != 0) {
        AUDIO_ERR_LOG("GetSceneTypeCode failed");
        return PA_HOOK_OK;
    }
    uint32_t sceneKeyCode = 0;
    sceneKeyCode = (sceneTypeCode << SCENE_TYPE_OFFSET) + (captureId << CAPTURER_ID_OFFSET) + renderId;
    if (EnhanceChainManagerCreateCb(sceneKeyCode) != 0) {
        AUDIO_INFO_LOG("Create EnhanceChain failed, set to bypass");
        pa_proplist_sets(so->proplist, "scene.bypass", DEFAULT_SCENE_BYPASS);
        return PA_HOOK_OK;
    }
    struct DeviceAttrAdapter deviceAttrAdapter;
    InitDeviceAttrAdapter(&deviceAttrAdapter, u);
    EnhanceChainManagerSetDeviceAttr(sceneKeyCode, deviceAttrAdapter);
    EnhanceChainManagerInitEnhanceBuffer();
    char sceneKey[MAX_SCENE_NAME_LEN];
    if (sprintf_s(sceneKey, sizeof(sceneKey), "%u", sceneKeyCode) < 0) {
        AUDIO_ERR_LOG("sprintf from sceneKeyCode to sceneKey failed");
        return PA_HOOK_OK;
    }
    IncreaseScenekeyCount(u->sceneToCountMap, sceneKey);
    pa_sample_spec algoConfig;
    pa_sample_spec_init(&algoConfig);
    if (EnhanceChainManagerGetAlgoConfig(sceneKeyCode, &algoConfig) != 0) {
        AUDIO_ERR_LOG("Get algo config failed");
        return PA_HOOK_OK;
    }
    SetResampler(so, &algoConfig, sceneKey, u->sceneToResamplerMap);
    return PA_HOOK_OK;
}

static pa_hook_result_t SourceOutputUnlinkCb(pa_core *c, pa_source_output *so)
{
    AUDIO_INFO_LOG("Trigger SourceOutputUnlinkCb");
    struct Userdata *u = (struct Userdata *)so->source->userdata;
    if (u == NULL) {
        AUDIO_ERR_LOG("Get Userdata failed! userdata is NULL");
        return PA_HOOK_OK;
    }
    pa_assert(c);
    const char *sceneType = pa_proplist_gets(so->proplist, "scene.type");
    uint32_t captureId = u->captureId;
    uint32_t renderId = u->renderId;
    uint32_t sceneTypeCode = 0;
    if (GetSceneTypeCode(sceneType, &sceneTypeCode) != 0) {
        AUDIO_ERR_LOG("GetSceneTypeCode failed");
        return PA_HOOK_OK;
    }
    uint32_t sceneKeyCode = 0;
    sceneKeyCode = (sceneTypeCode << SCENE_TYPE_OFFSET) + (captureId << CAPTURER_ID_OFFSET) + renderId;
    EnhanceChainManagerReleaseCb(sceneKeyCode);
    
    char sceneKey[MAX_SCENE_NAME_LEN];
    if (sprintf_s(sceneKey, sizeof(sceneKey), "%u", sceneKeyCode) < 0) {
        AUDIO_ERR_LOG("sprintf from sceneKeyCode to sceneKey failed");
        return PA_HOOK_OK;
    }
    if (DecreaseScenekeyCount(u->sceneToCountMap, sceneKey)) {
        pa_hashmap_remove_and_free(u->sceneToResamplerMap, sceneKey);
    }
    return PA_HOOK_OK;
}

int pa__init(pa_module *m)
{
    pa_modargs *ma = NULL;

    pa_assert(m);

    if (!(ma = pa_modargs_new(m->argument, VALID_MODARGS))) {
        pa_log("Failed to parse module arguments");
        goto fail;
    }

    if (!(m->userdata = PaHdiSourceNew(m, ma, __FILE__))) {
        goto fail;
    }

    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_PUT], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputPutCb, NULL);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_UNLINK], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputUnlinkCb, NULL);

    pa_modargs_free(ma);

    return 0;

fail:

    if (ma) {
        pa_modargs_free(ma);
    }

    pa__done(m);

    return -1;
}

int pa__get_n_used(pa_module *m)
{
    pa_source *source = NULL;

    pa_assert(m);
    pa_assert_se(source = m->userdata);

    return pa_source_linked_by(source);
}

void pa__done(pa_module *m)
{
    pa_source *source = NULL;

    pa_assert(m);

    if ((source = m->userdata)) {
        PaHdiSourceFree(source);
    }
}
