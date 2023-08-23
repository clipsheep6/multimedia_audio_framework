/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <cstdint>
#include <string>
#include <map>

#define AUDIO_MS_PER_SECOND 1000
#define AUDIO_US_PER_SECOND 1000000
#define AUDIO_NS_PER_SECOND ((int64_t)1000000000)
namespace OHOS {
namespace AudioStandard {
class Trace {
public:
    static void Count(const std::string &value, int64_t count, bool isEnable = true);
    Trace(const std::string &value, bool isShowLog = false, bool isEnable = true);
    void End();
    ~Trace();
private:
    std::string value_;
    bool isShowLog_;
    bool isEnable_;
    bool isFinished_;
};

class ClockTime {
public:
    static int64_t GetCurNano();
    static int32_t AbsoluteSleep(int64_t nanoTime);
    static int32_t RelativeSleep(int64_t nanoTime);
};

class PermissionUtil {
public:
    static bool VerifyIsSystemApp();
    static bool VerifySelfPermission();
    static bool VerifySystemPermission();
};

void AdjustStereoToMonoForPCM8Bit(int8_t *data, uint64_t len);
void AdjustStereoToMonoForPCM16Bit(int16_t *data, uint64_t len);
void AdjustStereoToMonoForPCM24Bit(int8_t *data, uint64_t len);
void AdjustStereoToMonoForPCM32Bit(int32_t *data, uint64_t len);
void AdjustAudioBalanceForPCM8Bit(int8_t *data, uint64_t len, float left, float right);
void AdjustAudioBalanceForPCM16Bit(int16_t *data, uint64_t len, float left, float right);
void AdjustAudioBalanceForPCM24Bit(int8_t *data, uint64_t len, float left, float right);
void AdjustAudioBalanceForPCM32Bit(int32_t *data, uint64_t len, float left, float right);

template <typename T>
bool GetSysPara(const char *key, T &value);

enum AudioDumpFileType {
    AUDIO_APP = 0,
    AUDIO_SERVICE = 1,
    AUDIO_PULSE = 2,
};

class DumpFileUtil {
public:
    static FILE *OpenDumpFile(std::string para, std::string fileName, AudioDumpFileType fileType);
    static int32_t WriteDumpFile(FILE *dumpFile, void *buffer, size_t bufferSize);
    static void CloseDumpFile(FILE **dumpFile);
    static int32_t ChangeDumpFileState(std::string para, FILE **dumpFile, std::string fileName,
        AudioDumpFileType fileType);
    static std::map<std::string, std::string> g_lastPara;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_UTILS_H
