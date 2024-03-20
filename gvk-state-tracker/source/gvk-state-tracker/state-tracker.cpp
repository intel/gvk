
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

#include "gvk-layer/registry.hpp"
#include "gvk-state-tracker/state-tracker.hpp"
#include "gvk-state-tracker/generated/state-tracked-handles.hpp"
#include "gvk-structures/defaults.hpp"
#include "gvk-structures/get-stype.hpp"

#include <cassert>
#include <vector>

namespace gvk {
namespace state_tracker {

StateTracker::PhysicalDeviceEnumerationMode StateTracker::smPhysicalDeviceEnumerationMode { PhysicalDeviceEnumerationMode::Application };

#if 0
// NOTE : Defined in /build/gvk-state-tracker/source/generated/set-object-name.cpp
void set_state_tracked_object_name(const GvkStateTrackedObject* pStateTrackedObject, const char* pName);
// NOTE : Defined in /build/gvk-state-tracker/source/generated/enumerate-objects.cpp
void StateTracker::enumerate_state_tracked_objects(const GvkStateTrackedObject* pStateTrackedObject, PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData);
// NOTE : Defined in /build/gvk-state-tracker/source/generated/enumerate-objects.cpp
void StateTracker::enumerate_state_tracked_object_dependencies(const GvkStateTrackedObject* pStateTrackedObject, PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData);
// NOTE : Defined in /build/gvk-state-tracker/source/generated/get-object-status.cpp
void StateTracker::get_state_tracked_object_status(const GvkStateTrackedObject* pStateTrackedObject, GvkStateTrackedObjectStatus* pStateTrackedObjectStatus);
// NOTE : Defined in /build/gvk-state-tracker/source/generated/get-object-create-info.cpp
void StateTracker::get_state_tracked_object_create_info(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo);
#endif

///////////////////////////////////////////////////////////////////////////////
// vkSetDebugUtilsObjectNameEXT()
VkResult StateTracker::post_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        assert(pNameInfo);
        auto stateTrackedObject = get_default<GvkStateTrackedObject>();
        stateTrackedObject.type = pNameInfo->objectType;
        stateTrackedObject.handle = pNameInfo->objectHandle;
        stateTrackedObject.dispatchableHandle = (uint64_t)device;
        switch (stateTrackedObject.type) {
        case VK_OBJECT_TYPE_SURFACE_KHR: {
            stateTrackedObject.dispatchableHandle = (uint64_t)Device(device).get<PhysicalDevice>().get<VkInstance>();
        } break;
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: {
            stateTrackedObject.dispatchableHandle = (uint64_t)Device(device).get<PhysicalDevice>().get<VkInstance>();
        } break;
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT: {
            stateTrackedObject.dispatchableHandle = (uint64_t)Device(device).get<PhysicalDevice>().get<VkInstance>();
        } break;
        case VK_OBJECT_TYPE_DISPLAY_KHR: {
            stateTrackedObject.dispatchableHandle = (uint64_t)Device(device).get<PhysicalDevice>().get<VkPhysicalDevice>();
        } break;
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: {
            stateTrackedObject.dispatchableHandle = (uint64_t)Device(device).get<PhysicalDevice>().get<VkPhysicalDevice>();
        } break;
        default: {
            // NOOP :
        } break;
        }
        set_state_tracked_object_name(&stateTrackedObject, pNameInfo->pObjectName);
    }
    return gvkResult;
}

StateTracker::PhysicalDeviceEnumerationMode StateTracker::get_physical_device_enumeration_mode()
{
    return smPhysicalDeviceEnumerationMode;
}

void StateTracker::set_physical_device_enumeration_mode(PhysicalDeviceEnumerationMode physicalDeviceRetrievalMode)
{
    smPhysicalDeviceEnumerationMode = physicalDeviceRetrievalMode;
}

VkPhysicalDevice StateTracker::get_loader_physical_device_handle(VkPhysicalDevice applicationVkPhyicalDevice)
{
    auto loaderVkPhysicalDeviceItr = layer::Registry::get().VkPhysicalDevices.find(applicationVkPhyicalDevice);
    assert(loaderVkPhysicalDeviceItr != layer::Registry::get().VkPhysicalDevices.end());
    return loaderVkPhysicalDeviceItr->second;
}

void StateTracker::get_state_tracker_physical_device(VkInstance instance, VkPhysicalDevice physicalDevice, VkPhysicalDevice* pStateTrackerPhysicalDevice)
{
    assert(instance);
    (void)instance;
    assert(pStateTrackerPhysicalDevice);
    auto itr = layer::Registry::get().VkPhysicalDevices.find(physicalDevice);
    *pStateTrackerPhysicalDevice = itr != layer::Registry::get().VkPhysicalDevices.end() ? itr->second : VK_NULL_HANDLE;
}

void StateTracker::enumerate_state_tracked_object_bindings(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    assert(pStateTrackedObject);
    assert(pEnumerateInfo);
    assert(pEnumerateInfo->pfnCallback);
    switch (pStateTrackedObject->type) {
    case VK_OBJECT_TYPE_BUFFER: {
        Buffer gvkBuffer({ (VkDevice)pStateTrackedObject->dispatchableHandle, (VkBuffer)pStateTrackedObject->handle });
        if (gvkBuffer) {
            auto& gvkBufferControlBlock = gvkBuffer.mReference.get_obj();
            if (gvkBufferControlBlock.mBindBufferMemoryInfo->sType == gvk::get_stype<VkBindBufferMemoryInfo>()) {
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&*gvkBufferControlBlock.mBindBufferMemoryInfo, pEnumerateInfo->pUserData);
            }
        }
    } break;
    case VK_OBJECT_TYPE_IMAGE: {
        Image gvkImage({ (VkDevice)pStateTrackedObject->dispatchableHandle, (VkImage)pStateTrackedObject->handle });
        if (gvkImage) {
            auto& gvkImageControlBlock = gvkImage.mReference.get_obj();
            if (gvkImageControlBlock.mBindImageMemoryInfo->sType == gvk::get_stype<VkBindImageMemoryInfo>()) {
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&*gvkImageControlBlock.mBindImageMemoryInfo, pEnumerateInfo->pUserData);
            }
        }
    } break;
    case VK_OBJECT_TYPE_DEVICE_MEMORY: {
        DeviceMemory gvkDeviceMemory({ (VkDevice)pStateTrackedObject->dispatchableHandle, (VkDeviceMemory)pStateTrackedObject->handle });
        if (gvkDeviceMemory) {
            const auto& gvkDeviceMemoryControlBlock = gvkDeviceMemory.mReference.get_obj();
            for (const auto& vkBuffer : gvkDeviceMemoryControlBlock.mVkBufferBindings) {
                Buffer gvkBuffer({ (VkDevice)pStateTrackedObject->dispatchableHandle, vkBuffer });
                assert(gvkBuffer);
                const auto& gvkBufferControlBlock = gvkBuffer.mReference.get_obj();
                if (gvkBufferControlBlock.mBindBufferMemoryInfo->sType == gvk::get_stype<VkBindBufferMemoryInfo>()) {
                    pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&*gvkBufferControlBlock.mBindBufferMemoryInfo, pEnumerateInfo->pUserData);
                }
            }
            for (const auto& vkImage : gvkDeviceMemoryControlBlock.mVkImageBindings) {
                Image gvkImage({ (VkDevice)pStateTrackedObject->dispatchableHandle, vkImage });
                assert(gvkImage);
                const auto& gvkImageControlBlock = gvkImage.mReference.get_obj();
                if (gvkImageControlBlock.mBindImageMemoryInfo->sType == gvk::get_stype<VkBindImageMemoryInfo>()) {
                    pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&*gvkImageControlBlock.mBindImageMemoryInfo, pEnumerateInfo->pUserData);
                }
            }
        }
    } break;
    case VK_OBJECT_TYPE_COMMAND_BUFFER: {
        // TODO : Enumerate cmd bindings
    } break;
    case VK_OBJECT_TYPE_DESCRIPTOR_SET: {
        DescriptorSet gvkDescriptorSet({ (VkDevice)pStateTrackedObject->dispatchableHandle, (VkDescriptorSet)pStateTrackedObject->handle });
        if (gvkDescriptorSet) {
            for (const auto& descriptorItr : gvkDescriptorSet.mReference.get_obj().mDescriptors) {
                assert(descriptorItr.first == descriptorItr.second.descriptorSetLayoutBinding.binding);
                const auto& descriptor = descriptorItr.second;
                VkWriteDescriptorSetInlineUniformBlock inlineUniformBlockInfo{ };
                VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo{ };
                auto descriptorInfo = get_default<VkWriteDescriptorSet>();
                descriptorInfo.dstSet = gvkDescriptorSet;
                descriptorInfo.dstBinding = descriptor.descriptorSetLayoutBinding.binding;
                descriptorInfo.descriptorCount = descriptor.descriptorSetLayoutBinding.descriptorCount;
                descriptorInfo.descriptorType = descriptor.descriptorSetLayoutBinding.descriptorType;
                switch (descriptorInfo.descriptorType) {
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
                    assert(descriptorInfo.descriptorCount == descriptor.descriptorBufferInfos.size());
                    descriptorInfo.pBufferInfo = descriptor.descriptorBufferInfos.data();
                } break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
                    assert(descriptorInfo.descriptorCount == descriptor.descriptorImageInfos.size());
                    descriptorInfo.pImageInfo = descriptor.descriptorImageInfos.data();
                } break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
                    assert(descriptorInfo.descriptorCount == descriptor.texelBufferViews.size());
                    descriptorInfo.pTexelBufferView = descriptor.texelBufferViews.data();
                } break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
                    assert(descriptorInfo.descriptorCount == descriptor.inlineUniformBlock.size());
                    inlineUniformBlockInfo = get_default<VkWriteDescriptorSetInlineUniformBlock>();
                    inlineUniformBlockInfo.dataSize = descriptorInfo.descriptorCount;
                    inlineUniformBlockInfo.pData = descriptor.inlineUniformBlock.data();
                    descriptorInfo.pNext = &inlineUniformBlockInfo;
                } break;
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
                    assert(descriptorInfo.descriptorCount == descriptor.accelerationStructures.size());
                    accelerationStructureInfo = get_default<VkWriteDescriptorSetAccelerationStructureKHR>();
                    accelerationStructureInfo.accelerationStructureCount = descriptorInfo.descriptorCount;
                    accelerationStructureInfo.pAccelerationStructures = descriptor.accelerationStructures.data();
                    descriptorInfo.pNext = &accelerationStructureInfo;
                } break;
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
                case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
                case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                default: {
                    assert(false && "Unserviced VkDescriptorType");
                } break;
                }
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&descriptorInfo, pEnumerateInfo->pUserData);
            }
        }
    } break;
    default: {
    } break;
    }
}

void StateTracker::enumerate_state_tracked_command_buffer_cmds(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    assert(pStateTrackedObject);
    assert(pEnumerateInfo);
    assert(pEnumerateInfo->pfnCallback);
    switch (pStateTrackedObject->type) {
    case VK_OBJECT_TYPE_COMMAND_BUFFER: {
        CommandBuffer gvkCommandBuffer((VkCommandBuffer)pStateTrackedObject->handle);
        if (gvkCommandBuffer) {
            const auto& commandBufferControlBlock = gvkCommandBuffer.mReference.get_obj();
            auto beginCommandBufferFlags = GVK_STATE_TRACKER_OBJECT_STATUS_RECORDING_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_EXECUTABLE_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_PENDING_BIT;
            if (commandBufferControlBlock.mStateTrackedObjectInfo.flags & beginCommandBufferFlags) {
                GvkCommandStructureBeginCommandBuffer commandStructureBeginCommandBuffer { };
                commandStructureBeginCommandBuffer.sType = get_stype<GvkCommandStructureBeginCommandBuffer>();
                commandStructureBeginCommandBuffer.commandBuffer = commandBufferControlBlock.mVkCommandBuffer;
                commandStructureBeginCommandBuffer.pBeginInfo = &(const VkCommandBufferBeginInfo&)commandBufferControlBlock.mCommandbufferBeginInfo;
                commandStructureBeginCommandBuffer.result = commandBufferControlBlock.mBeginEndCommandBufferResults.first;
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&commandStructureBeginCommandBuffer, pEnumerateInfo->pUserData);
            }
            for (auto pCmd : gvkCommandBuffer.mReference.get_obj().mCmdTracker.get_cmds()) {
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)pCmd, pEnumerateInfo->pUserData);
            }
            auto endCommandBufferFlags = GVK_STATE_TRACKER_OBJECT_STATUS_EXECUTABLE_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_PENDING_BIT;
            if (commandBufferControlBlock.mStateTrackedObjectInfo.flags & endCommandBufferFlags) {
                GvkCommandStructureEndCommandBuffer commandStructureEndCommandBuffer { };
                commandStructureEndCommandBuffer.sType = get_stype<GvkCommandStructureEndCommandBuffer>();
                commandStructureEndCommandBuffer.commandBuffer = commandBufferControlBlock.mVkCommandBuffer;
                commandStructureEndCommandBuffer.result = commandBufferControlBlock.mBeginEndCommandBufferResults.second;
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&commandStructureEndCommandBuffer, pEnumerateInfo->pUserData);
            }
        }
    } break;
    default: {
    } break;
    }
}

void StateTracker::get_state_tracked_object_allocate_info(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pAllocateInfoType, VkBaseOutStructure* pAllocateInfo)
{
    get_state_tracked_object_create_info(pStateTrackedObject, pAllocateInfoType, pAllocateInfo);
}

void StateTracker::get_state_tracked_image_layouts(const GvkStateTrackedObject* pStateTrackedImage, const VkImageSubresourceRange* pImageSubresourceRange, VkImageLayout* pImageLayouts)
{
    assert(pStateTrackedImage);
    assert(pImageSubresourceRange);
    Image image({ (VkDevice)pStateTrackedImage->dispatchableHandle, (VkImage)pStateTrackedImage->handle });
    if (image) {
        image.mReference.get_obj().mImageLayoutTracker.get_image_layouts(*pImageSubresourceRange, pImageLayouts);
    }
}

void StateTracker::get_state_tracked_mapped_memory(const GvkStateTrackedObject* pStateTrackedDeviceMemory, VkDeviceSize* pOffset, VkDeviceSize* pSize, VkMemoryMapFlags* pFlags, void** ppData)
{
    assert(pStateTrackedDeviceMemory);
    DeviceMemory deviceMemory({ (VkDevice)pStateTrackedDeviceMemory->dispatchableHandle, (VkDeviceMemory)pStateTrackedDeviceMemory->handle });
    if (deviceMemory) {
        auto memoryMapInfo = deviceMemory.mReference.get_obj().mMemoryMapInfo;
        if (pOffset) {
            *pOffset = memoryMapInfo.offset;
        }
        if (pSize) {
            *pSize = memoryMapInfo.size;
        }
        if (pFlags) {
            *pFlags = memoryMapInfo.flags;
        }
        if (ppData) {
            *ppData = memoryMapInfo.pData;
        }
    }
}

void StateTracker::gvk_state_tracked_accleration_structure_build_info(const GvkStateTrackedObject* pStateTrackedAcclerationStructure, VkAccelerationStructureBuildGeometryInfoKHR* pBuildGeometryInfo, VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos)
{
    assert(pStateTrackedAcclerationStructure);
    AccelerationStructureKHR accelerationStructure({ (VkDevice)pStateTrackedAcclerationStructure->dispatchableHandle, (VkAccelerationStructureKHR)pStateTrackedAcclerationStructure->handle });
    if (accelerationStructure) {
        const auto& controlBlock = accelerationStructure.mReference.get_obj();
        if (pBuildGeometryInfo) {
            *pBuildGeometryInfo = controlBlock.mBuildGeometryInfo;
        }
        if (pBuildRangeInfos) {
            assert(controlBlock.mBuildGeometryInfo->geometryCount == controlBlock.mBuildRangeInfos.size());
            for (uint32_t i = 0; i < controlBlock.mBuildGeometryInfo->geometryCount; ++i) {
                pBuildRangeInfos[i] = controlBlock.mBuildRangeInfos[i];
            }
        }
    }
}

} // namespace state_tracker
} // namespace gvk

namespace gvk {
namespace layer {

void on_load(Registry& registry)
{
    registry.layers.push_back(std::make_unique<state_tracker::StateTracker>());
}

} // namespace layer
} // namespace gvk

#ifdef __cplusplus
extern "C" {
#endif

void VKAPI_CALL gvkGetStateTrackerPhysicalDevice(VkInstance instance, VkPhysicalDevice physicalDevice, VkPhysicalDevice* pStateTrackerPhysicalDevice)
{
    gvk::state_tracker::StateTracker::get_state_tracker_physical_device(instance, physicalDevice, pStateTrackerPhysicalDevice);
}

void VKAPI_CALL gvkEnumerateStateTrackedObjects(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    gvk::state_tracker::StateTracker::enumerate_state_tracked_objects(pStateTrackedObject, pEnumerateInfo);
}

void VKAPI_CALL gvkEnumerateStateTrackedObjectDependencies(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    gvk::state_tracker::StateTracker::enumerate_state_tracked_object_dependencies(pStateTrackedObject, pEnumerateInfo);
}

void VKAPI_CALL gvkEnumerateStateTrackedObjectBindings(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    gvk::state_tracker::StateTracker::enumerate_state_tracked_object_bindings(pStateTrackedObject, pEnumerateInfo);
}

void VKAPI_CALL gvkEnumerateStateTrackedCommandBufferCmds(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    gvk::state_tracker::StateTracker::enumerate_state_tracked_command_buffer_cmds(pStateTrackedObject, pEnumerateInfo);
}

void VKAPI_CALL gvkGetStateTrackedObjectInfo(const GvkStateTrackedObject* pStateTrackedObject, GvkStateTrackedObjectInfo* pStateTrackedObjectInfo)
{
    gvk::state_tracker::StateTracker::get_state_tracked_object_info(pStateTrackedObject, pStateTrackedObjectInfo);
}

void VKAPI_CALL gvkGetStateTrackedObjectCreateInfo(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo)
{
    gvk::state_tracker::StateTracker::get_state_tracked_object_create_info(pStateTrackedObject, pCreateInfoType, pCreateInfo);
}

void VKAPI_CALL gvkGetStateTrackedObjectAllocateInfo(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pAllocateInfoType, VkBaseOutStructure* pAllocateInfo)
{
    gvk::state_tracker::StateTracker::get_state_tracked_object_allocate_info(pStateTrackedObject, pAllocateInfoType, pAllocateInfo);
}

void VKAPI_CALL gvkGetStateTrackedImageLayouts(const GvkStateTrackedObject* pStateTrackedImage, const VkImageSubresourceRange* pImageSubresourceRange, VkImageLayout* pImageLayouts)
{
    gvk::state_tracker::StateTracker::get_state_tracked_image_layouts(pStateTrackedImage, pImageSubresourceRange, pImageLayouts);
}

void VKAPI_CALL gvkGetStateTrackedMappedMemory(const GvkStateTrackedObject* pStateTrackedDeviceMemory, VkDeviceSize* pOffset, VkDeviceSize* pSize, VkMemoryMapFlags* pFlags, void** ppData)
{
    gvk::state_tracker::StateTracker::get_state_tracked_mapped_memory(pStateTrackedDeviceMemory, pOffset, pSize, pFlags, ppData);
}

void VKAPI_PTR gvkGetStateTrackedAcclerationStructureBuildInfo(const GvkStateTrackedObject* pStateTrackedAcclerationStructure, VkAccelerationStructureBuildGeometryInfoKHR* pBuildGeometryInfo, VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos)
{
    gvk::state_tracker::StateTracker::gvk_state_tracked_accleration_structure_build_info(pStateTrackedAcclerationStructure, pBuildGeometryInfo, pBuildRangeInfos);
}

void VKAPI_CALL gvkDisableStateTracker()
{
    for (auto& layer : gvk::layer::Registry::get().layers) {
        layer->enabled = false;
    }
}

void VKAPI_CALL gvkEnableStateTracker()
{
    for (auto& layer : gvk::layer::Registry::get().layers) {
        layer->enabled = true;
    }
}

VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pNegotiateLayerInterface)
{
    assert(pNegotiateLayerInterface);
    pNegotiateLayerInterface->pfnGetInstanceProcAddr = gvk::layer::get_instance_proc_addr;
    pNegotiateLayerInterface->pfnGetPhysicalDeviceProcAddr = gvk::layer::get_physical_device_proc_addr;
    pNegotiateLayerInterface->pfnGetDeviceProcAddr = gvk::layer::get_device_proc_addr;
    return VK_SUCCESS;
}

#ifdef __cplusplus
}
#endif
