
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

#include "gvk-state-tracker/descriptor.hpp"
#include "gvk-state-tracker/state-tracker.hpp"
#include "gvk-structures/get-stype.hpp"

namespace gvk {
namespace state_tracker {

Descriptor::Descriptor(const VkDescriptorSetLayoutBinding& binding)
    : descriptorSetLayoutBinding { binding }
{
    switch (descriptorSetLayoutBinding.descriptorType) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
        descriptorBufferInfos.resize(descriptorSetLayoutBinding.descriptorCount);
    } break;
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
        descriptorImageInfos.resize(descriptorSetLayoutBinding.descriptorCount);
        if (descriptorSetLayoutBinding.pImmutableSamplers) {
            for (uint32_t i = 0; i < descriptorSetLayoutBinding.descriptorCount; ++i) {
                descriptorImageInfos[i].sampler = descriptorSetLayoutBinding.pImmutableSamplers[i];
            }
        }
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
        texelBufferViews.resize(descriptorSetLayoutBinding.descriptorCount);
    } break;
    case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
        inlineUniformBlock.resize(descriptorSetLayoutBinding.descriptorCount);
    } break;
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
    case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
    case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
    case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
    default: {
        assert(false && "Unsupported VkDescriptorType");
    } break;
    }
}

uint32_t Descriptor::write(const VkWriteDescriptorSet& descriptorWrite)
{
    uint32_t writeCount = 0;
    if (descriptorWrite.descriptorType == descriptorSetLayoutBinding.descriptorType) {
        uint32_t resourceIndex = 0;
        uint32_t dstArrayElement = descriptorWrite.dstArrayElement;
        uint32_t descriptorCount = descriptorWrite.descriptorCount;
        auto updateIndices = [&]()
        {
            --descriptorCount;
            ++dstArrayElement;
            ++resourceIndex;
            ++writeCount;
        };
        switch (descriptorSetLayoutBinding.descriptorType) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            if (descriptorWrite.pBufferInfo) {
                while (descriptorCount && dstArrayElement < descriptorBufferInfos.size()) {
                    descriptorBufferInfos[dstArrayElement] = descriptorWrite.pBufferInfo[resourceIndex];
                    updateIndices();
                }
            }
        } break;
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            if (descriptorWrite.pImageInfo) {
                while (descriptorCount && dstArrayElement < descriptorImageInfos.size()) {
                    descriptorImageInfos[dstArrayElement].imageView = descriptorWrite.pImageInfo[resourceIndex].imageView;
                    descriptorImageInfos[dstArrayElement].imageLayout = descriptorWrite.pImageInfo[resourceIndex].imageLayout;
                    if (!immutableSamplers) {
                        descriptorImageInfos[dstArrayElement].sampler = descriptorWrite.pImageInfo[resourceIndex].sampler;
                    }
                    updateIndices();
                }
            }
        } break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            if (descriptorWrite.pTexelBufferView) {
                while (descriptorCount && dstArrayElement < texelBufferViews.size()) {
                    texelBufferViews[dstArrayElement] = descriptorWrite.pTexelBufferView[resourceIndex];
                    updateIndices();
                }
            }
        } break;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
            // NOTE : If descriptorType is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK then
            //  dstArrayElement specifies the starting byte offset within the inline uniform
            //  block.  Any bytes that aren't written to this Descriptor will rollover to
            //  the next, in this case the writeCount that we return indicates the number
            //  bytes that were written to this Descriptor.  We handle update the offsets
            //  for the rollover to the next Descriptor in write_descriptor_sets().
            // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkDescriptorSetLayoutBinding.html
            auto pWriteDescriptorSetInlineUniformBlock = (const VkWriteDescriptorSetInlineUniformBlock*)descriptorWrite.pNext;
            if (pWriteDescriptorSetInlineUniformBlock) {
                assert(pWriteDescriptorSetInlineUniformBlock->sType == gvk::get_stype<VkWriteDescriptorSetInlineUniformBlock>());
                writeCount = std::min(pWriteDescriptorSetInlineUniformBlock->dataSize, (uint32_t)inlineUniformBlock.size() - dstArrayElement);
                memcpy(inlineUniformBlock.data() + dstArrayElement, pWriteDescriptorSetInlineUniformBlock->pData, writeCount);
            }
        } break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
        default: {
            assert(false && "Unsupported VkDescriptorType");
        } break;
        }
    }
    return writeCount;
}

VkResult StateTracker::post_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout, VkResult gvkResult)
{
    gvkResult = BasicStateTracker::post_vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, gvkResult);
    if (gvkResult == VK_SUCCESS) {
        assert(pSetLayout);
        DescriptorSetLayout gvkDescriptorSetLayout({ device, *pSetLayout });
        assert(gvkDescriptorSetLayout);
        assert(pCreateInfo);
        assert(!pCreateInfo->bindingCount == !pCreateInfo->pBindings);
        for (uint32_t binding_i = 0; binding_i < pCreateInfo->bindingCount; ++binding_i) {
            const auto& binding = pCreateInfo->pBindings[binding_i];
            switch (binding.descriptorType) {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                if (binding.pImmutableSamplers) {
                    auto& immutableSamplers = gvkDescriptorSetLayout.mReference.get_obj().mImmutableSamplers[binding_i];
                    for (uint32_t immutableSampler_i = 0; immutableSampler_i < binding.descriptorCount; ++immutableSampler_i) {
                        immutableSamplers.push_back(Sampler({ device, binding.pImmutableSamplers[immutableSampler_i] }));
                        assert(immutableSamplers.back());
                    }
                }
            } break;
            default: {
            } break;
            }
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags, VkResult gvkResult)
{
    (void)flags;
    if (gvkResult == VK_SUCCESS) {
        DescriptorPool gvkDescriptorPool({ device, descriptorPool });
        assert(gvkDescriptorPool);
        gvkDescriptorPool.mReference.get_obj().mDescriptorSetTracker.clear();
    }
    return gvkResult;
}

void StateTracker::post_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    if (descriptorPool) {
        DescriptorPool gvkDescriptorPool({ device, descriptorPool });
        assert(gvkDescriptorPool);
        gvkDescriptorPool.mReference.get_obj().mDescriptorSetTracker.clear();
    }
    BasicStateTracker::post_vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
}

VkResult StateTracker::post_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        assert(pAllocateInfo);
        assert(pDescriptorSets);
        Device gvkDevice = device;
        assert(gvkDevice);
        DescriptorPool gvkDescriptorPool({ device, pAllocateInfo->descriptorPool });
        assert(gvkDescriptorPool);
        for (uint32_t descriptorSet_i = 0; descriptorSet_i < pAllocateInfo->descriptorSetCount; ++descriptorSet_i) {
            DescriptorSet gvkDescriptorSet;
            gvkDescriptorSet.mReference.reset(gvk::newref, { device, pDescriptorSets[descriptorSet_i] });
            auto& controlBlock = gvkDescriptorSet.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkDescriptorSet = pDescriptorSets[descriptorSet_i];
            controlBlock.mDescriptorPool = gvkDescriptorPool;
            controlBlock.mDescriptorSetLayout = gvk::HandleId<VkDevice, VkDescriptorSetLayout>(device, pAllocateInfo->pSetLayouts[descriptorSet_i]);
            controlBlock.mDevice = gvkDevice;
            controlBlock.mDescriptorSetAllocateInfo = *pAllocateInfo;
            gvkDescriptorPool.mReference.get_obj().mDescriptorSetTracker.insert(gvkDescriptorSet);
            assert(controlBlock.mDescriptorSetLayout);
            auto descriptorSetLayoutCreateInfo = controlBlock.mDescriptorSetLayout.get<VkDescriptorSetLayoutCreateInfo>();
            for (uint32_t binding_i = 0; binding_i < descriptorSetLayoutCreateInfo.bindingCount; ++binding_i) {
                auto inserted = controlBlock.mDescriptors.insert({ binding_i, Descriptor(descriptorSetLayoutCreateInfo.pBindings[binding_i]) }).second;
                (void)inserted;
                assert(inserted);
            }
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS && descriptorSetCount && pDescriptorSets) {
        gvkResult = BasicStateTracker::post_vkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets, gvkResult);
        assert(gvkResult == VK_SUCCESS);
        auto descriptorPoolReference = DescriptorPool({ device, descriptorPool }).mReference;
        assert(descriptorPoolReference);
        auto& descriptorPoolControlBlock = descriptorPoolReference.get_obj();
        for (uint32_t i = 0; i < descriptorSetCount; ++i) {
            DescriptorSet descriptorSet({ device, pDescriptorSets[i] });
            assert(descriptorSet);
            descriptorSet.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
            descriptorSet.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
            descriptorPoolControlBlock.mDescriptorSetTracker.erase(pDescriptorSets[i]);
        }
    }
    return gvkResult;
}

template <typename DescriptorInfoType>
inline const DescriptorInfoType* get_descriptor_writes_from_update_template_entry(const VkDescriptorUpdateTemplateEntry& descriptorUpdateTemplateEntry, const void* pData)
{
    // TODO : Scratchpad allocator
    thread_local std::vector<DescriptorInfoType> tlDescriptorInfos;
    tlDescriptorInfos.resize(descriptorUpdateTemplateEntry.descriptorCount);
    pData = (const uint8_t*)pData + descriptorUpdateTemplateEntry.offset;
    for (uint32_t i = 0; i < descriptorUpdateTemplateEntry.descriptorCount; ++i) {
        tlDescriptorInfos[i] = *(const DescriptorInfoType*)pData;
        pData = (const uint8_t*)pData + descriptorUpdateTemplateEntry.stride;
    }
    return !tlDescriptorInfos.empty() ? tlDescriptorInfos.data() : nullptr;
}

void StateTracker::post_vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData)
{
    assert(pData);
    DescriptorSet gvkDescriptorSet({ device, descriptorSet });
    assert(gvkDescriptorSet);
    DescriptorUpdateTemplate gvkDescriptorUpdateTemplate({ device, descriptorUpdateTemplate });
    assert(gvkDescriptorUpdateTemplate);
    auto descriptorUpdateTemplateCreateInfo = gvkDescriptorUpdateTemplate.get<VkDescriptorUpdateTemplateCreateInfo>();
    assert(!descriptorUpdateTemplateCreateInfo.descriptorUpdateEntryCount == !descriptorUpdateTemplateCreateInfo.pDescriptorUpdateEntries);
    for (uint32_t i = 0; i < descriptorUpdateTemplateCreateInfo.descriptorUpdateEntryCount; ++i) {
        auto descriptorUpdateTemplateEntry = descriptorUpdateTemplateCreateInfo.pDescriptorUpdateEntries[i];
        auto writeDescriptorSetInlineUniformBlock = get_default<VkWriteDescriptorSetInlineUniformBlockEXT>();
        auto writeDescriptorSet = get_default<VkWriteDescriptorSet>();
        writeDescriptorSet.dstSet = gvkDescriptorSet;
        writeDescriptorSet.dstBinding = descriptorUpdateTemplateEntry.dstBinding;
        writeDescriptorSet.dstArrayElement = descriptorUpdateTemplateEntry.dstArrayElement;
        writeDescriptorSet.descriptorCount = descriptorUpdateTemplateEntry.descriptorCount;
        writeDescriptorSet.descriptorType = descriptorUpdateTemplateEntry.descriptorType;
        switch (writeDescriptorSet.descriptorType) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            writeDescriptorSet.pBufferInfo = get_descriptor_writes_from_update_template_entry<VkDescriptorBufferInfo>(descriptorUpdateTemplateEntry, pData);
        } break;
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            writeDescriptorSet.pImageInfo = get_descriptor_writes_from_update_template_entry<VkDescriptorImageInfo>(descriptorUpdateTemplateEntry, pData);
        } break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            writeDescriptorSet.pTexelBufferView = get_descriptor_writes_from_update_template_entry<VkBufferView>(descriptorUpdateTemplateEntry, pData);
        } break;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
            writeDescriptorSetInlineUniformBlock.dataSize = descriptorUpdateTemplateEntry.descriptorCount;
            writeDescriptorSetInlineUniformBlock.pData = (const uint8_t*)pData + descriptorUpdateTemplateEntry.offset;
            writeDescriptorSet.pNext = &writeDescriptorSetInlineUniformBlock;
        } break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
        default: {
            assert(false && "Unsupported VkDescriptorType");
        } break;
        }
        write_descriptor_sets(device, 1, &writeDescriptorSet);
    }
}

void StateTracker::post_vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData)
{
    post_vkUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}

void StateTracker::post_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies)
{
    write_descriptor_sets(device, descriptorWriteCount, pDescriptorWrites);
    copy_descriptor_sets(device, descriptorCopyCount, pDescriptorCopies);
}

void StateTracker::write_descriptor_sets(VkDevice vkDevice, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites)
{
    // NOTE : Updating VkDescriptorSets requires logic to handle rollover of
    //  descriptor entries.  This is necessary when updates specify arrays that
    //  are larger than the array of descriptors at a given binding.  When this
    //  occurs, the next binding with a non zero descriptor count will consume
    //  the remaining updates recursively.
    //  https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#descriptorsets-updates-consecutive
    assert(vkDevice);
    if (descriptorWriteCount && pDescriptorWrites) {
        DescriptorSet dstSet;
        for (uint32_t i = 0; i < descriptorWriteCount; ++i) {
            const auto& descriptorWrite = pDescriptorWrites[i];
            if (dstSet != descriptorWrite.dstSet) {
                dstSet = DescriptorSet({ vkDevice, descriptorWrite.dstSet });
            }
            assert(dstSet);
            auto descriptorCount = descriptorWrite.descriptorCount;
            auto& descriptors = dstSet.mReference.get_obj().mDescriptors;
            auto descriptorItr = descriptors.find(descriptorWrite.dstBinding);
            if (descriptorItr != descriptors.end()) {
                descriptorCount -= descriptorItr->second.write(descriptorWrite);
                ++descriptorItr;
            }
            while (descriptorCount && descriptorItr != descriptors.end()) {
                if (descriptorWrite.descriptorType == descriptorItr->second.descriptorSetLayoutBinding.descriptorType) {
                    VkWriteDescriptorSetInlineUniformBlock rolloverInlineUniformBlock { };
                    auto rolloverDescriptorWrite = descriptorWrite;
                    rolloverDescriptorWrite.dstArrayElement = 0;
                    rolloverDescriptorWrite.descriptorCount = descriptorCount;
                    auto offset = descriptorWrite.descriptorCount - descriptorCount;
                    switch (descriptorWrite.descriptorType) {
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
                        if (descriptorWrite.pBufferInfo) {
                            rolloverDescriptorWrite.pBufferInfo += offset;
                        }
                    } break;
                    case VK_DESCRIPTOR_TYPE_SAMPLER:
                    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
                        if (descriptorWrite.pImageInfo) {
                            rolloverDescriptorWrite.pImageInfo += offset;
                        }
                    } break;
                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
                        if (descriptorWrite.pTexelBufferView) {
                            rolloverDescriptorWrite.pTexelBufferView += offset;
                        }
                    } break;
                    case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
                        // NOTE : If descriptorType is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK then
                        //  descriptorCount is the number of bytes to write.  When a Descriptor doesn't
                        //  have the capacity for the number of bytes provided we'll rollover the bytes
                        //  that remain to the next Descriptor.
                        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkDescriptorSetLayoutBinding.html
                        auto pWriteDescriptorSetInlineUniformBlock = (const VkWriteDescriptorSetInlineUniformBlockEXT*)descriptorWrite.pNext;
                        if (pWriteDescriptorSetInlineUniformBlock) {
                            assert(pWriteDescriptorSetInlineUniformBlock->sType == gvk::get_stype<VkWriteDescriptorSetInlineUniformBlock>());
                            rolloverInlineUniformBlock = *pWriteDescriptorSetInlineUniformBlock;
                            rolloverInlineUniformBlock.pData = (const uint8_t*)pWriteDescriptorSetInlineUniformBlock->pData + offset;
                            rolloverDescriptorWrite.pNext = &rolloverInlineUniformBlock;
                        }
                    } break;
                    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                    case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
                    case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
                    case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                    default: {
                        assert(false && "Unsupported VkDescriptorType");
                    } break;
                    }
                    descriptorCount -= descriptorItr->second.write(rolloverDescriptorWrite);
                    ++descriptorItr;
                } else {
                    break;
                }
            }
        }
    }
}

void StateTracker::copy_descriptor_sets(VkDevice vkDevice, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies)
{
    // NOTE : Updating VkDescriptorSets requires logic to handle rollover of
    //  descriptor entries.  This is necessary when updates specify arrays that
    //  are larger than the array of descriptors at a given binding.  When this
    //  occurs, the next binding with a non zero descriptor count will consume
    //  the remaining updates recursively.
    //  https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#descriptorsets-updates-consecutive
    assert(vkDevice);
    if (descriptorCopyCount && pDescriptorCopies) {
        DescriptorSet srcSet;
        DescriptorSet dstSet;
        for (uint32_t i = 0; i < descriptorCopyCount; ++i) {
            const auto& descriptorCopy = pDescriptorCopies[i];

            if (srcSet != descriptorCopy.srcSet) {
                srcSet = DescriptorSet({ vkDevice, descriptorCopy.srcSet });
            }
            assert(srcSet);
            auto srcArrayElement = descriptorCopy.srcArrayElement;
            auto& srcDescriptors = srcSet.mReference.get_obj().mDescriptors;
            auto srcDescriptorItr = srcDescriptors.lower_bound(descriptorCopy.srcBinding);
            if (srcDescriptorItr != srcDescriptors.end() && srcDescriptorItr->second.descriptorSetLayoutBinding.binding != descriptorCopy.srcBinding) {
                srcArrayElement = 0;
            }

            if (dstSet != descriptorCopy.dstSet) {
                dstSet = DescriptorSet({ vkDevice, descriptorCopy.dstSet });
            }
            assert(dstSet);
            auto dstArrayElement = descriptorCopy.dstArrayElement;
            auto& dstDescriptors = dstSet.mReference.get_obj().mDescriptors;
            auto dstDescriptorItr = dstDescriptors.lower_bound(descriptorCopy.dstBinding);
            if (dstDescriptorItr != dstDescriptors.end() && dstDescriptorItr->second.descriptorSetLayoutBinding.binding != descriptorCopy.dstBinding) {
                dstArrayElement = 0;
            }

            auto descriptorCount = descriptorCopy.descriptorCount;
            while (descriptorCount && srcDescriptorItr != srcDescriptors.end() && dstDescriptorItr != dstDescriptors.end()) {
                const auto& srcDescriptorSetLayoutBinding = srcDescriptorItr->second.descriptorSetLayoutBinding;
                const auto& dstDescriptorSetLayoutBinding = dstDescriptorItr->second.descriptorSetLayoutBinding;
                if (srcDescriptorSetLayoutBinding.descriptorType == dstDescriptorSetLayoutBinding.descriptorType) {
                    while (srcArrayElement < srcDescriptorSetLayoutBinding.descriptorCount && dstArrayElement < dstDescriptorSetLayoutBinding.descriptorCount) {
                        switch (srcDescriptorSetLayoutBinding.descriptorType) {
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
                            assert(srcArrayElement < srcDescriptorItr->second.descriptorBufferInfos.size());
                            assert(dstArrayElement < dstDescriptorItr->second.descriptorBufferInfos.size());
                            dstDescriptorItr->second.descriptorBufferInfos[dstArrayElement++] = srcDescriptorItr->second.descriptorBufferInfos[srcArrayElement++];
                            --descriptorCount;
                        } break;
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
                            assert(srcArrayElement < srcDescriptorItr->second.descriptorImageInfos.size());
                            assert(dstArrayElement < dstDescriptorItr->second.descriptorImageInfos.size());
                            dstDescriptorItr->second.descriptorImageInfos[dstArrayElement].imageView = srcDescriptorItr->second.descriptorImageInfos[srcArrayElement].imageView;
                            dstDescriptorItr->second.descriptorImageInfos[dstArrayElement].imageLayout = srcDescriptorItr->second.descriptorImageInfos[srcArrayElement].imageLayout;
                            if (!dstDescriptorItr->second.immutableSamplers) {
                                dstDescriptorItr->second.descriptorImageInfos[dstArrayElement].sampler = srcDescriptorItr->second.descriptorImageInfos[srcArrayElement].sampler;
                            }
                            ++dstArrayElement;
                            ++srcArrayElement;
                            --descriptorCount;
                        } break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
                            assert(srcArrayElement < srcDescriptorItr->second.texelBufferViews.size());
                            assert(dstArrayElement < dstDescriptorItr->second.texelBufferViews.size());
                            dstDescriptorItr->second.texelBufferViews[dstArrayElement++] = srcDescriptorItr->second.texelBufferViews[srcArrayElement++];
                            --descriptorCount;
                        } break;
                        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
                            assert(srcArrayElement < srcDescriptorItr->second.inlineUniformBlock.size());
                            assert(dstArrayElement < dstDescriptorItr->second.inlineUniformBlock.size());
                            dstDescriptorItr->second.inlineUniformBlock[dstArrayElement++] = srcDescriptorItr->second.inlineUniformBlock[srcArrayElement++];
                            --descriptorCount;
                        } break;
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
                        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
                        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                        default: {
                            assert(false && "Unsupported VkDescriptorType");
                        } break;
                        }
                        while (srcDescriptorItr != srcDescriptors.end() && srcDescriptorItr->second.descriptorSetLayoutBinding.descriptorCount <= srcArrayElement) {
                            srcArrayElement = 0;
                            ++srcDescriptorItr;
                        }
                        while (dstDescriptorItr != dstDescriptors.end() && dstDescriptorItr->second.descriptorSetLayoutBinding.descriptorCount <= dstArrayElement) {
                            dstArrayElement = 0;
                            ++dstDescriptorItr;
                        }
                        if (srcDescriptorItr == srcDescriptors.end() || dstDescriptorItr == dstDescriptors.end()) {
                            break;
                        }
                    }
                } else {
                    assert(false && "VkDescriptorType mismatch");
                    break;
                }
            }
        }
    }
}

} // namespace state_tracker
} // namespace gvk
