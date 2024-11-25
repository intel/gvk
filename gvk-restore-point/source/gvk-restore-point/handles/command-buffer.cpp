
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
#include "gvk-restore-point/generated/update-structure-handles.hpp"
#include "gvk-command-structures/generated/command-structure-enumerate-handles.hpp"
#include "gvk-command-structures/generated/execute-command-structure.hpp"

namespace gvk {
namespace restore_point {

void Layer::pre_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    auto stateTrackedCommandPool = get_default<GvkStateTrackedObject>();
    stateTrackedCommandPool.type = VK_OBJECT_TYPE_COMMAND_POOL;
    stateTrackedCommandPool.handle = (uint64_t)commandPool;
    stateTrackedCommandPool.dispatchableHandle = (uint64_t)device;
    GvkStateTrackedObjectEnumerateInfo enumerateInfo{ };
    enumerateInfo.pfnCallback = [](const GvkStateTrackedObject* pStateTrackedCommandBuffer, const VkBaseInStructure*, void* pUserData)
    {
        assert(pStateTrackedCommandBuffer);
        assert(pStateTrackedCommandBuffer->type == VK_OBJECT_TYPE_COMMAND_BUFFER);
        assert(pUserData);
        for (auto gvkRestorePoint : *(std::set<GvkRestorePoint>*)pUserData) {
#if 0
            gvkRestorePoint->objectMap.register_object_destruction(*pStateTrackedCommandBuffer);
#else
            gvkRestorePoint->createdObjects.erase(*pStateTrackedCommandBuffer);
#endif
        }
    };
    enumerateInfo.pUserData = &get_restore_points();
    gvkEnumerateStateTrackedObjects(&stateTrackedCommandPool, &enumerateInfo);
    for (auto gvkRestorePoint : get_restore_points()) {
#if 0
        gvkRestorePoint->objectMap.register_object_destruction(stateTrackedCommandPool);
#else
        gvkRestorePoint->createdObjects.erase(stateTrackedCommandPool);
#endif
    }
}

void Layer::post_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
    (void)device;
    (void)commandPool;
    (void)pAllocator;
}

VkResult Layer::pre_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult gvkResult)
{
    (void)flags;
    for (auto gvkRestorePoint : get_restore_points()) {
        auto stateTrackedCommandBuffer = get_default<GvkStateTrackedObject>();
        stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
        stateTrackedCommandBuffer.handle = (uint64_t)commandBuffer;
        stateTrackedCommandBuffer.dispatchableHandle = (uint64_t)stateTrackedCommandBuffer.handle;
        gvkRestorePoint->stateRestorationRequired.insert(stateTrackedCommandBuffer);
        gvkRestorePoint->dataRestorationRequired.insert(stateTrackedCommandBuffer);
    }
    return gvkResult;
}

VkResult Layer::post_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult gvkResult)
{
    (void)commandBuffer;
    (void)flags;
    return gvkResult;
}

VkResult Layer::pre_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult gvkResult)
{
    (void)device;
    (void)commandPool;
    (void)flags;
    auto stateTrackedCommandPool = get_default<GvkStateTrackedObject>();
    stateTrackedCommandPool.type = VK_OBJECT_TYPE_COMMAND_POOL;
    stateTrackedCommandPool.handle = (uint64_t)commandPool;
    stateTrackedCommandPool.dispatchableHandle = (uint64_t)device;
    GvkStateTrackedObjectEnumerateInfo enumerateInfo{ };
    enumerateInfo.pfnCallback = [](const GvkStateTrackedObject* pStateTrackedCommandBuffer, const VkBaseInStructure*, void* pUserData)
    {
        assert(pStateTrackedCommandBuffer);
        assert(pStateTrackedCommandBuffer->type == VK_OBJECT_TYPE_COMMAND_BUFFER);
        assert(pUserData);
        for (auto gvkRestorePoint : *(std::set<GvkRestorePoint>*)pUserData) {
            gvkRestorePoint->stateRestorationRequired.insert(*pStateTrackedCommandBuffer);
            gvkRestorePoint->dataRestorationRequired.insert(*pStateTrackedCommandBuffer);
        }
    };
    enumerateInfo.pUserData = &get_restore_points();
    gvkEnumerateStateTrackedObjects(&stateTrackedCommandPool, &enumerateInfo);
    return gvkResult;
}

VkResult Layer::post_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult gvkResult)
{
    (void)device;
    (void)commandPool;
    (void)flags;
    return gvkResult;
}

VkResult Layer::pre_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers, VkResult gvkResult)
{
    (void)device;
    (void)pAllocateInfo;
    (void)pCommandBuffers;
    // NOOP :
    return gvkResult;
}

VkResult Layer::post_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers, VkResult gvkResult)
{
    (void)device;
    if (gvkResult == VK_SUCCESS) {
        for (auto gvkRestorePoint : get_restore_points()) {
            assert(gvkRestorePoint);
            assert(pAllocateInfo);
            assert(pCommandBuffers);
            for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; ++i) {
                auto stateTrackedCommandBuffer = get_default<GvkStateTrackedObject>();
                stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
                stateTrackedCommandBuffer.handle = (uint64_t)pCommandBuffers[i];
                stateTrackedCommandBuffer.dispatchableHandle = stateTrackedCommandBuffer.handle;
#if 0
                gvkRestorePoint->objectDestructionRequired.insert(stateTrackedCommandBuffer);
#else
                gvkRestorePoint->createdObjects.insert(stateTrackedCommandBuffer);
#endif
            }
        }
    }
    return gvkResult;
}

void Layer::pre_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    (void)commandPool;
    for (auto gvkRestorePoint : get_restore_points()) {
        assert(gvkRestorePoint);
        for (uint32_t i = 0; i < commandBufferCount; ++i) {
            if (pCommandBuffers[i]) {
                auto stateTrackedCommandBuffer = get_default<GvkStateTrackedObject>();
                stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
                stateTrackedCommandBuffer.handle = (uint64_t)pCommandBuffers[i];
                stateTrackedCommandBuffer.dispatchableHandle = (uint64_t)device;
#if 0
                gvkRestorePoint->objectMap.register_object_destruction(stateTrackedCommandBuffer);
                gvkRestorePoint->objectDestructionRequired.erase(stateTrackedCommandBuffer);
#else
                gvkRestorePoint->createdObjects.erase(stateTrackedCommandBuffer);
#endif
            }
        }
    }
}

void Layer::post_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    (void)device;
    (void)commandPool;
    (void)commandBufferCount;
    (void)pCommandBuffers;
}

#if 0
// NOTE : Implemented in "build/gvk-restore-point/source/gvk-restore-point/generated/creator-process-command-buffer.cpp"
VkResult Creator::process_VkCommandBuffer(GvkCommandBufferRestoreInfo& restoreInfo)
{
    return BasicCreator::process_VkCommandBuffer(restorePointCreateInfo, restoreInfo);
}
#endif

#if 0
// NOTE : Implemented in "build/gvk-restore-point/source/gvk-restore-point/generated/creator-process-command-buffer.cpp"
VkResult Applier::restore_VkCommandBuffer_cmds(const GvkStateTrackedObject& capturedCommandBuffer)
{
    (void)capturedCommandBuffer;
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}
#endif

#if 0
VkResult Applier::restore_VkCommandBuffer(const GvkStateTrackedObject& restorePointObject, const GvkCommandBufferRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (restoreInfo.pCommandBufferAllocateInfo) {
            auto commandStructure = get_default<GvkCommandStructureAllocateCommandBuffers>();
            commandStructure.device = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
            commandStructure.pAllocateInfo = restoreInfo.pCommandBufferAllocateInfo;
            gvk_result(process_GvkCommandStructureAllocateCommandBuffers(restorePointObject, restoreInfo, commandStructure));
            gvk_result(update_command_structure_handles(mApplyInfo.gvkRestorePoint->objectMap.get_restored_objects(), commandStructure));
            VkCommandBuffer handle = restoreInfo.handle;
            commandStructure.pCommandBuffers = &handle;
            gvk_result(detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure));
            auto restoredObject = restorePointObject;
            restoredObject.handle = (uint64_t)handle;
            restoredObject.dispatchableHandle = (uint64_t)handle;
            gvk_result(register_restored_object(restorePointObject, restoredObject));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void Applier::destroy_VkCommandBuffer(const GvkStateTrackedObject& restorePointObject)
{
    auto commandStructure = get_default<GvkCommandStructureFreeCommandBuffers>();
    assert(false && "TODO : Get VkDevice");
    commandStructure.device = VK_NULL_HANDLE;
    commandStructure.commandBufferCount = 1;
    commandStructure.pCommandBuffers = (const VkCommandBuffer*)&restorePointObject.handle;
    detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure);
    register_restored_object_destruction(restorePointObject);
}
#endif

VkResult Applier::process_GvkCommandStructureAllocateCommandBuffers(const GvkStateTrackedObject& restorePointObject, const GvkCommandBufferRestoreInfo& restoreInfo, GvkCommandStructureAllocateCommandBuffers& commandStructure)
{
    // TODO : Reset after processing
    (void)restorePointObject;
    (void)restoreInfo;
    assert(commandStructure.pAllocateInfo);
    const_cast<VkCommandBufferAllocateInfo*>(commandStructure.pAllocateInfo)->commandBufferCount = 1;
    return VK_SUCCESS;
}

VkResult update_command_structure_handles(const std::map<GvkStateTrackedObject, GvkStateTrackedObject>& restorePointObjects, const GvkCommandStructureBeginCommandBuffer& commandStructure)
{
    auto vkResult = VK_SUCCESS;
    GvkStateTrackedObject dispatchableRestorePointObject{ };
    dispatchableRestorePointObject.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
    dispatchableRestorePointObject.handle = (uint64_t)commandStructure.commandBuffer;
    dispatchableRestorePointObject.dispatchableHandle = (uint64_t)commandStructure.commandBuffer;
    detail::enumerate_structure_handles(
        commandStructure,
        [&](VkObjectType objectType, const uint64_t& handle)
        {
            if (handle) {
                GvkStateTrackedObject restorePointObject{ };
                restorePointObject.type = objectType;
                restorePointObject.handle = handle;
                restorePointObject.dispatchableHandle = dispatchableRestorePointObject.handle;
                auto itr = restorePointObjects.find(restorePointObject);
                if (itr != restorePointObjects.end()) {
                    const_cast<uint64_t&>(handle) = itr->second.handle;
                } else if (objectType == VK_OBJECT_TYPE_FRAMEBUFFER) {
                    const_cast<uint64_t&>(handle) = 0;
                } else {
                    vkResult = VK_INCOMPLETE;
                }
            }
        }
    );
    return vkResult;
}

VkResult update_command_structure_handles(const std::map<GvkStateTrackedObject, GvkStateTrackedObject>& restorePointObjects, uint64_t parentHandle, const GvkCommandStructureBeginCommandBuffer& commandStructure)
{
    auto vkResult = VK_SUCCESS;
    detail::enumerate_structure_handles(
        commandStructure,
        [&](VkObjectType objectType, const uint64_t& handle)
        {
            if (handle) {
                GvkStateTrackedObject restorePointObject{ };
                restorePointObject.type = objectType;
                restorePointObject.handle = handle;
                restorePointObject.dispatchableHandle = parentHandle;
                auto itr = restorePointObjects.find(restorePointObject);
                if (itr != restorePointObjects.end()) {
                    const_cast<uint64_t&>(handle) = itr->second.handle;
                } else if (objectType == VK_OBJECT_TYPE_FRAMEBUFFER) {
                    const_cast<uint64_t&>(handle) = 0;
                } else {
                    vkResult = VK_INCOMPLETE;
                }
            }
        }
    );
    return vkResult;
}

} // namespace restore_point
} // namespace gvk
