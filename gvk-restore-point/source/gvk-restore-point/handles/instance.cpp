
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
#include "gvk-layer/registry.hpp"

namespace gvk {
namespace restore_point {

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

VkResult Applier::restore_VkInstance(const GvkRestorePointObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo)
{
    (void)restorePointObject;
    (void)restoreInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkInstance_state(const GvkRestorePointObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo)
{
    (void)restorePointObject;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        const auto& dispatchTableItr = layer::Registry::get().VkInstanceDispatchTables.find(layer::get_dispatch_key(restoreInfo.handle));
        assert(dispatchTableItr != layer::Registry::get().VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& dispatchTable = dispatchTableItr->second;
        gvk_result(Instance::create_unmanaged(restoreInfo.pInstanceCreateInfo, nullptr, &dispatchTable, mApplyInfo.instance, &mInstance));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkInstance(const GvkRestorePointObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo)
{
    (void)restoreInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // TODO : Logic to force/filter/unfilter objects for restoration
        if (mApplyInfo.restoreInstance_HACK) {
            gvk_result(BasicApplier::process_VkInstance(restorePointObject, restoreInfo));
        } else {
            auto restoredObject = restorePointObject;
            restoredObject.handle = (uint64_t)mApplyInfo.instance;
            restoredObject.dispatchableHandle = (uint64_t)mApplyInfo.instance;
            gvk_result(register_restored_object(restorePointObject, restoredObject));
        }

        DispatchTable::load_global_entry_points(&mApplicationDispatchTable);
        DispatchTable::load_instance_entry_points(mApplyInfo.instance, &mApplicationDispatchTable);
        // TODO : Logic to select mApplicationDispatchTable vs layer DispatchTable
        //  Live apply needs to go down the layer dispatch table...recordable apply
        //  needs to hit the provided dispatch table...
        auto recorderEnumeratePhysicalDevices = mApplyInfo.dispatchTable.gvkEnumeratePhysicalDevices;

        gvk_result(Instance::create_unmanaged(restoreInfo.pInstanceCreateInfo, nullptr, &mApplicationDispatchTable, mApplyInfo.instance, &mInstance));
        uint32_t physicalDeviceCount = 0;
        gvk_result(mApplicationDispatchTable.gvkEnumeratePhysicalDevices(mApplyInfo.instance, &physicalDeviceCount, nullptr));
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        gvk_result(mApplicationDispatchTable.gvkEnumeratePhysicalDevices(mApplyInfo.instance, &physicalDeviceCount, physicalDevices.data()));

        ///////////////////////////////////////////////////////////////////////////////
        // TODO : Figure out why this is necessary...this call is triggering GPA FW's
        //  object mapping logic, but the RestorePointOperation created by the call to
        //  register_restored_object() _should_ be enough.
        gvk_result(recorderEnumeratePhysicalDevices(mApplyInfo.instance, &physicalDeviceCount, nullptr));
        gvk_result(recorderEnumeratePhysicalDevices(mApplyInfo.instance, &physicalDeviceCount, physicalDevices.data()));
        ///////////////////////////////////////////////////////////////////////////////

        for (auto physicalDevice : physicalDevices) {
            VkPhysicalDeviceProperties physicalDeviceProperties{ };
            mApplicationDispatchTable.gvkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
            mUnrestoredPhysicalDevices[physicalDeviceProperties].push_back(physicalDevice);
        }

        // TODO : Documentation
        if (restoreInfo.pInstanceCreateInfo->pApplicationInfo->apiVersion < VK_API_VERSION_1_2) {
            mApplyInfo.dispatchTable.gvkCreateRenderPass2 = mApplyInfo.dispatchTable.gvkCreateRenderPass2KHR;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk
