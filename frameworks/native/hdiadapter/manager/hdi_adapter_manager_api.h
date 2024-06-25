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

#ifndef HDI_ADAPTER_MANAGER_API_H
#define HDI_ADAPTER_MANAGER_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HdiCaptureHandle {
    int32_t halType;
    int32_t deviceType;

    int32_t (*Init)(const CaptureAttr *attr);
    void (*Deinit)();
    int32_t (*Start)();
    int32_t (*Stop)();
    int32_t (*CaptureFrame)(
        char *frame, uint64_t requestBytes, uint64_t *replyBytes);
    int32_t (*CaptureFrameWithEc)(
        char *frame, uint64_t requestBytes, uint64_t *replyBytes,
        char *frameEc, uint64_t requestBytesEc, uint64_t *replyBytesEc);
};

int32_t GetEcCaptureInstance();
int32_t GetRefCaptureInstance();


#ifdef __cplusplus
}
#endif
#endif // HDI_ADAPTER_MANAGER_API_H