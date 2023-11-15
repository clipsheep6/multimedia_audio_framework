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

#ifndef I_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_H
#define I_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AudioStandard {
class IStandardSpatializationEnabledChangeListener : public IRemoteBroker {
public:
    virtual ~IStandardSpatializationEnabledChangeListener() = default;
    virtual void OnSpatializationEnabledChange(const bool &enabled) = 0;

    enum AudioSpatializationEnabledChangeListenerMsg {
        ON_ERROR = 0,
        ON_SPATIALIZATION_ENABLED_CHANGE,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardSpatializationEnabledChangeListener");
};

class IStandardHeadTrackingEnabledChangeListener : public IRemoteBroker {
public:
    virtual ~IStandardHeadTrackingEnabledChangeListener() = default;
    virtual void OnHeadTrackingEnabledChange(const bool &enabled) = 0;

    enum AudioHeadTrackingEnabledChangeListenerMsg {
        ON_ERROR = 0,
        ON_HEAD_TRACKING_ENABLED_CHANGE,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardHeadTrackingEnabledChangeListener");
};

class IStandardSpatializationStateChangeListener : public IRemoteBroker {
public:
    virtual ~IStandardSpatializationStateChangeListener() = default;
    virtual void OnSpatializationStateChange(const std::vector<bool> &spatializationState) = 0;

    enum AudioSpatializationStateChangeListenerMsg {
        ON_ERROR = 0,
        ON_SPATIALIZATION_STATE_CHANGE,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardSpatializationStateChangeListener");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_H
