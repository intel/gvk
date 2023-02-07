
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

#if 0
// NOTE : Defined in /build/gvk-state-tracker/source/generated/enumerate-objects.cpp
void StateTracker::enumerate_state_tracked_objects(const GvkStateTrackedObject* pStateTrackedObject, PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData);
// NOTE : Defined in /build/gvk-state-tracker/source/generated/enumerate-objects.cpp
void StateTracker::enumerate_state_tracked_object_dependencies(const GvkStateTrackedObject* pStateTrackedObject, PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData);
// NOTE : Defined in /build/gvk-state-tracker/source/generated/get-object-status.cpp
void StateTracker::get_state_tracked_object_status(const GvkStateTrackedObject* pStateTrackedObject, GvkStateTrackedObjectStatus* pStateTrackedObjectStatus);
// NOTE : Defined in /build/gvk-state-tracker/source/generated/get-object-create-info.cpp
void StateTracker::get_state_tracked_object_create_info(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo);
#endif

void StateTracker::enumerate_state_tracked_object_bindings(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    assert(pStateTrackedObject);
    assert(pEnumerateInfo);
    assert(pEnumerateInfo->pfnCallback);
    switch (pStateTrackedObject->type) {
    case VK_OBJECT_TYPE_BUFFER:
    {
        Buffer gvkBuffer({ (VkDevice)pStateTrackedObject->dispatchableHandle, (VkBuffer)pStateTrackedObject->handle });
        if (gvkBuffer) {
            auto& gvkBufferControlBlock = gvkBuffer.mReference.get_obj();
            if (gvkBufferControlBlock.mBindBufferMemoryInfo->sType == gvk::get_stype<VkBindBufferMemoryInfo>()) {
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&*gvkBufferControlBlock.mBindBufferMemoryInfo, pEnumerateInfo->pUserData);
            }
        }
    } break;
    case VK_OBJECT_TYPE_IMAGE:
    {
        Image gvkImage({ (VkDevice)pStateTrackedObject->dispatchableHandle, (VkImage)pStateTrackedObject->handle });
        if (gvkImage) {
            auto& gvkImageControlBlock = gvkImage.mReference.get_obj();
            if (gvkImageControlBlock.mBindImageMemoryInfo->sType == gvk::get_stype<VkBindImageMemoryInfo>()) {
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&*gvkImageControlBlock.mBindImageMemoryInfo, pEnumerateInfo->pUserData);
            }
        }
    } break;
    case VK_OBJECT_TYPE_DEVICE_MEMORY:
    {
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
    case VK_OBJECT_TYPE_COMMAND_BUFFER:
    {
        // TODO : Enumerate cmd bindings
    } break;
    case VK_OBJECT_TYPE_DESCRIPTOR_SET:
    {
        DescriptorSet gvkDescriptorSet({ (VkDevice)pStateTrackedObject->dispatchableHandle, (VkDescriptorSet)pStateTrackedObject->handle });
        if (gvkDescriptorSet) {
            for (const auto& descriptorItr : gvkDescriptorSet.mReference.get_obj().mDescriptors) {
                assert(descriptorItr.first == descriptorItr.second.descriptorSetLayoutBinding.binding);
                const auto& descriptor = descriptorItr.second;
                VkWriteDescriptorSetInlineUniformBlock inlineUniformBlockInfo { };
                auto descriptorInfo = gvk::get_default<VkWriteDescriptorSet>();
                descriptorInfo.dstSet = gvkDescriptorSet;
                descriptorInfo.dstBinding = descriptor.descriptorSetLayoutBinding.binding;
                descriptorInfo.descriptorCount = descriptor.descriptorSetLayoutBinding.descriptorCount;
                descriptorInfo.descriptorType = descriptor.descriptorSetLayoutBinding.descriptorType;
                switch (descriptorInfo.descriptorType) {
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                {
                    assert(descriptorInfo.descriptorCount == descriptor.descriptorBufferInfos.size());
                    descriptorInfo.pBufferInfo = descriptor.descriptorBufferInfos.data();
                } break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                {
                    // TODO : immutable samplers
                    assert(descriptorInfo.descriptorCount == descriptor.descriptorImageInfos.size());
                    descriptorInfo.pImageInfo = descriptor.descriptorImageInfos.data();
                } break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                {
                    assert(descriptorInfo.descriptorCount == descriptor.texelBufferViews.size());
                    descriptorInfo.pTexelBufferView = descriptor.texelBufferViews.data();
                } break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
                {
                    assert(descriptorInfo.descriptorCount == descriptor.inlineUniformBlock.size());
                    inlineUniformBlockInfo = gvk::get_default<VkWriteDescriptorSetInlineUniformBlock>();
                    inlineUniformBlockInfo.dataSize = descriptorInfo.descriptorCount;
                    inlineUniformBlockInfo.pData = descriptor.inlineUniformBlock.data();
                    descriptorInfo.pNext = &inlineUniformBlockInfo;
                } break;
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
                case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
                case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                default:
                {
                    assert(false && "Unsupported VkDescriptorType");
                } break;
                }
                pEnumerateInfo->pfnCallback(pStateTrackedObject, (const VkBaseInStructure*)&descriptorInfo, pEnumerateInfo->pUserData);
            }
        }
    } break;
    default:
    {
    } break;
    }
}

void StateTracker::get_state_tracked_object_allocate_info(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pAllocateInfoType, VkBaseOutStructure* pAllocateInfo)
{
    get_state_tracked_object_create_info(pStateTrackedObject, pAllocateInfoType, pAllocateInfo);
}

void StateTracker::get_state_tracked_image_layouts(const GvkStateTrackedObject* pStateTrackedObject, const VkImageSubresourceRange* pImageSubresourceRange, VkImageLayout* pImageLayouts)
{
    assert(pStateTrackedObject);
    assert(pImageSubresourceRange);
    Image image({ (VkDevice)pStateTrackedObject->dispatchableHandle, (VkImage)pStateTrackedObject->handle });
    if (image) {
        image.mReference.get_obj().mImageLayoutTracker.get_image_layouts(*pImageSubresourceRange, pImageLayouts);
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

VK_LAYER_EXPORT void VKAPI_CALL gvkEnumerateStateTrackedObjects(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    gvk::state_tracker::StateTracker::enumerate_state_tracked_objects(pStateTrackedObject, pEnumerateInfo);
}

VK_LAYER_EXPORT void VKAPI_CALL gvkEnumerateStateTrackedObjectDependencies(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    gvk::state_tracker::StateTracker::enumerate_state_tracked_object_dependencies(pStateTrackedObject, pEnumerateInfo);
}

VK_LAYER_EXPORT void VKAPI_CALL gvkEnumerateStateTrackedObjectBindings(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)
{
    gvk::state_tracker::StateTracker::enumerate_state_tracked_object_bindings(pStateTrackedObject, pEnumerateInfo);
}

VK_LAYER_EXPORT void VKAPI_CALL gvkGetStateTrackedObjectInfo(const GvkStateTrackedObject* pStateTrackedObject, GvkStateTrackedObjectInfo* pStateTrackedObjectInfo)
{
    gvk::state_tracker::StateTracker::get_state_tracked_object_info(pStateTrackedObject, pStateTrackedObjectInfo);
}

VK_LAYER_EXPORT void VKAPI_CALL gvkGetStateTrackedObjectCreateInfo(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo)
{
    gvk::state_tracker::StateTracker::get_state_tracked_object_create_info(pStateTrackedObject, pCreateInfoType, pCreateInfo);
}

VK_LAYER_EXPORT void VKAPI_CALL gvkGetStateTrackedObjectAllocateInfo(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pAllocateInfoType, VkBaseOutStructure* pAllocateInfo)
{
    gvk::state_tracker::StateTracker::get_state_tracked_object_allocate_info(pStateTrackedObject, pAllocateInfoType, pAllocateInfo);
}

VK_LAYER_EXPORT void VKAPI_CALL gvkGetStateTrackedImageLayouts(const GvkStateTrackedObject* pStateTrackedImage, const VkImageSubresourceRange* pImageSubresourceRange, VkImageLayout* pImageLayouts)
{
    gvk::state_tracker::StateTracker::get_state_tracked_image_layouts(pStateTrackedImage, pImageSubresourceRange, pImageLayouts);
}

VK_LAYER_EXPORT VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pNegotiateLayerInterface)
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
