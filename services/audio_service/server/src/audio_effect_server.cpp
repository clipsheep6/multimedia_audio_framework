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
#include "audio_effect_server.h"
#include "audio_log.h"
#include "audio_effect_server.h"

# define AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR "AELI"
# define EFFECT_STRING_LEN_MAX 64

namespace OHOS {
namespace AudioStandard {

constexpr const char *LD_EFFECT_LIBRARY_PATH[] = {"/system/lib/"};

struct LoadEffectResult {
    bool success = false;
    LibEntryT *lib = nullptr;
    std::unique_ptr<EffectDescriptorT> effectDesc;
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

static bool LoadLibrary(const std::string relativePath, std::unique_ptr<LibEntryT>& libEntry) noexcept
{
    std::string absolutePath;
    // find library in adsolutePath
    if (!ResolveLibrary(relativePath, absolutePath)) {
        AUDIO_ERR_LOG("<log error> find library falied in effect directories: %{public}s",
            relativePath.c_str());
        libEntry->path = relativePath;
        return false;
    }

    // load hundle
    libEntry->path = absolutePath;

    void* handle = dlopen(absolutePath.c_str(), 1);
    if (!handle) {
        AUDIO_ERR_LOG("<log error> Open lib Fail");
        return false;
    } else {
        AUDIO_INFO_LOG("<log info> dlopen lib successful");
    }

    AudioEffectLibraryT *description = static_cast<AudioEffectLibraryT *>(dlsym(handle,
        AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR));
    const char* error = dlerror();
    if (!error) {
        AUDIO_ERR_LOG("<log error> dlsym failed: error: %{public}s, %{public}p", error, description);
    } else {
        AUDIO_INFO_LOG("<log info> dlsym lib successful");
    }

    libEntry->handle = handle;
    libEntry->desc = description;

    return true;
}

void LoadLibraries(const std::vector<Library> &libs,
                   std::vector<std::unique_ptr<LibEntryT>> &libList,
                   std::vector<std::unique_ptr<LibEntryT>> &glibFailedList)
{
    for (Library library: libs) {
        AUDIO_INFO_LOG("<log info> loading %{public}s : %{public}s", library.name.c_str(), library.path.c_str());

        std::unique_ptr<LibEntryT> libEntry = std::make_unique<LibEntryT>();
        libEntry->name = library.name;

        bool loadLibrarySuccess = LoadLibrary(library.path, libEntry);
        if (!loadLibrarySuccess) {
            // Register library load failure
            glibFailedList.emplace_back(std::move(libEntry));
            continue;
        }

        // Register library load success
        libList.emplace_back(std::move(libEntry));
    }
}

LibEntryT *FindLibrary(const std::string name, std::vector<std::unique_ptr<LibEntryT>> &libList)
{
    for (const std::unique_ptr<LibEntryT>& lib : libList) {
        if (lib->name == name) {
            return lib.get();
        }
    }

    return nullptr;
}

LoadEffectResult LoadEffect(const Effect &effect, const std::string &name,
                            std::vector<std::unique_ptr<LibEntryT>> &libList)
{
    LoadEffectResult result;

    result.lib = FindLibrary(effect.libraryName, libList);
    if (result.lib == nullptr) {
        AUDIO_ERR_LOG("<log error> could not find library %{public}s to load effect %{public}s",
                      effect.libraryName.c_str(), effect.name.c_str());
        result.success = false;
        return result;
    }

    result.effectDesc = std::make_unique<EffectDescriptorT>();

    result.success = true;
    return result;
}

void LoadEffects(const std::vector<Effect> &effects,
                 std::vector<std::unique_ptr<LibEntryT>> &libList,
                 std::vector<std::unique_ptr<EffectDescriptorT>> &gSkippedEffectList,
                 std::vector<Effect> &successEffectList)
{
    for (Effect effect: effects) {
        LoadEffectResult effectLoadResult = LoadEffect(effect, effect.name, libList);
        if (!effectLoadResult.success) {
            AUDIO_ERR_LOG("<log error> LoadEffects have failures!");
            if (effectLoadResult.effectDesc != nullptr) {
                gSkippedEffectList.emplace_back(std::move(effectLoadResult.effectDesc));
            }
            continue;
        }

        effectLoadResult.lib->effects.emplace_back(std::move(effectLoadResult.effectDesc));
        successEffectList.push_back(effect);
    }
}

AudioEffectServer::AudioEffectServer()
{
    AUDIO_INFO_LOG("AudioEffectServer ctor");
}

AudioEffectServer::~AudioEffectServer()
{
}

bool AudioEffectServer::LoadAudioEffects(const std::vector<Library> libraries, const std::vector<Effect> effects,
                                         std::vector<Effect>& successEffectList)
{
    // load library
    AUDIO_INFO_LOG("<log info> load library");
    LoadLibraries(libraries, effectLibraryList, effectLibraryFailedList);

    // load effect
    AUDIO_INFO_LOG("<log info> load effect");
    LoadEffects(effects, effectLibraryList, effectSkippedEffects, successEffectList);
    if (successEffectList.size()>0) {
        return true;
    } else {
        return false;
    }
}

std::vector<std::unique_ptr<LibEntryT>>& AudioEffectServer::GetAvailableEffects()
{
    return effectLibraryList;
}

} // namespce AudioStandard
} // namespace OHOS