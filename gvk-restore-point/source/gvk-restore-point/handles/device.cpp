
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

VkResult Creator::process_VkDevice(GvkDeviceRestoreInfo& restoreInfo)
{
    assert(restoreInfo.pDeviceCreateInfo);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto instance = get_dependency<VkInstance>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        auto physicalDevice = get_dependency<VkPhysicalDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        VkPhysicalDevice stateTrackerPhysicalDevice = VK_NULL_HANDLE;
        gvkGetStateTrackerPhysicalDevice(instance, physicalDevice, &stateTrackerPhysicalDevice);
        const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(restoreInfo.handle));
        assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& dispatchTable = dispatchTableItr->second;
        gvk_result(dispatchTable.gvkDeviceWaitIdle(restoreInfo.handle));
        Device gvkDevice;
        gvk_result(Device::create_unmanaged(stateTrackerPhysicalDevice, restoreInfo.pDeviceCreateInfo, nullptr, &dispatchTable, restoreInfo.handle, &gvkDevice));
        gvk_result(mDevices.insert(gvkDevice).second ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        std::vector<VkQueue> queues;
        for (uint32_t queueCreateInfo_i = 0; queueCreateInfo_i < restoreInfo.pDeviceCreateInfo->queueCreateInfoCount; ++queueCreateInfo_i) {
            const auto& queueCreateInfo = restoreInfo.pDeviceCreateInfo->pQueueCreateInfos[queueCreateInfo_i];
            queues.reserve(queues.size() + queueCreateInfo.queueCount);
            for (uint32_t queue_i = 0; queue_i < queueCreateInfo.queueCount; ++queue_i) {
                VkQueue queue = VK_NULL_HANDLE;
                dispatchTable.gvkGetDeviceQueue((VkDevice)restoreInfo.handle, queueCreateInfo.queueFamilyIndex, queue_i, &queue);
                mDeviceQueueCreateInfos[queue] = queueCreateInfo;
                queues.push_back(queue);
            }
        }
        restoreInfo.queueCount = (uint32_t)queues.size();
        restoreInfo.pQueues = queues.data();
        auto& copyEngine = mCopyEngines[restoreInfo.handle];
        assert(!copyEngine);
        auto copyEngineCreateInfo = get_default<CopyEngine::CreateInfo>();
        copyEngineCreateInfo.threadCount = mCreateInfo.threadCount;
        copyEngineCreateInfo.pfnInitializeThreadCallback = mCreateInfo.pfnInitializeThreadCallback;
        gvk_result(CopyEngine::create(restoreInfo.handle, &copyEngineCreateInfo, &copyEngine));
        gvk_result(BasicCreator::process_VkDevice(restoreInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkDevice(const GvkRestorePointObject& restorePointObject, const GvkDeviceRestoreInfo& restoreInfo)
{
    (void)restorePointObject;
    (void)restoreInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkDevice_state(const GvkRestorePointObject& restorePointObject, const GvkDeviceRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(restoreInfo.handle));
        assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& dispatchTable = dispatchTableItr->second;

        auto vkInstance = get_dependency<VkInstance>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        auto vkPhysicalDevice = get_dependency<VkPhysicalDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        VkPhysicalDevice stateTrackerPhysicalDevice = VK_NULL_HANDLE;
        gvkGetStateTrackerPhysicalDevice(vkInstance, vkPhysicalDevice, &stateTrackerPhysicalDevice);

        auto vkDevice = (VkDevice)get_restored_object(restorePointObject).handle;
        Device gvkDevice;
        gvk_result(Device::create_unmanaged(stateTrackerPhysicalDevice, restoreInfo.pDeviceCreateInfo, nullptr, &dispatchTable, vkDevice, &gvkDevice));
        mDevices.insert(gvkDevice);

        // TODO : Documentation
        auto& copyEngine = mCopyEngines[vkDevice];
        assert(!copyEngine);
        auto copyEngineCreateInfo = get_default<CopyEngine::CreateInfo>();
        copyEngineCreateInfo.threadCount = mApplyInfo.threadCount;
        copyEngineCreateInfo.pfnInitializeThreadCallback = mApplyInfo.pfnInitializeThreadCallback;
        gvk_result(CopyEngine::create(gvkDevice, &copyEngineCreateInfo, &copyEngine));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkDevice(const GvkRestorePointObject& restorePointObject, const GvkDeviceRestoreInfo& restoreInfo)
{
    // auto recorderGetDeviceQueue = mApplyInfo.dispatchTable.gvkGetDeviceQueue;
    // (void)recorderGetDeviceQueue;
    // dispatchTable.gvkGetDeviceQueue = mApplicationDispatchTable.gvkGetDeviceQueue;
    gvk_result_scope_begin(VK_SUCCESS) {
        auto physicalDevice = (VkPhysicalDevice)get_restored_object(get_restore_point_object_dependency<VkPhysicalDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies)).handle;
        uint32_t queueFamilyProptyCount = 0;
        mApplyInfo.dispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyProptyCount, nullptr);
        gvk_result(BasicApplier::process_VkDevice(restorePointObject, restoreInfo));
        auto device = (VkDevice)get_restored_object(restorePointObject).handle;
        // TODO : Logic to select mApplicationDispatchTable vs layer DispatchTable

        auto dispatchTable = mApplyInfo.dispatchTable;
        dispatchTable.gvkGetDeviceQueue = mApplicationDispatchTable.gvkGetDeviceQueue;
        Device gvkDevice;
        gvk_result(Device::create_unmanaged(physicalDevice, restoreInfo.pDeviceCreateInfo, nullptr, &dispatchTable, device, &gvkDevice));

        uint32_t capturedQueue_i = 0;
        for (uint32_t queueCreateInfo_i = 0; queueCreateInfo_i < restoreInfo.pDeviceCreateInfo->queueCreateInfoCount; ++queueCreateInfo_i) {
            const auto& deviceQueueCreateInfo = restoreInfo.pDeviceCreateInfo->pQueueCreateInfos[queueCreateInfo_i];
            for (uint32_t restoredQueue_i = 0; restoredQueue_i < deviceQueueCreateInfo.queueCount; ++restoredQueue_i) {
                auto capturedQueue = restoreInfo.pQueues[capturedQueue_i++];
                VkQueue restoredQueue = VK_NULL_HANDLE;
                mApplicationDispatchTable.gvkGetDeviceQueue(device, deviceQueueCreateInfo.queueFamilyIndex, restoredQueue_i, &restoredQueue);
                mApplyInfo.dispatchTable.gvkGetDeviceQueue(device, deviceQueueCreateInfo.queueFamilyIndex, restoredQueue_i, &restoredQueue);
                // ///////////////////////////////////////////////////////////////////////////////
                // ^
                // // TODO : Figure out why this is necessary...this call is triggering GPA FW's
                // //  object mapping logic, but the RestorePointOperation created by the call to
                // //  register_restored_object() _should_ be enough.
                // ///////////////////////////////////////////////////////////////////////////////

                GvkRestorePointObject capturedQueueRestorePointObject{ };
                capturedQueueRestorePointObject.type = VK_OBJECT_TYPE_QUEUE;
                capturedQueueRestorePointObject.handle = (uint64_t)capturedQueue;
                capturedQueueRestorePointObject.dispatchableHandle = (uint64_t)capturedQueue;
                auto restoredQueueRestorePointObject = capturedQueueRestorePointObject;
                restoredQueueRestorePointObject.handle = (uint64_t)restoredQueue;
                restoredQueueRestorePointObject.dispatchableHandle = (uint64_t)restoredQueue;
                gvk_result(register_restored_object(capturedQueueRestorePointObject, restoredQueueRestorePointObject));
            }
        }

        // TODO : Documentation
        auto& copyEngine = mCopyEngines[device];
        assert(!copyEngine);
        auto copyEngineCreateInfo = get_default<CopyEngine::CreateInfo>();
        copyEngineCreateInfo.threadCount = mApplyInfo.threadCount;
        copyEngineCreateInfo.pfnInitializeThreadCallback = mApplyInfo.pfnInitializeThreadCallback;
        gvk_result(CopyEngine::create(gvkDevice, &copyEngineCreateInfo, &copyEngine));

        // TODO : Documentation
        auto& commandPool = mCommandPools[device];
        assert(!commandPool);
        auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        commandPoolCreateInfo.queueFamilyIndex = gvkDevice.get<QueueFamilies>()[0].index;
        gvk_result(CommandPool::create(gvkDevice, &commandPoolCreateInfo, nullptr, &commandPool));
        auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
#if 0
        gvk_result(CommandBuffer::allocate(gvkDevice, &commandBufferAllocateInfo, &commandBuffer));
#else
        gvk_result(gvkDevice.get<DispatchTable>().gvkAllocateCommandBuffers(gvkDevice, &commandBufferAllocateInfo, &mVkCommandBuffers[device]));
        // HACK : TODO : Documentation
        const auto& layerDispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(restoreInfo.handle));
        assert(layerDispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& layerDisatchTable = layerDispatchTableItr->second;
        if (gvkDevice.get<DispatchTable>().gvkAllocateCommandBuffers == mApplicationDispatchTable.gvkAllocateCommandBuffers ||
            gvkDevice.get<DispatchTable>().gvkAllocateCommandBuffers == layerDisatchTable.gvkAllocateCommandBuffers) {
            *(void**)mVkCommandBuffers[device] = *(void**)gvkDevice.get<VkDevice>();
        }
#endif

        // TODO : Documentation
        gvk_result(Fence::create(gvkDevice, &get_default<VkFenceCreateInfo>(), nullptr, &mFences[device]));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk
