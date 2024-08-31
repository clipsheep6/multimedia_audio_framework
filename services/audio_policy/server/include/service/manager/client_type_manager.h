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

#ifndef CLIENT_TYPE_MANAGER_H
#define CLIENT_TYPE_MANAGER_H

#include <unordered_map>
#include <string>

namespace OHOS {
namespace AudioStandard {
using namespace std;
enum ClientType {
    CLIENT_TYPE_OTHERS = 0,
    CLIENT_TYPE_GAME = 1,
};

class ClientTypeManager {
public:
    static ClientTypeManager *GetInstance();
    void GetAndSaveClientType(uint32_t uid, const std::string &bundleName);
    ClientType GetClientTypeByUid(uint32_t uid);

private:
    std::unordered_map<uint32_t, ClientType> clientTypeMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // CLIENT_TYPE_MANAGER_H
