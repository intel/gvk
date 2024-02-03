
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

#include "gvk-state-tracker/state-tracker.hpp"
#include "gvk-structures/pnext.hpp"

namespace gvk {
namespace state_tracker {

// NOTE : The following entry points reference VkSemaphore
////////////////////////////////////////////////////////////////////////////////
// vkCreateSemaphore
// vkImportSemaphoreFdKHR
// vkImportSemaphoreWin32HandleKHR
// vkImportSemaphoreZirconHandleFUCHSIA
// vkSignalSemaphore
// vkSignalSemaphoreKHR
// vkWaitSemaphores
// vkWaitSemaphoresKHR
////////////////////////////////////////////////////////////////////////////////
// NOOP : No state modification
// vkDestroySemaphore
// vkGetSemaphoreCounterValue
// vkGetSemaphoreCounterValueKHR
// vkGetSemaphoreFdKHR
// vkGetSemaphoreWin32HandleKHR
// vkGetSemaphoreZirconHandleFUCHSIA
////////////////////////////////////////////////////////////////////////////////
// Handled in /source/gvk-state-tracker/queue.cpp
// vkQueueBindSparse
// vkQueueSubmit
// vkQueuePresentKHR
////////////////////////////////////////////////////////////////////////////////
// Handled in /source/gvk-state-tracker/swapchain.cpp
// vkAcquireNextImage2KHR
// vkAcquireNextImageKHR
// vkLatencySleepNV

VkResult StateTracker::post_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore, VkResult gvkResult)
{
    return BasicStateTracker::post_vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, gvkResult);
}

VkResult StateTracker::post_vkImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo, VkResult gvkResult)
{
    (void)device;
    (void)pImportSemaphoreFdInfo;
    (void)gvkResult;
    assert(false && "VK_LAYER_INTEL_gvk_state_tracker vkImportSemaphoreFdKHR() unserviced; gvk maintenance required");
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult StateTracker::post_vkImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, VkResult gvkResult)
{
    (void)device;
    (void)pImportSemaphoreWin32HandleInfo;
    (void)gvkResult;
    assert(false && "VK_LAYER_INTEL_gvk_state_tracker vkImportSemaphoreWin32HandleKHR() unserviced; gvk maintenance required");
    return VK_ERROR_FEATURE_NOT_PRESENT;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_FUCHSIA
VkResult StateTracker::post_vkImportSemaphoreZirconHandleFUCHSIA(VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo, VkResult gvkResult)
{
    (void)device;
    (void)pImportSemaphoreZirconHandleInfo;
    (void)gvkResult;
    assert(false && "VK_LAYER_INTEL_gvk_state_tracker vkImportSemaphoreZirconHandleFUCHSIA() unserviced; gvk maintenance required");
    return VK_ERROR_FEATURE_NOT_PRESENT;
}
#endif // VK_USE_PLATFORM_FUCHSIA

VkResult StateTracker::post_vkSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult gvkResult)
{
    return BasicStateTracker::post_vkSignalSemaphore(device, pSignalInfo, gvkResult);
}

VkResult StateTracker::post_vkSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult gvkResult)
{
    return BasicStateTracker::post_vkSignalSemaphoreKHR(device, pSignalInfo, gvkResult);
}

VkResult StateTracker::post_vkWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult gvkResult)
{
    return BasicStateTracker::post_vkWaitSemaphores(device, pWaitInfo, timeout, gvkResult);
}

VkResult StateTracker::post_vkWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult gvkResult)
{
    return post_vkWaitSemaphores(device, pWaitInfo, timeout, gvkResult);
}

} // namespace state_tracker
} // namespace gvk
