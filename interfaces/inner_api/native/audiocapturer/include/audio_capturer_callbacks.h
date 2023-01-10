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

#ifndef AUDIO_CAPTURER_CALLBACKS_H
#define AUDIO_CAPTURER_CALLBACKS_H

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief Defines callbacks used in audio capturer and related classes.
 */
class AudioCapturerCallback {
public:
    virtual ~AudioCapturerCallback() = default;

    /**
     * Called when an interrupt is received.
     *
     * @param interruptEvent Indicates the InterruptEvent information needed by client.
     * For details, refer InterruptEvent struct in audio_info.h
     */
    virtual void OnInterrupt(const InterruptEvent &interruptEvent) = 0;

    /**
     * Called when capturer state is updated.
     *
     * @param state Indicates updated state of the capturer.
     * For details, refer enum CapturerState.
     */
    virtual void OnStateChange(const CapturerState state) = 0;
};

class CapturerPositionCallback {
public:
    virtual ~CapturerPositionCallback() = default;

    /**
     * Called when the requested frame number is read.
     *
     * @param framePosition requested frame position.
     */
    virtual void OnMarkReached(const int64_t &framePosition) = 0;
};

class CapturerPeriodPositionCallback {
public:
    virtual ~CapturerPeriodPositionCallback() = default;

    /**
     * Called when the requested frame count is read.
     *
     * @param frameCount requested frame frame count for callback.
     */
    virtual void OnPeriodReached(const int64_t &frameNumber) = 0;
};

class AudioCapturerReadCallback {
public:
    virtual ~AudioCapturerReadCallback() = default;

    /**
     * Called when buffer to be enqueued.
     *
     * @param length Indicates requested buffer length.
     */
    virtual void OnReadData(size_t length) = 0;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_CAPTURER_CALLBACKS_H
