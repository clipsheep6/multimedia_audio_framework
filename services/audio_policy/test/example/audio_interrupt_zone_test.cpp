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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "audio_policy_manager.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;
using namespace std;

static void CreateAudioInterruptZone(set<int32_t> pids, int32_t zoneID)
{
    cout << "CreateAudioInterruptZone  zoneID: " << zoneID << endl;
    AudioPolicyManager::GetInstance().CreateAudioInterruptZone(pids, zoneID);
}

static void AddAudioInterruptZonePids(set<int32_t> pids, int32_t zoneID)
{
    cout << "AddAudioInterruptZonePids  zoneID: " << zoneID << endl;
    AudioPolicyManager::GetInstance().AddAudioInterruptZonePids(pids, zoneID);
}

static void RemoveAudioInterruptZonePids(set<int32_t> pids, int32_t zoneID)
{
    cout << "RemoveAudioInterruptZonePids  zoneID: " << zoneID << endl;
    AudioPolicyManager::GetInstance().RemoveAudioInterruptZonePids(pids, zoneID);
}


static void ReleaseAudioInterruptZone(int32_t zoneID)
{
    cout << "ReleaseAudioInterruptZone  zoneID: " << zoneID << endl;
    AudioPolicyManager::GetInstance().ReleaseAudioInterruptZone(zoneID);
}


int main(int argc, char* argv[])
{
    constexpr int minRequiredArgCount = 2;
    string path;
    if (argc >= minRequiredArgCount && argv[1] != nullptr) {
        path = argv[1];
    }
    while (true) {
        cout << "Please select a scenario number(default CreateAudioInterruptZone):" << endl;
        cout << "0:CreateAudioInterruptZone" << endl;
        cout << "1:AddAudioInterruptZonePids" << endl;
        cout << "2:RemoveAudioInterruptZonePids" << endl;
        cout << "3:ReleaseAudioInterruptZone" << endl;

        string mode;
        (void)getline(cin, mode);

        if (mode == "" || mode == "0") {
            cout << "CreateAudioInterruptZone -- Input ZoneID pls(integer num):" << endl;
            string zoneID;
            (void)getline(cin, zoneID);
            cout << "CreateAudioInterruptZone -- Input pid pls(integer num):" << endl;
            string pid;
            set<int32_t> pids;
            (void)getline(cin, pid);
            pids.insert(stoi(pid));
            CreateAudioInterruptZone(pids, stoi(zoneID));
        } else if (mode == "1") {
            cout << "AddAudioInterruptZonePids -- Input ZoneID pls(integer num):" << endl;
            string zoneID;
            (void)getline(cin, zoneID);
            cout << "AddAudioInterruptZonePids -- Input pid pls(integer num):" << endl;
            string pid;
            set<int32_t> pids;
            (void)getline(cin, pid);
            pids.insert(stoi(pid));
            AddAudioInterruptZonePids(pids, stoi(zoneID));
        } else if (mode == "2") {
            cout << "RemoveAudioInterruptZonePids -- Input ZoneID pls(integer num):" << endl;
            string zoneID;
            (void)getline(cin, zoneID);
            cout << "RemoveAudioInterruptZonePids -- Input pid pls(integer num):" << endl;
            string pid;
            set<int32_t> pids;
            (void)getline(cin, pid);
            pids.insert(stoi(pid));
            RemoveAudioInterruptZonePids(pids, stoi(zoneID));
        } else if (mode == "3") {
            cout << "ReleaseAudioInterruptZone -- Input ZoneID pls(integer num):" << endl;
            string zoneID;
            (void)getline(cin, zoneID);
            ReleaseAudioInterruptZone(stoi(zoneID));
        } else {
            cout << "Invalid cmd." << endl;
        }

        cout << "Do you want to exit: y/n?" << endl;
        string cmd;
        (void)getline(cin, cmd);

        if (cmd == "y") {
            cout << "Bye Bye." << endl;
            break;
        }
    }
    return 0;
}
