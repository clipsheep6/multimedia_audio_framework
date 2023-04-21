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

#include <iostream>
#include <vector>
#include "functional"
#include "memory"
#include <dlfcn.h>
#include "unistd.h"
#include <string.h>
#include "audio_effect_server.h"
#include "audio_log.h"

# define AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR "AELI"
# define EFFECT_STRING_LEN_MAX 64

namespace OHOS {
namespace AudioStandard {

constexpr const char *LD_EFFECT_LIBRARY_PATH[] = {"/system/lib/"};

struct LoadEffectResult {
    bool success = false;
    libEntryT *lib = nullptr;
    std::unique_ptr<effectDescriptorT> effectDesc;
};

bool ResolveLibrary(const std::string &path, std::string &resovledPath)
{
    for (auto *libDir: LD_EFFECT_LIBRARY_PATH) {
        std::string candidatePath = std::string(libDir) + "/" + path;
        if (access(candidatePath.c_str(), R_OK) == 0) {
            resovledPath = std::move(candidatePath);
            return true;
        }
    }

    return false;
}

bool loadLibrary(const char *relativePath, std::unique_ptr<libEntryT>& libEntry) noexcept
{
    std::string absolutePath;
    // find library in adsolutePath
    if (!ResolveLibrary(relativePath, absolutePath)) {
        AUDIO_ERR_LOG("<log error> find library falied in effect directories: %{public}s", relativePath);
        libEntry->path = strdup(relativePath);
        return false;
    }

    // load hundle
    const char *path = absolutePath.c_str();
    libEntry->path = strdup(path);

    char *error;
    void* handle = dlopen(path, 1);
    if (!handle) {
        AUDIO_ERR_LOG("<log error> Open lib Fail");
        return false;
    } else {
        AUDIO_INFO_LOG("<log info> dlopen lib successful");
    }

    audioEffectLibraryT *description = static_cast<audioEffectLibraryT *>(dlsym(handle,
        AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR));
    error = dlerror();
    if (!error) {
        AUDIO_ERR_LOG("<log error> dlsym failed: error: %{public}s, %{public}p", error, description);
    } else {
        AUDIO_INFO_LOG("<log info> dlsym lib successful");
    }

    libEntry->handle = handle;
    libEntry->desc = description;

    return true;
}

void loadLibraries(const std::vector<Library> &libs,
                   std::vector<std::unique_ptr<libEntryT>> &libList,
                   std::vector<std::unique_ptr<libEntryT>> &glibFailedList)
{
    for (Library library: libs) {
        AUDIO_INFO_LOG("<log info> loading %{public}s : %{public}s", library.name.c_str(), library.path.c_str());

        std::unique_ptr<libEntryT> libEntry = std::make_unique<libEntryT>();
        libEntry->name = strdup(library.name.c_str());

        bool loadLibrarySuccess = loadLibrary(library.path.c_str(), libEntry);
        if (!loadLibrarySuccess) {
            // Register library load failure
            glibFailedList.emplace_back(std::move(libEntry));
            continue;
        }

        // Register library load success
        libList.emplace_back(std::move(libEntry));
    }
}

libEntryT *findLibrary(const std::string name, std::vector<std::unique_ptr<libEntryT>> &glibList)
{
    for (const std::unique_ptr<libEntryT>& lib : glibList) {
        if (lib->name == name) {
            return lib.get();
        }
    }

    return nullptr;
}

LoadEffectResult loadEffect(const Effect &effect, const std::string &name,
                            std::vector<std::unique_ptr<libEntryT>> &glibList)
{
    LoadEffectResult result;

    result.lib = findLibrary(effect.libraryName, glibList);
    if (result.lib == nullptr) {
        AUDIO_ERR_LOG("<log error> could not find library %{public}s to load effect %{public}s",
                      effect.libraryName.c_str(), effect.name.c_str());
        result.success = false;
        return result;
    }

    result.effectDesc = std::make_unique<effectDescriptorT>();

    result.success = true;
    return result;
}

void loadEffects(const std::vector<Effect> &effects,
                 std::vector<std::unique_ptr<libEntryT>> &glibList,
                 std::vector<std::unique_ptr<effectDescriptorT>> &gSkippedEffectList,
                 std::vector<Effect> &successEffectList)
{
    for (Effect effect: effects) {
        std::cout << effect.name << std::endl;
        LoadEffectResult effectLoadResult = loadEffect(effect, effect.name, glibList);
        if (!effectLoadResult.success) {
            if (effectLoadResult.effectDesc != nullptr) {
                gSkippedEffectList.emplace_back(std::move(effectLoadResult.effectDesc));
            }
            continue;
        }

        effectLoadResult.lib->effects.emplace_back(std::move(effectLoadResult.effectDesc));
        successEffectList.push_back(effect);
    }
}

AudioEffectServer::AudioEffectServer() // 构造函数
{
    AUDIO_INFO_LOG("AudioEffectServer ctor");
}

AudioEffectServer::~AudioEffectServer()  // 析构函数
{
}

bool AudioEffectServer::LoadAudioEffects(std::vector<Library> libraries, std::vector<Effect> effects,
                                         std::vector<Effect>& successEffectList)
{
    // load library
    AUDIO_INFO_LOG("<log info> load library");
    loadLibraries(libraries, effectLibraryList, effectLibraryFailedList);

    // load effect
    AUDIO_INFO_LOG("<log info> load effect");
    loadEffects(effects, effectLibraryList, effectSkippedEffects, successEffectList);
    if (successEffectList.size()>0) {
        return true;
    } else {
        return false;
    }
}

std::vector<std::unique_ptr<libEntryT>>& AudioEffectServer::GetAvailableEffects()
{
    return effectLibraryList;
}

} // namespce AudioStandard
} // namespace OHOS