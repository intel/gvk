
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

#include "gvk-restore-point/logger.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-structures/defaults.hpp"

namespace gvk {
namespace restore_point {

void Log::set_instance(VkInstance vkInstance)
{
    mVkInstance = vkInstance;
    DispatchTable dispatchTable{ };
    if (mVkInstance) {
        DispatchTable::load_global_entry_points(&dispatchTable);
        DispatchTable::load_instance_entry_points(mVkInstance, &dispatchTable);
    }
    mPfnVkSubmitDebugUtilsMessage = dispatchTable.gvkSubmitDebugUtilsMessageEXT;
}

template <> Log& operator<<<VkDebugUtilsMessageSeverityFlagBitsEXT>(Log& log, const VkDebugUtilsMessageSeverityFlagBitsEXT& severity)
{
    log.mSeverity = severity ? severity : VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    return log;
}

template <> Log& operator<<<VkDebugUtilsMessageTypeFlagBitsEXT>(Log& log, const VkDebugUtilsMessageTypeFlagBitsEXT& type)
{
    log.mType = type ? type : VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    return log;
}

template <> Log& operator<<<Log::Ctrl>(Log& log, const Log::Ctrl& ctrl)
{
    switch (ctrl) {
    case Log::Flush: {
        log.mStrStrm.flush();
        if (log.mPfnVkSubmitDebugUtilsMessage) {
            assert(log.mVkInstance);
            auto message = log.mStrStrm.str();
            auto debugUtilsMessengerCallbackData = get_default<VkDebugUtilsMessengerCallbackDataEXT>();
            debugUtilsMessengerCallbackData.pMessage = message.c_str();
            log.mPfnVkSubmitDebugUtilsMessage(log.mVkInstance, log.mSeverity, log.mType, &debugUtilsMessengerCallbackData);
        }
        log.mSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        log.mType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        log.mStrStrm.str(std::string());
    } break;
    default: {
        assert(false);
    } break;
    }
    return log;
}

} // namespace restore_point
} // namespace gvk
