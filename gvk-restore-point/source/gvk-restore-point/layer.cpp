
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

#include "gvk-restore-point/layer.hpp"
#include "gvk-layer/registry.hpp"
#include "gvk-restore-point/applier.hpp"
#include "gvk-restore-point/creator.hpp"
#include "gvk-structures/auto.hpp"
#include "gvk-structures/defaults.hpp"
#include "gvk-structures/pnext.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-environment.hpp"

// TODO : Straighten out layer interfaces...
#define VK_LAYER_INTEL_gvk_state_tracker_hpp_IMPLEMENTATION
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#include <cassert>
#include <fstream>
#include <unordered_set>
#include <vector>

namespace gvk {
namespace restore_point {

///////////////////////////////////////////////////////////////////////////////
// vkCreateInstance()
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

///////////////////////////////////////////////////////////////////////////////
// vkCreateDevice()
thread_local VkDeviceCreateInfo tlApplicationDeviceCreateInfo;
thread_local VkDeviceCreateInfo tlRestorePointDeviceCreateInfo;
VkResult Layer::pre_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult)
{
    (void)physicalDevice;
    (void)pAllocator;
    (void)pDevice;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        tlApplicationDeviceCreateInfo = *pCreateInfo;
        tlRestorePointDeviceCreateInfo = tlApplicationDeviceCreateInfo;

        auto pNext = (VkBaseOutStructure*)pCreateInfo->pNext;
        while (pNext) {
            switch (pNext->sType) {
            case get_stype<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(): {
                ((VkPhysicalDeviceAccelerationStructureFeaturesKHR*)pNext)->accelerationStructureCaptureReplay = ((VkPhysicalDeviceAccelerationStructureFeaturesKHR*)pNext)->accelerationStructure;
            } break;
            case get_stype<VkPhysicalDeviceBufferDeviceAddressFeatures>(): {
                ((VkPhysicalDeviceBufferDeviceAddressFeatures*)pNext)->bufferDeviceAddressCaptureReplay = ((VkPhysicalDeviceBufferDeviceAddressFeatures*)pNext)->bufferDeviceAddress;
            } break;
            case get_stype<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT>(): {
                ((VkPhysicalDeviceBufferDeviceAddressFeaturesEXT*)pNext)->bufferDeviceAddressCaptureReplay = ((VkPhysicalDeviceBufferDeviceAddressFeaturesEXT*)pNext)->bufferDeviceAddress;
            } break;
            case get_stype<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(): {
                ((VkPhysicalDeviceDescriptorBufferFeaturesEXT*)pNext)->descriptorBufferCaptureReplay = ((VkPhysicalDeviceDescriptorBufferFeaturesEXT*)pNext)->descriptorBuffer;
            } break;
            case get_stype<VkPhysicalDeviceOpacityMicromapFeaturesEXT>(): {
                ((VkPhysicalDeviceOpacityMicromapFeaturesEXT*)pNext)->micromapCaptureReplay = ((VkPhysicalDeviceOpacityMicromapFeaturesEXT*)pNext)->micromap;
            } break;
            case get_stype<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(): {
                // ((VkPhysicalDeviceRayTracingPipelineFeaturesKHR*)pNext)->rayTracingPipelineShaderGroupHandleCaptureReplay = ((VkPhysicalDeviceRayTracingPipelineFeaturesKHR*)pNext)->rayTracingPipeline;
                // ((VkPhysicalDeviceRayTracingPipelineFeaturesKHR*)pNext)->rayTracingPipelineShaderGroupHandleCaptureReplayMixed = ((VkPhysicalDeviceRayTracingPipelineFeaturesKHR*)pNext)->rayTracingPipeline;
            } break;
            case get_stype<VkPhysicalDeviceVulkan12Features>(): {
                ((VkPhysicalDeviceVulkan12Features*)pNext)->bufferDeviceAddressCaptureReplay = ((VkPhysicalDeviceVulkan12Features*)pNext)->bufferDeviceAddress;
            } break;
            default: {
            } break;
            }
            pNext = pNext->pNext;
        }

        *const_cast<VkDeviceCreateInfo*>(pCreateInfo) = tlRestorePointDeviceCreateInfo;
    }
    return gvkResult;
}

VkResult Layer::post_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult)
{
    (void)physicalDevice;
    (void)pAllocator;
    (void)pDevice;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        *const_cast<VkDeviceCreateInfo*>(pCreateInfo) = tlApplicationDeviceCreateInfo;
    }
    return gvkResult;
}

///////////////////////////////////////////////////////////////////////////////
// vkAllocateMemory()
VkResult Layer::pre_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult gvkResult)
{
    (void)device;
    (void)pAllocator;
    (void)pMemory;
    if (gvkResult == VK_SUCCESS) {
        assert(pAllocateInfo);
        auto pMemoryAllocateFlagsInfo = const_cast<VkMemoryAllocateFlagsInfo*>(get_pnext<VkMemoryAllocateFlagsInfo>(*pAllocateInfo));
        if (pMemoryAllocateFlagsInfo && pMemoryAllocateFlagsInfo->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT) {
            pMemoryAllocateFlagsInfo->flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
        }
    }
    return gvkResult;
}

///////////////////////////////////////////////////////////////////////////////
// vkCreateAccelerationStructureKHR()
VkResult Layer::pre_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure, VkResult gvkResult)
{
    (void)device;
    (void)pAllocator;
    (void)pAccelerationStructure;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        const_cast<VkAccelerationStructureCreateInfoKHR*>(pCreateInfo)->createFlags |= VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    }
    return gvkResult;
}

///////////////////////////////////////////////////////////////////////////////
// vkCreateBuffer()
VkResult Layer::pre_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, VkResult gvkResult)
{
    (void)device;
    (void)pAllocator;
    (void)pBuffer;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        if (pCreateInfo->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            const_cast<VkBufferCreateInfo*>(pCreateInfo)->flags |= VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
        }
        const_cast<VkBufferCreateInfo*>(pCreateInfo)->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    return gvkResult;
}

///////////////////////////////////////////////////////////////////////////////
// vkCreateImage()
VkResult Layer::pre_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage, VkResult gvkResult)
{
    (void)device;
    (void)pAllocator;
    (void)pImage;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        if (!(pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
            const_cast<VkImageCreateInfo*>(pCreateInfo)->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
    }
    return gvkResult;
}

///////////////////////////////////////////////////////////////////////////////
// vkCreateSwapchainKHR()
VkResult Layer::pre_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult)
{
    (void)device;
    (void)pAllocator;
    (void)pSwapchain;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        const_cast<VkSwapchainCreateInfoKHR*>(pCreateInfo)->imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    return gvkResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VkResult Layer::pre_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult gvkResult)
{
    auto command = gvk::get_default<GvkCommandStructureResetCommandBuffer>();
    command.commandBuffer = commandBuffer;
    command.flags = flags;
    (void)command;
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
    (void)flags;
    if (mLayerInfo.objectMap.get_manifest().objectCount) {
        GvkStateTrackedObject stateTrackedCommandPool{ };
        stateTrackedCommandPool.type = VK_OBJECT_TYPE_COMMAND_POOL;
        stateTrackedCommandPool.handle = (uint64_t)commandPool;
        stateTrackedCommandPool.dispatchableHandle = (uint64_t)device;
        GvkStateTrackedObjectEnumerateInfo enumerateInfo{ };
        enumerateInfo.pfnCallback = [](const GvkStateTrackedObject* pStateTrackedCommandBuffer, const VkBaseInStructure*, void* pUserData)
        {
            assert(pStateTrackedCommandBuffer);
            assert(pStateTrackedCommandBuffer->type == VK_OBJECT_TYPE_COMMAND_BUFFER);
            assert(pUserData);
            ((LayerInfo*)pUserData)->register_object_destruction(*(const GvkRestorePointObject*)pStateTrackedCommandBuffer);
        };
        enumerateInfo.pUserData = &mLayerInfo;
        gvkEnumerateStateTrackedObjects(&stateTrackedCommandPool, &enumerateInfo);
    }
    return gvkResult;
}

VkResult Layer::post_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult gvkResult)
{
    (void)device;
    (void)commandPool;
    (void)flags;
    return gvkResult;
}

VkResult Layer::pre_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags, VkResult gvkResult)
{
    (void)flags;
    if (mLayerInfo.objectMap.get_manifest().objectCount) {
        GvkStateTrackedObject stateTrackedDescriptorPool{ };
        stateTrackedDescriptorPool.type = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
        stateTrackedDescriptorPool.handle = (uint64_t)descriptorPool;
        stateTrackedDescriptorPool.dispatchableHandle = (uint64_t)device;
        GvkStateTrackedObjectEnumerateInfo enumerateInfo{ };
        enumerateInfo.pfnCallback = [](const GvkStateTrackedObject* pStateTrackedDescriptorSet, const VkBaseInStructure*, void* pUserData)
        {
            assert(pStateTrackedDescriptorSet);
            assert(pStateTrackedDescriptorSet->type == VK_OBJECT_TYPE_DESCRIPTOR_SET);
            assert(pUserData);
            ((LayerInfo*)pUserData)->register_object_destruction(*(const GvkRestorePointObject*)pStateTrackedDescriptorSet);
        };
        enumerateInfo.pUserData = &mLayerInfo;
        gvkEnumerateStateTrackedObjects(&stateTrackedDescriptorPool, &enumerateInfo);
    }
    return gvkResult;
}

VkResult Layer::post_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags, VkResult gvkResult)
{
    (void)device;
    (void)descriptorPool;
    (void)flags;
    return gvkResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VkResult Layer::create_restore_point(VkInstance instance, const GvkRestorePointCreateInfo* pCreateInfo, GvkRestorePoint* pRestorePoint)
{
    assert(instance);
    assert(pCreateInfo);
    assert(pRestorePoint);
    *pRestorePoint = new GvkRestorePoint_T;

    CreateInfo createInfo { };
    auto defaultFlags =
        GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT |
        GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT |
        // GVK_RESTORE_POINT_CREATE_DEVICE_MEMORY_DATA_BIT |
        GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT |
        GVK_RESTORE_POINT_CREATE_BUFFER_DATA_BIT |
        GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT;
    createInfo.flags = pCreateInfo->flags ? pCreateInfo->flags : defaultFlags;
    createInfo.instance = instance;
    createInfo.threadCount = 0; // TODO : Enable user control...pCreateInfo->threadCount
    createInfo.pfnInitializeThreadCallback = pCreateInfo->pfnInitializeThreadCallback;
    createInfo.pfnProcessResourceDataCallback = pCreateInfo->pfnProcessResourceDataCallback;
    if (string::to_lower(get_env_var("DIRECT_MEMORY")) == "true") {
        createInfo.flags =
            GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT |
            GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT |
            GVK_RESTORE_POINT_CREATE_DEVICE_MEMORY_DATA_BIT;
    } else {
        createInfo.flags =
            GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT |
            GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT |
            GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT |
            GVK_RESTORE_POINT_CREATE_BUFFER_DATA_BIT |
            GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT;
    }
    if (pCreateInfo->pPath) {
        createInfo.path = pCreateInfo->pPath;
    } else if (pCreateInfo->pwPath) {
        createInfo.path = pCreateInfo->pwPath;
    }
    if (createInfo.path.empty()) {
        createInfo.path = "gvk-restore-point";
    }
    createInfo.repeating_HACK = pCreateInfo->repeating_HACK;
    Creator creator;
    auto vkResult = creator.create_restore_point(createInfo);
    if (createInfo.repeating_HACK) {
        const auto& restorePointObjects = creator.get_restore_point_objects();
        for (const auto& restorePointObject : restorePointObjects) {
            auto inserted = (*pRestorePoint)->objects.insert((const GvkStateTrackedObject&)restorePointObject).second;
            (void)inserted;
            assert(inserted);
        }
        auto restorePointManifest = get_default<GvkRestorePointManifest>();
        restorePointManifest.objectCount = (uint32_t)restorePointObjects.size();
        restorePointManifest.pObjects = restorePointObjects.data();
        assert(layer::Registry::get().layers.size() == 1);
        auto pLayer = (restore_point::Layer*)layer::Registry::get().layers[0].get();
        pLayer->mLayerInfo.objectMap = restorePointManifest;
    }

    return vkResult;
}

VkResult Layer::get_restore_point_objects(VkInstance instance, GvkRestorePoint restorePoint, uint32_t* pRestorePointObjectCount, GvkStateTrackedObject* pRestorePointObjects)
{
    (void)instance;
    assert(instance);
    assert(restorePoint);
    if (pRestorePointObjectCount) {
        if (pRestorePointObjects) {
            auto itr = restorePoint->objects.begin();
            auto restorePointObjectCount = *pRestorePointObjectCount;
            while (itr != restorePoint->objects.end() && restorePointObjectCount) {
                *pRestorePointObjects = *itr;
                --restorePointObjectCount;
                ++pRestorePointObjects;
                ++itr;
            }
        } else {
            *pRestorePointObjectCount = (uint32_t)restorePoint->objects.size();
        }
    }
    return pRestorePointObjectCount && *pRestorePointObjectCount == (uint32_t)restorePoint->objects.size() ? VK_SUCCESS : VK_INCOMPLETE;
}

VkResult Layer::apply_restore_point(VkInstance instance, const GvkRestorePointApplyInfo* pApplyInfo, GvkRestorePoint restorePoint)
{
    (void)restorePoint;
    assert(instance);
    assert(pApplyInfo);
    ApplyInfo applyInfo{ };
    applyInfo.flags = pApplyInfo->flags;
    applyInfo.instance = instance;
    if (pApplyInfo->pPath) {
        applyInfo.path = pApplyInfo->pPath;
    } else if (pApplyInfo->pwPath) {
        applyInfo.path = pApplyInfo->pwPath;
    }
    applyInfo.threadCount = pApplyInfo->threadCount;
    if (pApplyInfo->excludedObjectCount && pApplyInfo->pExcludedObjects) {
        applyInfo.excludedObjects.insert(pApplyInfo->pExcludedObjects, pApplyInfo->pExcludedObjects + pApplyInfo->excludedObjectCount);
    }
    applyInfo.pfnInitializeThreadCallback = pApplyInfo->pfnInitializeThreadCallback;
    applyInfo.pfnProcessResourceDataCallback = pApplyInfo->pfnProcessResourceDataCallback;
    applyInfo.pfnProcessRestoredObjectCallback = pApplyInfo->pfnProcessRestoredObjectCallback;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    applyInfo.pfnProcessWin32SurfaceCreateInfoCallback = pApplyInfo->pfnProcessWin32SurfaceCreateInfoCallback;
#endif
    if (pApplyInfo->pfnGetInstanceProcAddr) {
        applyInfo.restoreInstance_HACK = true;
        applyInfo.dispatchTable.gvkGetInstanceProcAddr = pApplyInfo->pfnGetInstanceProcAddr;
        DispatchTable::load_instance_entry_points(instance, &applyInfo.dispatchTable);
    } else {
        DispatchTable::load_global_entry_points(&applyInfo.dispatchTable);
        DispatchTable::load_instance_entry_points(instance, &applyInfo.dispatchTable);
    }

    applyInfo.repeating_HACK = pApplyInfo->repeating_HACK;
    if (applyInfo.repeating_HACK) {
        auto& layers = layer::Registry::get().layers;
        assert(layers.size() == 1);
        auto pLayer = (restore_point::Layer*)layers[0].get();
        applyInfo.pLayerInfo = &pLayer->mLayerInfo;
    }
    state_tracker::load_layer_entry_points();
    assert(layer::Registry::get().layers.size() == 1);
    layer::Registry::get().layers.back()->enabled = false;
    auto vkResult = !applyInfo.path.empty() ? Applier().apply_restore_point(applyInfo) : VK_ERROR_INITIALIZATION_FAILED;
    layer::Registry::get().layers.back()->enabled = true;
    return vkResult;
}

void Layer::destroy_restore_point(VkInstance instance, GvkRestorePoint restorePoint)
{
    (void)instance;
    assert(instance);
    delete restorePoint;
}

} // namespace restore_point
} // namespace gvk

namespace gvk {
namespace layer {

void on_load(Registry& registry)
{
    registry.layers.push_back(std::make_unique<restore_point::Layer>());
}

} // namespace layer
} // namespace gvk

extern "C" {

VkResult VKAPI_CALL gvkCreateRestorePoint(VkInstance instance, const GvkRestorePointCreateInfo* pCreateInfo, GvkRestorePoint* pRestorePoint)
{
    return gvk::restore_point::Layer::create_restore_point(instance, pCreateInfo, pRestorePoint);
}

VkResult gvkGetRestorePointObjects(VkInstance instance, GvkRestorePoint restorePoint, uint32_t* pRestorePointObjectCount, GvkStateTrackedObject* pRestorePointObjects)
{
    return gvk::restore_point::Layer::get_restore_point_objects(instance, restorePoint, pRestorePointObjectCount, pRestorePointObjects);
}

VkResult VKAPI_CALL gvkApplyRestorePoint(VkInstance instance, const GvkRestorePointApplyInfo* pApplyInfo, GvkRestorePoint restorePoint)
{
    return gvk::restore_point::Layer::apply_restore_point(instance, pApplyInfo, restorePoint);
}

void VKAPI_CALL gvkDestroyRestorePoint(VkInstance instance, GvkRestorePoint restorePoint)
{
    gvk::restore_point::Layer::destroy_restore_point(instance, restorePoint);
}

VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pNegotiateLayerInterface)
{
    assert(pNegotiateLayerInterface);
    pNegotiateLayerInterface->pfnGetInstanceProcAddr = gvk::layer::get_instance_proc_addr;
    pNegotiateLayerInterface->pfnGetPhysicalDeviceProcAddr = gvk::layer::get_physical_device_proc_addr;
    pNegotiateLayerInterface->pfnGetDeviceProcAddr = gvk::layer::get_device_proc_addr;
    return VK_SUCCESS;
}

} // extern "C"
