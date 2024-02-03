
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

#include "gvk-restore-point/applier.hpp"
#include "gvk-restore-point/creator.hpp"

namespace gvk {
namespace restore_point {

VkResult Creator::process_VkSurfaceKHR(GvkSurfaceRestoreInfoKHR& restoreInfo)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto& win32SurfacetCreateInfo = restoreInfo.pWin32SurfaceCreateInfoKHR ? *restoreInfo.pWin32SurfaceCreateInfoKHR : VkWin32SurfaceCreateInfoKHR { };
    if (win32SurfacetCreateInfo.sType == get_stype<VkWin32SurfaceCreateInfoKHR>()) {
        RECT rect { };
        auto success = GetClientRect(win32SurfacetCreateInfo.hwnd, &rect);
        if (success) {
            restoreInfo.width = rect.right - rect.left;
            restoreInfo.height = rect.bottom - rect.top;
        }
    }
#endif
    return BasicCreator::process_VkSurfaceKHR(restoreInfo);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult Applier::process_GvkCommandStructureCreateWin32SurfaceKHR(const GvkRestorePointObject& restorePointObject, const GvkSurfaceRestoreInfoKHR& restoreInfo, GvkCommandStructureCreateWin32SurfaceKHR& commandStructure)
{
    (void)restorePointObject;
    if (mApplyInfo.pfnProcessWin32SurfaceCreateInfoCallback) {
        mApplyInfo.pfnProcessWin32SurfaceCreateInfoCallback(restoreInfo.width, restoreInfo.height, const_cast<VkWin32SurfaceCreateInfoKHR*>(commandStructure.pCreateInfo));
    }
    return VK_SUCCESS;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

} // namespace restore_point
} // namespace gvk
