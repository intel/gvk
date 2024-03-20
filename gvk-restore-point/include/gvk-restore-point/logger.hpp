
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

#pragma once

#include "gvk-defines.hpp"

#include <ostream>
#include <sstream>

namespace gvk {
namespace restore_point {

class Log final
{
public:
    enum Ctrl
    {
        Flush,
    };

    Log() = default;

    void set_instance(VkInstance vkInstance);

    template <typename T>
    friend Log& operator<<(Log& log, const T& obj);

private:
    VkInstance mVkInstance{ };
    PFN_vkSubmitDebugUtilsMessageEXT mPfnVkSubmitDebugUtilsMessage{ };
    VkDebugUtilsMessageSeverityFlagBitsEXT mSeverity{ VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT };
    VkDebugUtilsMessageTypeFlagBitsEXT mType{ VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT };
    std::stringstream mStrStrm;

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
};

template <typename T>
inline Log& operator<<(Log& log, const T& obj)
{
    log.mStrStrm << obj;
    return log;
}

template <> Log& operator<<<VkDebugUtilsMessageSeverityFlagBitsEXT>(Log& log, const VkDebugUtilsMessageSeverityFlagBitsEXT& severity);
template <> Log& operator<<<VkDebugUtilsMessageTypeFlagBitsEXT>(Log& log, const VkDebugUtilsMessageTypeFlagBitsEXT& type);
template <> Log& operator<<<Log::Ctrl>(Log& log, const Log::Ctrl& ctrl);

} // namespace restore_point
} // namespace gvk
