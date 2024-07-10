
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
#include "gvk-restore-point/layer.hpp"
#include "gvk-layer/registry.hpp"

namespace gvk {
namespace restore_point {

thread_local VkInstanceCreateInfo tlApplicationInstanceCreateInfo;
thread_local VkInstanceCreateInfo tlRestorePointInstanceCreateInfo;
VkResult Layer::pre_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult layerResult)
{
    (void)pAllocator;
    (void)pInstance;
    assert(pCreateInfo);
    tlApplicationInstanceCreateInfo = *pCreateInfo;
    if (layerResult == VK_SUCCESS) {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // TODO : Documentation
            static const auto layerNpos = std::numeric_limits<uint32_t>::max();
            auto restorePointLayerIndex = layerNpos;
            auto stateTrackerLayerIndex = layerNpos;
            assert(pCreateInfo->ppEnabledLayerNames);
            for (uint32_t i = 0; i < pCreateInfo->enabledLayerCount; ++i) {
                if (!strcmp(pCreateInfo->ppEnabledLayerNames[i], VK_LAYER_INTEL_GVK_STATE_TRACKER_NAME)) {
                    stateTrackerLayerIndex = i;
                } else
                if (!strcmp(pCreateInfo->ppEnabledLayerNames[i], VK_LAYER_INTEL_GVK_RESTORE_POINT_NAME)) {
                    restorePointLayerIndex = i;
                }
            }
            if (restorePointLayerIndex == layerNpos || stateTrackerLayerIndex == layerNpos || restorePointLayerIndex < stateTrackerLayerIndex) {
                gvk_result(VK_ERROR_LAYER_NOT_PRESENT);
            }

            // TODO : Documentation
            auto restorePointInstanceCreateInfo = tlApplicationInstanceCreateInfo;
            // TODO : Modify restorePointInstanceCreateInfo
            tlRestorePointInstanceCreateInfo = restorePointInstanceCreateInfo;
            *const_cast<VkInstanceCreateInfo*>(pCreateInfo) = tlRestorePointInstanceCreateInfo;
        } gvk_result_scope_end;
        layerResult = gvkResult;
    }
    return layerResult;
}

VkResult Layer::post_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult)
{
    (void)pAllocator;
    (void)pInstance;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        *const_cast<VkInstanceCreateInfo*>(pCreateInfo) = tlApplicationInstanceCreateInfo;
        gvkResult = state_tracker::load_layer_entry_points();
    }
    return gvkResult;
}

VkResult Creator::process_VkInstance(GvkInstanceRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        const auto& dispatchTableItr = layer::Registry::get().VkInstanceDispatchTables.find(layer::get_dispatch_key(restoreInfo.handle));
        assert(dispatchTableItr != layer::Registry::get().VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& dispatchTable = dispatchTableItr->second;
        gvk_result(Instance::create_unmanaged(restoreInfo.pInstanceCreateInfo, nullptr, &dispatchTable, restoreInfo.handle, &mInstance));

        // TODO : Documentation
        auto applicationInfo = get_default<VkApplicationInfo>();
        if (restoreInfo.pInstanceCreateInfo->pApplicationInfo) {
            applicationInfo = *restoreInfo.pInstanceCreateInfo->pApplicationInfo;
        } else {
            applicationInfo.apiVersion = layer::Registry::get().apiVersion;
            const_cast<VkInstanceCreateInfo*>(restoreInfo.pInstanceCreateInfo)->pApplicationInfo = &applicationInfo;
        }
        std::string apiVersionStr =
            std::to_string(VK_API_VERSION_VARIANT(applicationInfo.apiVersion)) + '.' +
            std::to_string(VK_API_VERSION_MAJOR(applicationInfo.apiVersion)) + '.' +
            std::to_string(VK_API_VERSION_MINOR(applicationInfo.apiVersion)) + '.' +
            std::to_string(VK_API_VERSION_PATCH(applicationInfo.apiVersion));
        restoreInfo.pApiVersion = apiVersionStr.c_str();

        // TODO : Documentation
        uint32_t physicalDeviceCount = 0;
        dispatchTable.gvkEnumeratePhysicalDevices(mCreateInfo.instance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        dispatchTable.gvkEnumeratePhysicalDevices(mCreateInfo.instance, &physicalDeviceCount, physicalDevices.data());
        restoreInfo.physicalDeviceCount = (uint32_t)physicalDevices.size();
        restoreInfo.pPhysicalDevices = !physicalDevices.empty() ? physicalDevices.data() : nullptr;

        // TODO : Documentation
        gvk_result(BasicCreator::process_VkInstance(restoreInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkInstance(const GvkStateTrackedObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // TODO : Documentation
        gvk_result(BasicApplier::restore_VkInstance(restorePointObject, restoreInfo));

        // TODO : Documentation
        uint32_t physicalDeviceCount = 0;
        gvk_result(mApplicationDispatchTable.gvkEnumeratePhysicalDevices(mApplyInfo.vkInstance, &physicalDeviceCount, nullptr));
        std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount);
        gvk_result(mApplicationDispatchTable.gvkEnumeratePhysicalDevices(mApplyInfo.vkInstance, &physicalDeviceCount, vkPhysicalDevices.data()));

        // TODO : Figure out why this is necessary...this call is triggering GPA FW's
        //  object mapping logic, but the RestorePointOperation created by the call to
        //  register_restored_object() _should_ be enough.
        const auto& dispatchTable = mApplyInfo.dispatchTable;
        gvk_result(dispatchTable.gvkEnumeratePhysicalDevices(mApplyInfo.vkInstance, &physicalDeviceCount, nullptr));
        gvk_result(dispatchTable.gvkEnumeratePhysicalDevices(mApplyInfo.vkInstance, &physicalDeviceCount, vkPhysicalDevices.data()));

        // TODO : Documentation
        for (auto vkPhysicalDevice : vkPhysicalDevices) {
            VkPhysicalDeviceProperties physicalDeviceProperties{ };
            mApplicationDispatchTable.gvkGetPhysicalDeviceProperties(vkPhysicalDevice, &physicalDeviceProperties);
            mUnrestoredPhysicalDevices[physicalDeviceProperties].push_back(vkPhysicalDevice);
        }

        // TODO : Documentation
        if (restoreInfo.pInstanceCreateInfo->pApplicationInfo->apiVersion < VK_API_VERSION_1_2) {
            mApplyInfo.dispatchTable.gvkCreateRenderPass2 = mApplyInfo.dispatchTable.gvkCreateRenderPass2KHR;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkInstance_state(const GvkStateTrackedObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo)
{
    (void)restorePointObject;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        const auto& layerDispatchTable = layer::Registry::get().get_instance_dispatch_table(restoreInfo.handle);
#if 0
        auto dispatchTable = mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT ? layerDispatchTable : mApplicationDispatchTable;
#else
        auto dispatchTable = layerDispatchTable;
#endif
        gvk_result(Instance::create_unmanaged(restoreInfo.pInstanceCreateInfo, nullptr, &dispatchTable, mApplyInfo.vkInstance, &mInstance));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk
