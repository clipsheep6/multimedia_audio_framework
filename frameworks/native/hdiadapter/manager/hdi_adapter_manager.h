/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef HDI_ADAPTER_MANAGER_H
#define HDI_ADAPTER_MANAGER_H

#include <cinttypes>
#include <string>

namespace OHOS {
namespace AudioStandard {

class HdiAdapterManager {
public:
    static std::shared_ptr<HdiAdapterManager> GetInstance();

    int32_t Init();
    int32_t LoadAdapter(const std::string &adapterName);
    int32_t UnLoadAdapter(const std::string &adapterName);
    int32_t CreateCapture(const std::string &adapterName, XXFormat format);
    int32_t DestroyCapture(const std::string &adapterName, XXFormat format);
    std::shared_ptr<IAudioCapturerSource> GetCapture();

private:
    HdiAdapterManager();
    virtual ~HdiAdapterManager();

};

} // namespace AudioStandard
} // namespace OHOS

#endif // HDI_ADAPTER_MANAGER_H