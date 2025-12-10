
/*******************************************************************************

MIT License

Copyright (c) Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#include "gvk-environment.hpp"
#include "gvk-string/utilities.hpp"

#include <cstring>
#include <filesystem>
#include <unordered_set>

namespace gvk {

void Environment::reset()
{
    mEnvVars.clear();
}

void Environment::set_env()
{
    reset();
#ifdef GVK_PLATFORM_WINDOWS
    auto freeEnvStrs = [](char* p) { FreeEnvironmentStrings(p); };
    auto env = std::unique_ptr<char, decltype(freeEnvStrs)>{ GetEnvironmentStrings(), freeEnvStrs };
    if (env) {
        for (auto i = env.get(); *i != '\0'; ++i) {
            std::string key;
            std::string value;
            for (; *i != '='; ++i) {
                key += *i;
            }
            ++i;
            for (; *i != '\0'; ++i) {
                value += *i;
            }
            mEnvVars[key] = value;
        }
    }
#else
    // TODO :
#endif
}

void Environment::get_env(uint32_t* pCount, char* pEnv) const
{
    if (pCount) {
        if (pEnv) {
            auto pWrite = pEnv;
            auto pEnd = pEnv + *pCount;
            memset(pEnv, '\0', *pCount);
            for (const auto& envVar : mEnvVars) {
                std::string kvp = envVar.first.string() + "=" + envVar.second.string();
                if (pWrite + kvp.size() + 1 /* '\0' */ + 1 /* '\0' */ < pEnd) {
                    memcpy(pWrite, kvp.c_str(), kvp.size());
                    pWrite += kvp.size() + 1;
                } else {
                    // TODO : Report incomplete
                }
            }
        } else {
            *pCount = 0;
            for (const auto& envVar : mEnvVars) {
                *pCount += (uint32_t)envVar.first.string().size() + 1 /* '=' */ + (uint32_t)envVar.second.string().size() + 1 /* '\0' */;
            }
            *pCount += 1 /* '\0' */;
            *pCount += 1 /* '\0' */;
        }
    }
}

std::string Environment::get_env_var(const std::string& key) const
{
    auto itr = !key.empty() ? mEnvVars.find(key) : mEnvVars.end();
    return itr != mEnvVars.end() ? itr->second.string() : std::string();
}

void Environment::set_env_var(const std::string& key, const std::string& value)
{
    if (!key.empty()) {
        if (value.empty()) {
            mEnvVars.erase(key);
        } else {
            mEnvVars[key] = value;
        }
    }
}

void Environment::append_value_to_env_var(const std::string& key, const std::string& value)
{
    if (!key.empty() && !value.empty()) {
        auto itr = !key.empty() ? mEnvVars.find(key) : mEnvVars.end();
        if (itr != mEnvVars.end()) {
#ifdef GVK_PLATFORM_WINDOWS
            std::string delimiter = ";";
#else
            std::string delimiter = ":";
#endif
            mEnvVars[key] = itr->second.string() + delimiter + value;
        }
    }
}

std::string get_env_var(const std::string& key)
{
    std::string value;
    if (!key.empty()) {
#ifdef GVK_PLATFORM_WINDOWS
        char* pValue = nullptr;
        size_t size = 0;
        auto error = _dupenv_s(&pValue, &size, key.c_str());
        value = (!error && pValue) ? pValue : std::string();
        free(pValue);
#else
        auto pValue = getenv(key.c_str());
        value = pValue ? pValue : std::string();
#endif
    }
    return value;
}

bool get_env_var_true(const std::string& key)
{
    auto value = gvk::string::to_lower(get_env_var(key));
    return value == "true" || value == "yes" || value == "on" || value == "y" || gvk::string::to_number<int>(value);
}

void set_env_var(const std::string& key, const std::string& value)
{
    if (!key.empty()) {
#ifdef GVK_PLATFORM_WINDOWS
        _putenv_s(key.c_str(), value.c_str());
#else
        if (!value.empty()) {
            setenv(key.c_str(), value.c_str(), 1);
        } else {
            unsetenv(key.c_str());
        }
#endif
    }
}

void append_value_to_env_var(const std::string& key, const std::string& value)
{
#ifdef GVK_PLATFORM_WINDOWS
    std::string delimiter = ";";
#else
    std::string delimiter = ":";
#endif
    auto currentValue = get_env_var(key);
    auto currentValues = string::split(currentValue, delimiter);
    if (std::find(currentValues.begin(), currentValues.end(), value) == currentValues.end()) {
        set_env_var(key, !currentValue.empty() ? currentValue + delimiter + value : value);
    }
}

#ifdef GVK_PLATFORM_WINDOWS
void set_vk_layer_path_from_windows_registry()
{
    if (get_env_var("VK_LAYER_PATH").empty()) {
        HKEY hKey = NULL;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Khronos\\Vulkan\\ExplicitLayers", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD cbMaxValueNameLen = 0;
            if (RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &cbMaxValueNameLen, NULL, NULL, NULL) == ERROR_SUCCESS) {
                std::unordered_set<std::string> explicitLayerPaths;
                std::string valueName(++cbMaxValueNameLen, '\0');
                for (DWORD i = 0;; ++i) {
                    DWORD cchValueName = cbMaxValueNameLen;
                    if (RegEnumValue(hKey, i, (char*)valueName.data(), &cchValueName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                        explicitLayerPaths.insert(std::filesystem::path(valueName).remove_filename().string());
                    } else {
                        break;
                    }
                }
                for (const auto& explicitLayerPath : explicitLayerPaths) {
                    append_value_to_env_var("VK_LAYER_PATH", explicitLayerPath);
                }
            }
        }
        RegCloseKey(hKey);
    }
}
#endif

} // namespace gvk
