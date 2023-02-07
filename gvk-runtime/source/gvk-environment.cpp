
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

#include <filesystem>
#include <unordered_set>

namespace gvk {

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

void set_env_var(const std::string& key, const std::string& value)
{
    if (!key.empty() && !value.empty()) {
#ifdef GVK_PLATFORM_WINDOWS
        _putenv_s(key.c_str(), value.c_str());
#else
        setenv(key.c_str(), value.c_str(), 1);
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
        set_env_var(key, currentValue + delimiter + value);
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
