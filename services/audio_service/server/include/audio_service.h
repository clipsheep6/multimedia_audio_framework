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

#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

#include <sstream>
#include <map>
#include <mutex>
#include <vector>

#include "audio_process_in_server.h"
#include "audio_endpoint.h"

namespace OHOS {
namespace AudioStandard {
class AudioService : public ProcessReleaseCallback {
public:
    static AudioService *GetInstance();
    ~AudioService();
    sptr<AudioProcessInServer> GetAudioProcess(const AudioProcessConfig &config);
    // override for ProcessReleaseCallback, do release process work.
    int32_t OnProcessRelease(IAudioProcessStream *process) override;

    DeviceInfo GetDeviceInfoForProcess(const AudioProcessConfig &config, bool isRemote);
    std::shared_ptr<AudioEndpoint> GetAudioEndpointForDevice(DeviceInfo deviceInfo);

    int32_t LinkProcessToEndpoint(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    int32_t UnlinkProcessToEndpoint(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    void Dump(std::stringstream &dumpString);
    int32_t ChangeProcessToEndpoint(sptr<AudioProcessInServer> process, const AudioProcessConfig &config);
private:
    AudioService();
    void Dump();
    std::mutex processListMutex_;
    std::set<sptr<AudioProcessInServer>> processList_;
    std::vector<std::pair<sptr<AudioProcessInServer>, std::shared_ptr<AudioEndpoint>>> linkedPairedList_;
    std::map<std::string, std::shared_ptr<AudioEndpoint>> endpointList_;

    // for change endpoint.. 
    sptr<AudioProcessInServer> process_;
    bool curInRemote_ = true;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVICE_H
