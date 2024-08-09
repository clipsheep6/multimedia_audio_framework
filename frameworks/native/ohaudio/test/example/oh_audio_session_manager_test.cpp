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

 #include "native_audio_session_manager.h"
 #include "native_audio_common.h"
 #include "audio_session_manager.h"
 #include <ostream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

class SessionNdkTest {
public:
    SessionNdkTest() = default;
    ~SessionNdkTest() = default;
    void init();
    void registerCallback(OH_AudioSession_DeactivatedCallback callback);
    void unRegisterCallback(OH_AudioSession_DeactivatedCallback callback);
    void activateAudioSession(OH_AudioSession_Strategy strategy);
    void deActivateAudioSession();
    void IsAudioSessionActivated();
    int32_t myCallbackFunction(OH_AudioSession_DeactivatedEvent event);
};

OH_AudioSessionManager *audioSessionManager;

OH_AudioSession_DeactivatedEvent event;

shared_ptr<SessionNdkTest> g_sessionNdkTest = nullptr;

OH_AudioSession_Strategy strategy;

void SessionNdkTest::init()
{
    OH_AudioManager_GetAudioSessionManager(&audioSessionManager);
}

void SessionNdkTest::registerCallback(OH_AudioSession_DeactivatedCallback callback)
{
    cout << "Start Register callback" << endl;
    OH_AudioCommon_Result result =
        OH_AudioSessionManager_RegisterSessionDeactivatedCallback(audioSessionManager, callback);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        cout << "Register callback SUCCESS" << endl;
    } else {
        cout << "Register callback FAILED" << endl;
    }
}

void SessionNdkTest::unRegisterCallback(OH_AudioSession_DeactivatedCallback callback)
{
    cout << "Start UnRegister callback" << endl;
    OH_AudioCommon_Result result =
        OH_AudioSessionManager_UnregisterSessionDeactivatedCallback(audioSessionManager, callback);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        cout << "UnRegister callback SUCCESS" << endl;
    } else {
        cout << "UnRegister callback FAILED" << endl;
    }
}

void SessionNdkTest::activateAudioSession(OH_AudioSession_Strategy strategy)
{
    cout << "Start Activate AudioSession" << endl;
    OH_AudioCommon_Result result =
        OH_AudioSessionManager_ActivateAudioSession(audioSessionManager, strategy);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        cout << "Activate AudioSession SUCCESS" << endl;
    } else {
        cout << "Activate AudioSession FAILED" << endl;
    }
}

void SessionNdkTest::deActivateAudioSession()
{
    cout << "Start DeActivate AudioSession" << endl;
    OH_AudioCommon_Result result = OH_AudioSessionManager_DeactivateAudioSession(audioSessionManager);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        cout << "DeActivate AudioSession SUCCESS" << endl;
    } else {
        cout << "DeActivate AudioSession FAILED" << endl;
    }
}

void SessionNdkTest::IsAudioSessionActivated()
{
    cout << "Start Is Activate AudioSession" << endl;
    bool result = OH_AudioSessionManager_IsAudioSessionActivated(audioSessionManager);
    if (result) {
        cout << "Is Activate AudioSession SUCCESS" << endl;
    } else {
        cout << "Is Activate AudioSession FAILED" << endl;
    }
}

int32_t myCallbackFunction(OH_AudioSession_DeactivatedEvent event)
{
    cout << "Callback For Event" << endl;
    return 0;
}

} // namespace AudioStandard
} // namespace OHOS


using namespace OHOS::AudioStandard;
int main()
{
    // Init Get AudioSessionManager
    g_sessionNdkTest = make_shared<SessionNdkTest>();
    
    g_sessionNdkTest->init();

    cout << "Init Completed, Start Test" << endl;

    bool runFlag = true;
    int operate;

    OH_AudioSession_DeactivatedCallback callback = myCallbackFunction;

    while(runFlag) {
        cout << "Please Input Operate" << endl;
        cout << "1 -----> Register Callback" << endl;
        cout << "2 -----> UnRegister Callback" << endl;
        cout << "3 -----> Activate AudioSession" << endl;
        cout << "4 -----> DeActivate AudioSession" << endl;
        cout << "5 -----> Is Activate Judging" << endl;
        cout << "0 -----> Exit" << endl;

        cin >> operate;

        if (operate == 0) {
            runFlag = false;
        }

        switch(operate) {
            case 1:
                cout << "Start Build Event" << endl;
                event.reason = DEACTIVATED_LOWER_PRIORITY;
                cout << "Build Event Completed; Use Event Start Build Callback" << endl;
                callback(event);
                cout << "Build Call Completed" << endl;
                g_sessionNdkTest->registerCallback(callback);
                break;
            case 2:
                event.reason = DEACTIVATED_TIMEOUT;
                callback(event);
                g_sessionNdkTest->unRegisterCallback(callback);
                break;
            case 3:
                strategy.concurrencyMode = CONCURRENCY_MIX_WITH_OTHERS;
                g_sessionNdkTest->activateAudioSession(strategy);
                break;
            case 4:
                g_sessionNdkTest->deActivateAudioSession();
                break;
            case 5:
                g_sessionNdkTest->IsAudioSessionActivated();
                break;
            default:
                cout << "Input Valid, RE Input";
                break;
        }
    }
    cout << "End Test" << endl;
}