/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const int64_t LIMIT_TIME = 1;
bool g_hasPermission = false;

void AudioFuzzTestGetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void GetCurNanoFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    ClockTime::GetCurNano();
}
void AbsoluteSleepFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(int64_t)) {
        return;
    }

    int64_t nanoTime = *(reinterpret_cast<const int64_t*>(rawData));
    if (nanoTime > LIMIT_TIME) {
        nanoTime = LIMIT_TIME;
    }
    ClockTime::AbsoluteSleep(nanoTime);
}

void RelativeSleepFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(int64_t)) {
        return;
    }

    int64_t nanoTime = *(reinterpret_cast<const int64_t*>(rawData));
    if (nanoTime > LIMIT_TIME) {
        nanoTime = LIMIT_TIME;
    }
    ClockTime::RelativeSleep(nanoTime);
}

void CountFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size <= sizeof(int64_t)) {
        return;
    }

    int64_t count = *(reinterpret_cast<const int64_t*>(rawData));
    rawData += sizeof(count);
    size -= sizeof(count);
    const std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    Trace::Count(value, count);
}

void CountVolumeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size <= sizeof(uint8_t)) {
        return;
    }

    uint8_t data = *(reinterpret_cast<const uint8_t*>(rawData));
    rawData += sizeof(data);
    size -= sizeof(data);
    const std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    Trace::CountVolume(value, data);
}

bool VerifySystemPermissionFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return false;
    }

    return PermissionUtil::VerifySystemPermission();
}

void VerifyPermissionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t tokenId = *reinterpret_cast<const uint32_t *>(rawData);
    rawData += sizeof(uint32_t);
    PermissionUtil::VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION, tokenId);
}

void VerifyIsShellFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    PermissionUtil::VerifyIsShell();
}

void VerifyIsSystemAppFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    PermissionUtil::VerifyIsSystemApp();
}


void NeedVerifyBackgroundCaptureFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(SourceType)) {
        return;
    }
    int32_t callingUid = *(reinterpret_cast<const int32_t*>(rawData));
    SourceType sourceType = *(reinterpret_cast<const SourceType*>(rawData));
    PermissionUtil::NeedVerifyBackgroundCapture(callingUid, sourceType);
}

void VerifyBackgroundCaptureFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    uint32_t tokenId = *(reinterpret_cast<const uint32_t*>(rawData));
    uint64_t fullTokenId = *(reinterpret_cast<const uint64_t*>(rawData));

    PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId);
}

void NotifyPrivacyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(AudioPermissionState)) {
        return;
    }
    uint32_t targetTokenId = *(reinterpret_cast<const uint32_t*>(rawData));
    AudioPermissionState state = *(reinterpret_cast<const AudioPermissionState*>(rawData));

    PermissionUtil::NotifyPrivacy(targetTokenId, state);
}

void AdjustStereoToMonoForPCM8BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    int8_t *data = const_cast<int8_t*>(reinterpret_cast<const int8_t*>(rawData));
    uint64_t len = *(reinterpret_cast<const uint64_t*>(rawData));
    AdjustStereoToMonoForPCM8Bit(data, len);
}

void AdjustStereoToMonoForPCM16BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    int16_t *data = const_cast<int16_t*>(reinterpret_cast<const int16_t*>(rawData));
    uint64_t len = *(reinterpret_cast<const uint64_t*>(rawData));
    AdjustStereoToMonoForPCM16Bit(data, len);
}

void AdjustStereoToMonoForPCM24BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    int8_t *data = const_cast<int8_t*>(reinterpret_cast<const int8_t*>(rawData));
    uint64_t len = *(reinterpret_cast<const uint64_t*>(rawData));
    AdjustStereoToMonoForPCM24Bit(data, len);
}

void AdjustStereoToMonoForPCM32BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    int32_t *data = const_cast<int32_t*>(reinterpret_cast<const int32_t*>(rawData));
    uint64_t len = *(reinterpret_cast<const uint64_t*>(rawData));
    AdjustStereoToMonoForPCM32Bit(data, len);
}

void AdjustAudioBalanceForPCM8BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    int8_t *data = const_cast<int8_t*>(reinterpret_cast<const int8_t*>(rawData));
    uint64_t len = *(reinterpret_cast<const uint64_t*>(rawData));
    float left = *(reinterpret_cast<const float*>(rawData));
    float right = *(reinterpret_cast<const float*>(rawData));
    AdjustAudioBalanceForPCM8Bit(data, len, left, right);
}

void AdjustAudioBalanceForPCM16BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    int16_t *data = const_cast<int16_t*>(reinterpret_cast<const int16_t*>(rawData));
    uint64_t len = *(reinterpret_cast<const uint64_t*>(rawData));
    float left = *(reinterpret_cast<const float*>(rawData));
    float right = *(reinterpret_cast<const float*>(rawData));
    AdjustAudioBalanceForPCM16Bit(data, len, left, right);
}

void AdjustAudioBalanceForPCM24BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    int8_t *data = const_cast<int8_t*>(reinterpret_cast<const int8_t*>(rawData));
    uint64_t len = *(reinterpret_cast<const uint64_t*>(rawData));
    float left = *(reinterpret_cast<const float*>(rawData));
    float right = *(reinterpret_cast<const float*>(rawData));
    AdjustAudioBalanceForPCM24Bit(data, len, left, right);
}

void AdjustAudioBalanceForPCM32BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    int32_t *data = const_cast<int32_t*>(reinterpret_cast<const int32_t*>(rawData));
    uint64_t len = *(reinterpret_cast<const uint64_t*>(rawData));
    float left = *(reinterpret_cast<const float*>(rawData));
    float right = *(reinterpret_cast<const float*>(rawData));
    AdjustAudioBalanceForPCM32Bit(data, len, left, right);
}

void ConvertFrom24BitToFloatFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size <sizeof(uint)) {
        return;
    }
    uint n = *(reinterpret_cast<const uint*>(rawData));
    const uint8_t *a = (reinterpret_cast<const uint8_t*>(rawData));
    float *b = const_cast<float*>(reinterpret_cast<const float*>(rawData));
    ConvertFrom24BitToFloat(n, a, b);
}

void ConvertFrom32BitToFloatFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size <sizeof(uint)) {
        return;
    }
    uint n = *(reinterpret_cast<const uint*>(rawData));
    const int32_t *a = (reinterpret_cast<const int32_t*>(rawData));
    float *b = const_cast<float*>(reinterpret_cast<const float*>(rawData));
    ConvertFrom32BitToFloat(n, a, b);
}

void ConvertFromFloatTo24BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size <sizeof(uint)) {
        return;
    }
    uint n = *(reinterpret_cast<const uint*>(rawData));
    uint8_t *a = const_cast<uint8_t *>(reinterpret_cast<const uint8_t*>(rawData));
    const float *b = (reinterpret_cast<const float*>(rawData));
    ConvertFromFloatTo24Bit(n, b, a);
}

void ConvertFromFloatTo32BitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size <sizeof(uint)) {
        return;
    }
    uint n = *(reinterpret_cast<const uint*>(rawData));
    int32_t *a = const_cast<int32_t *>(reinterpret_cast<const int32_t*>(rawData));
    const float *b = (reinterpret_cast<const float*>(rawData));
    ConvertFromFloatTo32Bit(n, b, a);
}

void UpdateMaxAmplitudeFuzzTest(const uint8_t * rawData, size_t size)
{
    if (rawData == nullptr || size == 0 || size < sizeof(ConvertHdiFormat)) {
        return;
    }
    ConvertHdiFormat format = *(reinterpret_cast<const ConvertHdiFormat*>(rawData));
    char* frame = const_cast<char*>(reinterpret_cast<const char*>(rawData));
    uint64_t replyBytes = *(reinterpret_cast<const uint64_t*>(rawData));
    UpdateMaxAmplitude(format, frame, replyBytes);
}

void CalculateMaxAmplitudeForPCM8BitFuzzTest(const uint8_t * rawData, size_t size)
{
    if (rawData == nullptr || size == 0 || sizeof(uint64_t)) {
        return;
    }
    int8_t *frame = const_cast<int8_t*>(reinterpret_cast<const int8_t*>(rawData));
    uint64_t nSamples = *(reinterpret_cast<const uint64_t*>(rawData));
    CalculateMaxAmplitudeForPCM8Bit(frame, nSamples);
}

void CalculateMaxAmplitudeForPCM16BitFuzzTest(const uint8_t * rawData, size_t size)
{
    if (rawData == nullptr || size == 0 ||  sizeof(uint64_t)) {
        return;
    }
    int16_t *frame = const_cast<int16_t*>(reinterpret_cast<const int16_t*>(rawData));
    uint64_t nSamples = *(reinterpret_cast<const uint64_t*>(rawData));
    CalculateMaxAmplitudeForPCM16Bit(frame, nSamples);
}

void CalculateMaxAmplitudeForPCM24BitFuzzTest(const uint8_t * rawData, size_t size)
{
    if (rawData == nullptr || size == 0 || sizeof(uint64_t)) {
        return;
    }
    char *frame = const_cast<char*>(reinterpret_cast<const char*>(rawData));
    uint64_t nSamples = *(reinterpret_cast<const uint64_t*>(rawData));
    CalculateMaxAmplitudeForPCM24Bit(frame, nSamples);
}

void CalculateMaxAmplitudeForPCM32BitFuzzTest(const uint8_t * rawData, size_t size)
{
    if (rawData == nullptr || size == 0 || sizeof(uint64_t)) {
        return;
    }
    int32_t *frame = const_cast<int32_t*>(reinterpret_cast<const int32_t*>(rawData));
    uint64_t nSamples = *(reinterpret_cast<const uint64_t*>(rawData));
    CalculateMaxAmplitudeForPCM32Bit(frame, nSamples);
}

void GetTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size == 0) {
        return;
    }
    GetTime();
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    AUDIO_INFO_LOG("111111111111111");
    OHOS::AudioStandard::GetCurNanoFuzzTest(data, size);
    AUDIO_INFO_LOG("2222222222222");
    OHOS::AudioStandard::AbsoluteSleepFuzzTest(data, size);
    AUDIO_INFO_LOG("333333333333333");
    OHOS::AudioStandard::RelativeSleepFuzzTest(data, size);
    AUDIO_INFO_LOG("4444444444444444");
    OHOS::AudioStandard::CountFuzzTest(data, size);
    AUDIO_INFO_LOG("55555555555555555");
    OHOS::AudioStandard::CountVolumeFuzzTest(data, size);
    AUDIO_INFO_LOG("6666666666666666");
    OHOS::AudioStandard::AdjustStereoToMonoForPCM8BitFuzzTest(data, size);
    AUDIO_INFO_LOG("7777777777777777");
    OHOS::AudioStandard::AdjustStereoToMonoForPCM16BitFuzzTest(data, size);
    AUDIO_INFO_LOG("88888888888888888");
    OHOS::AudioStandard::AdjustStereoToMonoForPCM24BitFuzzTest(data, size);
    AUDIO_INFO_LOG("999999999999999");
    OHOS::AudioStandard::AdjustStereoToMonoForPCM32BitFuzzTest(data, size);
    AUDIO_INFO_LOG("0000000000000000");
    OHOS::AudioStandard::AdjustAudioBalanceForPCM8BitFuzzTest(data, size);
    AUDIO_INFO_LOG("121212121212");
    OHOS::AudioStandard::AdjustAudioBalanceForPCM16BitFuzzTest(data, size);
    AUDIO_INFO_LOG("13131313131313");
    OHOS::AudioStandard::AdjustAudioBalanceForPCM24BitFuzzTest(data, size);AUDIO_INFO_LOG("1414414141414");
    OHOS::AudioStandard::AdjustAudioBalanceForPCM32BitFuzzTest(data, size);AUDIO_INFO_LOG("15155115151");
    OHOS::AudioStandard::ConvertFrom24BitToFloatFuzzTest(data, size);AUDIO_INFO_LOG("1616161616");
    OHOS::AudioStandard::ConvertFrom32BitToFloatFuzzTest(data, size);AUDIO_INFO_LOG("171717177171");
    OHOS::AudioStandard::ConvertFromFloatTo24BitFuzzTest(data, size);AUDIO_INFO_LOG("181818181881");
    OHOS::AudioStandard::ConvertFromFloatTo32BitFuzzTest(data, size);AUDIO_INFO_LOG("191919191991");
    OHOS::AudioStandard::UpdateMaxAmplitudeFuzzTest(data, size);AUDIO_INFO_LOG("202020202020");
    OHOS::AudioStandard::CalculateMaxAmplitudeForPCM8BitFuzzTest(data, size);AUDIO_INFO_LOG("212121212121");
    OHOS::AudioStandard::CalculateMaxAmplitudeForPCM16BitFuzzTest(data, size);AUDIO_INFO_LOG("232323232323");
    OHOS::AudioStandard::CalculateMaxAmplitudeForPCM24BitFuzzTest(data, size);AUDIO_INFO_LOG("242424242424");
    OHOS::AudioStandard::CalculateMaxAmplitudeForPCM32BitFuzzTest(data, size);AUDIO_INFO_LOG("252525252525");
    OHOS::AudioStandard::GetTimeFuzzTest(data, size);AUDIO_INFO_LOG("262626262626262");
    return 0;
}
