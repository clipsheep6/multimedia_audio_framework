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
#ifndef ST_AUDIO_DEVICE_MANAGER_H
#define ST_AUDIO_DEVICE_MANAGER_H

#include <list>
#include <string>
#include <memory>
#include <unordered_map>
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

typedef function<bool(const std::unique_ptr<AudioDeviceDescriptor> &desc)> IsPresentFunc;
std::string GetEncryptAddr(const std::string &addr);
class AudioAffinityDeviceManager {
public:
    static AudioAffinityDeviceManager& GetAudioAffinityDeviceManager()
    {
        static AudioAffinityDeviceManager audioAffinityDeviceManager;
        return audioAffinityDeviceManager;
    }
    
private:
    AudioAffinityDeviceManager();
    ~AudioAffinityDeviceManager() {};
    
};
} // namespace AudioStandard
} // namespace OHOS
#endif //ST_AUDIO_DEVICE_MANAGER_H
