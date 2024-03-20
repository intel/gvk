
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
#include "gvk-command-structures/generated/execute-command-structure.hpp"
#include "gvk-layer/registry.hpp"
#include "gvk-restore-point/generated/update-structure-handles.hpp"

namespace gvk {
namespace restore_point {

VkResult Creator::process_VkSwapchainKHR(GvkSwapchainRestoreInfoKHR& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        Device device = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        assert(device);
        gvk_result(device.get<DispatchTable>().gvkGetSwapchainImagesKHR(device, restoreInfo.handle, &restoreInfo.imageCount, nullptr));
        std::vector<VkImage> images(restoreInfo.imageCount);
        gvk_result(device.get<DispatchTable>().gvkGetSwapchainImagesKHR(device, restoreInfo.handle, &restoreInfo.imageCount, images.data()));
        std::vector<GvkSwapchainImageRestoreInfo> swapchainImageRestoreInfos(restoreInfo.imageCount);
        for (uint32_t i = 0; i < restoreInfo.imageCount; ++i) {
            swapchainImageRestoreInfos[i].image = images[i];
            swapchainImageRestoreInfos[i].acquired = VK_FALSE;        // TODO :
            swapchainImageRestoreInfos[i].fence = VK_NULL_HANDLE;     // TODO :
            swapchainImageRestoreInfos[i].semaphore = VK_NULL_HANDLE; // TODO :
        }
        restoreInfo.pImages = swapchainImageRestoreInfos.data();
        gvk_result(BasicCreator::process_VkSwapchainKHR(restoreInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkSwapchainKHR(const GvkRestorePointObject& restorePointObject, const GvkSwapchainRestoreInfoKHR& restoreInfo)
{
    (void)restorePointObject;
    (void)restoreInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkSwapchainKHR(const GvkRestorePointObject& restorePointObject, const GvkSwapchainRestoreInfoKHR& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (restoreInfo.pSwapchainCreateInfoKHR) {
            auto surface = restoreInfo.pSwapchainCreateInfoKHR->surface;

            VkSurfaceCapabilitiesKHR surfaceCapabilities{ };
            gvk_result(mApplyInfo.dispatchTable.gvkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                (VkPhysicalDevice)get_restored_object(get_restore_point_object_dependency<VkPhysicalDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies)).handle,
                (VkSurfaceKHR)get_restored_object(get_restore_point_object_dependency<VkSurfaceKHR>(restoreInfo.dependencyCount, restoreInfo.pDependencies)).handle,
                &surfaceCapabilities
            ));

            // TODO : Are there situations where the oldSwapchain is necessary for correct
            //  restoration?
            // FROM : https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSwapchainCreateInfoKHR.html
            //  "allows the application to still present any images that are already acquired from it"
            //  If the restore point is created while oldSwapchain has outstanding image
            //  acquisitions that will be presented, it may be necessary...
            // TODO : State tracker needs updating to manage the dependency on oldSwapchain
            const_cast<VkSwapchainCreateInfoKHR*>(restoreInfo.pSwapchainCreateInfoKHR)->oldSwapchain = VK_NULL_HANDLE;


            auto commandStructure = get_default<GvkCommandStructureCreateSwapchainKHR>();
            commandStructure.device = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
            commandStructure.pCreateInfo = restoreInfo.pSwapchainCreateInfoKHR;
            const_cast<VkSwapchainCreateInfoKHR*>(commandStructure.pCreateInfo)->surface = VK_NULL_HANDLE;
            update_command_structure_handles(mRestorePointObjects, commandStructure);
            auto surfaceRestorePointObject = restorePointObject;
            surfaceRestorePointObject.type = VK_OBJECT_TYPE_SURFACE_KHR;
            surfaceRestorePointObject.handle = (uint64_t)surface;
            surfaceRestorePointObject.dispatchableHandle = (uint64_t)get_dependency<VkInstance>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
            const_cast<VkSwapchainCreateInfoKHR*>(commandStructure.pCreateInfo)->surface = (VkSurfaceKHR)get_restored_object(surfaceRestorePointObject).handle;;
            VkSwapchainKHR handle = restoreInfo.handle;
            commandStructure.pSwapchain = &handle;
            gvk_result(process_GvkCommandStructureCreateSwapchainKHR(restorePointObject, restoreInfo, commandStructure));
            gvk_result(detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure));
            auto restoredObject = restorePointObject;
            restoredObject.handle = (uint64_t)handle;
            restoredObject.dispatchableHandle = (uint64_t)commandStructure.device;
            gvk_result(register_restored_object(restorePointObject, restoredObject));
            auto device = (VkDevice)get_restored_object(get_restore_point_object_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies)).handle;
            auto swapchain = (VkSwapchainKHR)get_restored_object(restorePointObject).handle;
            uint32_t swapchainImageCount = 0;

            auto layerDispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(device));
            gvk_result(layerDispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            const auto& layerDispatchTable = layerDispatchTableItr->second;
            gvk_result(layerDispatchTable.gvkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr));

            gvk_result(restoreInfo.imageCount == swapchainImageCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            std::vector<VkImage> swapchainImages(swapchainImageCount);
            gvk_result(layerDispatchTable.gvkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data()));

            ///////////////////////////////////////////////////////////////////////////////
            // TODO : Figure out why this is necessary...this call is triggering GPA FW's
            //  object mapping logic, but the RestorePointOperation created by the call to
            //  register_restored_object() _should_ be enough.
            gvk_result(mApplyInfo.dispatchTable.gvkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr));
            gvk_result(mApplyInfo.dispatchTable.gvkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data()));
            ///////////////////////////////////////////////////////////////////////////////

            for (uint32_t i = 0; i < swapchainImageCount; ++i) {
                auto swapchainImageRestorePointObject = restorePointObject;
                swapchainImageRestorePointObject.type = VK_OBJECT_TYPE_IMAGE;
                swapchainImageRestorePointObject.handle = (uint64_t)restoreInfo.pImages[i].image;
                restoredObject = swapchainImageRestorePointObject;
                restoredObject.handle = (uint64_t)swapchainImages[i];
                gvk_result(register_restored_object(swapchainImageRestorePointObject, restoredObject));
            }
            // Acquire image(s)
            // Reset semaphores
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkSwapchainKHR_state(const GvkRestorePointObject& restorePointObject, const GvkSwapchainRestoreInfoKHR& restoreInfo)
{
    (void)restorePointObject;
    (void)restoreInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

void Applier::destroy_VkSwapchainKHR(const GvkRestorePointObject& restorePointObject)
{
    auto commandStructure = get_default<GvkCommandStructureDestroySwapchainKHR>();
    commandStructure.device = (VkDevice)restorePointObject.dispatchableHandle;
    commandStructure.swapchain = (VkSwapchainKHR)restorePointObject.handle;
    detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure);
}

} // namespace restore_point
} // namespace gvk
