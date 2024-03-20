
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

#pragma once

#include "gvk-math/defines.hpp"
#include "gvk-spirv/context.hpp"
#include "gvk-system/surface.hpp"
#include "gvk-handles/context.hpp"
#include "gvk-defines.hpp"
#include "gvk-environment.hpp"
#include "gvk-format-info.hpp"
#include "gvk-handles/mesh.hpp"
#include "gvk-handles/render-target.hpp"
#include "gvk-structures/defaults.hpp"
#include "gvk-structures/get-stype.hpp"
#include "gvk-structures/to-string.hpp"
#include "gvk-handles/utilities.hpp"
#include "gvk-handles/wsi-manager.hpp"
#include "gvk-structures/to-string.hpp"
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

#include <array>
#include <map>
#include <vector>

class ObjectRecord;

namespace gvk {

template <> void print<GvkStateTrackedObjectStatusBits>(Printer& printer, const GvkStateTrackedObjectStatusBits& value);
template <> void print<GvkStateTrackedObjectStatusBits>(Printer& printer, std::underlying_type_t<GvkStateTrackedObjectStatusBits> flags);
template <> void print<GvkStateTrackedObject>(Printer& printer, const GvkStateTrackedObject& obj);
template <> void print<GvkStateTrackedObjectInfo>(Printer& printer, const GvkStateTrackedObjectInfo& obj);

template <>
inline VkFormat get_vertex_input_attribute_format<glm::vec3>()
{
    return VK_FORMAT_R32G32B32_SFLOAT;
}

template <>
inline auto get_vertex_description<glm::vec3>(uint32_t binding)
{
    return gvk::get_vertex_input_attribute_descriptions<glm::vec3>(binding);
}

} // namespace gvk

class StateTrackerValidationContext final
    : public gvk::Context
{
public:
    static VkResult create(StateTrackerValidationContext* pContext);

protected:
    VkResult create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, const VkAllocationCallbacks*) override final;
};

class ObjectRecord final
{
public:
    ObjectRecord() = default;

    template <typename CreateInfoType>
    inline ObjectRecord(const GvkStateTrackedObject& stateTrackedObject, const CreateInfoType& createInfo, GvkStateTrackedObjectStatusFlags statusFlags)
        : mStateTrackedObject { stateTrackedObject }
        , mStateTrackedObjectInfo { statusFlags, nullptr }
    {
        if constexpr (std::is_same_v<CreateInfoType, VkInstanceCreateInfo>) {
            mInstanceCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkDeviceCreateInfo>) {
            mDeviceCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkDeviceQueueCreateInfo>) {
            mDeviceQueueCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkCommandPoolCreateInfo>) {
            mCommandPoolCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkCommandBufferAllocateInfo>) {
            mCommandBufferAllocateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkShaderModuleCreateInfo>) {
            mShaderModuleCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkDescriptorSetLayoutCreateInfo>) {
            mDescriptorSetLayoutCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkRenderPassCreateInfo2>) {
            mRenderPassCreateInfo2 = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkPipelineLayoutCreateInfo>) {
            mPipelineLayoutCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkComputePipelineCreateInfo>) {
            mComputePipelineCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkGraphicsPipelineCreateInfo>) {
            mGraphicsPipelineCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkBufferCreateInfo>) {
            mBufferCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkImageCreateInfo>) {
            mImageCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkImageViewCreateInfo>) {
            mImageViewCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkMemoryAllocateInfo>) {
            mMemoryAllocateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkDescriptorPoolCreateInfo>) {
            mDescriptorPoolCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkDescriptorSetAllocateInfo>) {
            mDescriptorSetAllocateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkFenceCreateInfo>) {
            mFenceCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkSemaphoreCreateInfo>) {
            mSemaphoreCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkFramebufferCreateInfo>) {
            mFramebufferCreateInfo = createInfo;
        }
        if constexpr (std::is_same_v<CreateInfoType, VkSwapchainCreateInfoKHR>) {
            mSwapchainCreateInfo = createInfo;
        }
#ifdef VK_USE_PLATFORM_XLIB_KHR
        if constexpr (std::is_same_v<CreateInfoType, VkXlibSurfaceCreateInfoKHR>) {
            mXlibSurfaceCreateInfo = createInfo;
        }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        if constexpr (std::is_same_v<CreateInfoType, VkWin32SurfaceCreateInfoKHR>) {
            mWin32SurfaceCreateInfo = createInfo;
        }
#endif
    }

    inline ObjectRecord(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pBindingInfo)
    {
        assert(pStateTrackedObject);
        mStateTrackedObject = *pStateTrackedObject;
        gvkGetStateTrackedObjectInfo(pStateTrackedObject, &mStateTrackedObjectInfo);
        if (pStateTrackedObject->type != VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
            VkStructureType createInfoStructureType { };
            gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, nullptr);
            switch (createInfoStructureType) {
            case gvk::get_stype<VkApplicationInfo>(): {
                // NOOP :
            } break;
            case gvk::get_stype<VkInstanceCreateInfo>(): {
                VkInstanceCreateInfo instanceCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&instanceCreateInfo);
                mInstanceCreateInfo = instanceCreateInfo;
            } break;
            case gvk::get_stype<VkDeviceCreateInfo>(): {
                VkDeviceCreateInfo deviceCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&deviceCreateInfo);
                mDeviceCreateInfo = deviceCreateInfo;
            } break;
            case gvk::get_stype<VkDeviceQueueCreateInfo>(): {
                VkDeviceQueueCreateInfo deviceQueueCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&deviceQueueCreateInfo);
                mDeviceQueueCreateInfo = deviceQueueCreateInfo;
            } break;
            case gvk::get_stype<VkCommandPoolCreateInfo>(): {
                VkCommandPoolCreateInfo commandPoolCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&commandPoolCreateInfo);
                mCommandPoolCreateInfo = commandPoolCreateInfo;
            } break;
            case gvk::get_stype<VkCommandBufferAllocateInfo>(): {
                VkCommandBufferAllocateInfo commandBufferAllocateInfo { };
                gvkGetStateTrackedObjectAllocateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&commandBufferAllocateInfo);
                mCommandBufferAllocateInfo = commandBufferAllocateInfo;
            } break;
            case gvk::get_stype<VkShaderModuleCreateInfo>(): {
                VkShaderModuleCreateInfo shaderModuleCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&shaderModuleCreateInfo);
                mShaderModuleCreateInfo = shaderModuleCreateInfo;
            } break;
            case gvk::get_stype<VkDescriptorSetLayoutCreateInfo>(): {
                VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&descriptorSetLayoutCreateInfo);
                mDescriptorSetLayoutCreateInfo = descriptorSetLayoutCreateInfo;
            } break;
            case gvk::get_stype<VkRenderPassCreateInfo2>(): {
                VkRenderPassCreateInfo2 renderPassCreateInfo2 { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&renderPassCreateInfo2);
                mRenderPassCreateInfo2 = renderPassCreateInfo2;
            } break;
            case gvk::get_stype<VkPipelineLayoutCreateInfo>(): {
                VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&pipelineLayoutCreateInfo);
                mPipelineLayoutCreateInfo = pipelineLayoutCreateInfo;
            } break;
            case gvk::get_stype<VkComputePipelineCreateInfo>(): {
                VkComputePipelineCreateInfo computePipelineCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&computePipelineCreateInfo);
                mComputePipelineCreateInfo = computePipelineCreateInfo;
            } break;
            case gvk::get_stype<VkGraphicsPipelineCreateInfo>(): {
                VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&graphicsPipelineCreateInfo);
                mGraphicsPipelineCreateInfo = graphicsPipelineCreateInfo;
            } break;
            case gvk::get_stype<VkBufferCreateInfo>(): {
                VkBufferCreateInfo bufferCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&bufferCreateInfo);
                mBufferCreateInfo = bufferCreateInfo;
            } break;
            case gvk::get_stype<VkImageCreateInfo>(): {
                VkImageCreateInfo imageCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&imageCreateInfo);
                mImageCreateInfo = imageCreateInfo;
            } break;
            case gvk::get_stype<VkImageViewCreateInfo>(): {
                VkImageViewCreateInfo imageViewCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&imageViewCreateInfo);
                mImageViewCreateInfo = imageViewCreateInfo;
            } break;
            case gvk::get_stype<VkMemoryAllocateInfo>(): {
                VkMemoryAllocateInfo memoryAllocateInfo { };
                gvkGetStateTrackedObjectAllocateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&memoryAllocateInfo);
                mMemoryAllocateInfo = memoryAllocateInfo;
            } break;
            case gvk::get_stype<VkDescriptorPoolCreateInfo>(): {
                VkDescriptorPoolCreateInfo descriptorPoolCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&descriptorPoolCreateInfo);
                mDescriptorPoolCreateInfo = descriptorPoolCreateInfo;
            } break;
            case gvk::get_stype<VkDescriptorSetAllocateInfo>(): {
                VkDescriptorSetAllocateInfo descriptorAllocateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&descriptorAllocateInfo);
                mDescriptorSetAllocateInfo = descriptorAllocateInfo;
            } break;
            case gvk::get_stype<VkFenceCreateInfo>(): {
                VkFenceCreateInfo fenceCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&fenceCreateInfo);
                mFenceCreateInfo = fenceCreateInfo;
            } break;
            case gvk::get_stype<VkSemaphoreCreateInfo>(): {
                VkSemaphoreCreateInfo semaphoreCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&semaphoreCreateInfo);
                mSemaphoreCreateInfo = semaphoreCreateInfo;
            } break;
            case gvk::get_stype<VkFramebufferCreateInfo>(): {
                VkFramebufferCreateInfo framebufferCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&framebufferCreateInfo);
                mFramebufferCreateInfo = framebufferCreateInfo;
            } break;
            case gvk::get_stype<VkSwapchainCreateInfoKHR>(): {
                VkSwapchainCreateInfoKHR swapchainCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&swapchainCreateInfo);
                mSwapchainCreateInfo = swapchainCreateInfo;
            } break;
#ifdef VK_USE_PLATFORM_XLIB_KHR
            case gvk::get_stype<VkXlibSurfaceCreateInfoKHR>(): {
                VkXlibSurfaceCreateInfoKHR xlibSurfaceCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&xlibSurfaceCreateInfo);
                mXlibSurfaceCreateInfo = xlibSurfaceCreateInfo;
            } break;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case gvk::get_stype<VkWin32SurfaceCreateInfoKHR>(): {
                VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo { };
                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoStructureType, (VkBaseOutStructure*)&win32SurfaceCreateInfo);
                mWin32SurfaceCreateInfo = win32SurfaceCreateInfo;
            } break;
#endif
            default: {
                assert(false && "Unexpected VkStructureType");
            } break;
            }
            if (pBindingInfo) {
                switch (pBindingInfo->sType) {
                case VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO: {
                    mBindBufferMemoryInfo = *(const VkBindBufferMemoryInfo*)pBindingInfo;
                } break;
                case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO: {
                    mBindImageMemoryInfo = *(const VkBindImageMemoryInfo*)pBindingInfo;
                } break;
                case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET: {
                    mDescriptorSetBindingInfos.push_back(*(const VkWriteDescriptorSet*)pBindingInfo);
                } break;
                default: {
                    assert(false && "Unexpected VkStructureType");
                } break;
                }
            }
        }
    }

    inline friend bool operator==(const ObjectRecord& lhs, const ObjectRecord& rhs)
    {
        auto makeTuple = [](const ObjectRecord& obj)
        {
            return std::tie(
                obj.mStateTrackedObject,
                obj.mStateTrackedObjectInfo,
                obj.mInstanceCreateInfo,
                obj.mDeviceCreateInfo,
                obj.mDeviceQueueCreateInfo,
                obj.mCommandPoolCreateInfo,
                obj.mCommandBufferAllocateInfo,
                obj.mShaderModuleCreateInfo,
                obj.mDescriptorSetLayoutCreateInfo,
                obj.mRenderPassCreateInfo2,
                obj.mPipelineLayoutCreateInfo,
                obj.mComputePipelineCreateInfo,
                obj.mGraphicsPipelineCreateInfo,
                obj.mBufferCreateInfo,
                obj.mImageCreateInfo,
                obj.mImageViewCreateInfo,
                obj.mMemoryAllocateInfo,
                obj.mDescriptorPoolCreateInfo,
                obj.mDescriptorSetAllocateInfo,
                obj.mFenceCreateInfo,
                obj.mSemaphoreCreateInfo,
                obj.mFramebufferCreateInfo,
                obj.mSwapchainCreateInfo,
#ifdef VK_USE_PLATFORM_XLIB_KHR
                obj.mXlibSurfaceCreateInfo,
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
                obj.mWin32SurfaceCreateInfo,
#endif
                obj.mBindBufferMemoryInfo,
                obj.mBindImageMemoryInfo,
                obj.mDescriptorSetBindingInfos
            );
        };
        return makeTuple(lhs) == makeTuple(rhs);
    }

    inline friend bool operator!=(const ObjectRecord& lhs, const ObjectRecord& rhs)
    {
        return !(lhs == rhs);
    }

    GvkStateTrackedObject mStateTrackedObject { };
    GvkStateTrackedObjectInfo mStateTrackedObjectInfo { };
    // Create infos
    gvk::Auto<VkInstanceCreateInfo> mInstanceCreateInfo;
    gvk::Auto<VkDeviceCreateInfo> mDeviceCreateInfo;
    gvk::Auto<VkDeviceQueueCreateInfo> mDeviceQueueCreateInfo;
    gvk::Auto<VkCommandPoolCreateInfo> mCommandPoolCreateInfo;
    gvk::Auto<VkCommandBufferAllocateInfo> mCommandBufferAllocateInfo;
    gvk::Auto<VkShaderModuleCreateInfo> mShaderModuleCreateInfo;
    gvk::Auto<VkDescriptorSetLayoutCreateInfo> mDescriptorSetLayoutCreateInfo;
    gvk::Auto<VkRenderPassCreateInfo2> mRenderPassCreateInfo2;
    gvk::Auto<VkPipelineLayoutCreateInfo> mPipelineLayoutCreateInfo;
    gvk::Auto<VkComputePipelineCreateInfo> mComputePipelineCreateInfo;
    gvk::Auto<VkGraphicsPipelineCreateInfo> mGraphicsPipelineCreateInfo;
    gvk::Auto<VkBufferCreateInfo> mBufferCreateInfo;
    gvk::Auto<VkImageCreateInfo> mImageCreateInfo;
    gvk::Auto<VkImageViewCreateInfo> mImageViewCreateInfo;
    gvk::Auto<VkMemoryAllocateInfo> mMemoryAllocateInfo;
    gvk::Auto<VkDescriptorPoolCreateInfo> mDescriptorPoolCreateInfo;
    gvk::Auto<VkDescriptorSetAllocateInfo> mDescriptorSetAllocateInfo;
    gvk::Auto<VkFenceCreateInfo> mFenceCreateInfo;
    gvk::Auto<VkSemaphoreCreateInfo> mSemaphoreCreateInfo;
    gvk::Auto<VkFramebufferCreateInfo> mFramebufferCreateInfo;
    gvk::Auto<VkSwapchainCreateInfoKHR> mSwapchainCreateInfo;
#ifdef VK_USE_PLATFORM_XLIB_KHR
    gvk::Auto<VkXlibSurfaceCreateInfoKHR> mXlibSurfaceCreateInfo;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    gvk::Auto<VkWin32SurfaceCreateInfoKHR> mWin32SurfaceCreateInfo;
#endif
    // Binding infos
    gvk::Auto<VkBindBufferMemoryInfo> mBindBufferMemoryInfo;
    gvk::Auto<VkBindImageMemoryInfo> mBindImageMemoryInfo;
    std::vector<gvk::Auto<VkWriteDescriptorSet>> mDescriptorSetBindingInfos;
};

template <typename CreateInfoType>
inline bool create_state_tracked_object_record(const GvkStateTrackedObject& stateTrackedObject, const CreateInfoType& createInfo, std::map<GvkStateTrackedObject, ObjectRecord>& records)
{
    ObjectRecord objectRecord(stateTrackedObject, createInfo, GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT);
    auto recordItr = records.find(stateTrackedObject);
    if (recordItr == records.end()) {
        recordItr = records.insert({ stateTrackedObject, objectRecord }).first;
    }
    return recordItr->second == objectRecord;
}

template <typename GvkHandleType, typename CreateInfoType>
inline bool create_state_tracked_object_record(const GvkHandleType& gvkHandle, const CreateInfoType& createInfo, std::map<GvkStateTrackedObject, ObjectRecord>& records)
{
    return create_state_tracked_object_record(gvk::get_state_tracked_object(gvkHandle), createInfo, records);
}

class StateTrackerValidationEnumerator final
{
public:
    static void enumerate(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData)
    {
        ASSERT_NE(pStateTrackedObject, nullptr);
        ASSERT_NE(pUserData, nullptr);
        auto pStateTrackerEnumerationValidator = (StateTrackerValidationEnumerator*)pUserData;
        ObjectRecord record(pStateTrackedObject, pInfo);
        const auto& result = pStateTrackerEnumerationValidator->records.insert({ *pStateTrackedObject, record });
        if (!result.second) {
            const auto& itr = result.first;
            const auto& existingRecord = itr->second;
            EXPECT_EQ(existingRecord, record);
        }
    }

    std::map<GvkStateTrackedObject, ObjectRecord> records;
};

inline VkFormat get_render_pass_color_format(const gvk::Context& context)
{
    auto colorFormat = VK_FORMAT_UNDEFINED;
    const auto& physicalDevice = context.get_devices()[0].get<gvk::PhysicalDevice>();
    gvk::enumerate_formats(
        physicalDevice.get<gvk::DispatchTable>().gvkGetPhysicalDeviceFormatProperties2,
        physicalDevice,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT,
        [&](VkFormat format)
        {
            GvkFormatInfo formatInfo { };
            gvk::get_format_info(format, &formatInfo);
            if (gvk::get_bits_per_texel(format) == 32 &&
                formatInfo.componentCount == 4 &&
                formatInfo.compressionType == GVK_FORMAT_COMPRESSION_TYPE_NONE &&
                formatInfo.numericFormat == GVK_NUMERIC_FORMAT_UNORM &&
                !formatInfo.packed &&
                !formatInfo.chroma
            ) {
                colorFormat = format;
            }
            return colorFormat == VK_FORMAT_UNDEFINED;
        }
    );
    return colorFormat;
}

inline std::map<GvkStateTrackedObject, ObjectRecord> get_expected_instance_objects(const gvk::Context& context)
{
    std::map<GvkStateTrackedObject, ObjectRecord> expectedInstanceObjects;
    for (const auto& physicalDevice : context.get_physical_devices()) {
        create_state_tracked_object_record(physicalDevice, VkApplicationInfo { }, expectedInstanceObjects);
    }
    for (const auto& device : context.get_devices()) {
        create_state_tracked_object_record(device, device.get<VkDeviceCreateInfo>(), expectedInstanceObjects);
        for (const auto& queueFamily : device.get<gvk::QueueFamilies>()) {
            for (const auto& queue : queueFamily.queues) {
                create_state_tracked_object_record(queue, queue.get<VkDeviceQueueCreateInfo>(), expectedInstanceObjects);
            }
        }
    }
    for (const auto& commandBuffer : context.get_command_buffers()) {
        const auto& commandPool = context.get_command_buffers()[0].get<gvk::CommandPool>();
        create_state_tracked_object_record(commandPool, commandPool.get<VkCommandPoolCreateInfo>(), expectedInstanceObjects);
        create_state_tracked_object_record(commandBuffer, commandBuffer.get<VkCommandBufferAllocateInfo>(), expectedInstanceObjects);
    }
    return expectedInstanceObjects;
}
template <typename ExpectedCreateInfoStructureType>
inline void validate_state_tracked_create_info(const ExpectedCreateInfoStructureType& expectedCreateInfo, const GvkStateTrackedObject& stateTrackedObject)
{
    VkStructureType actualCreateInfoStructureType { };
    gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &actualCreateInfoStructureType, nullptr);
    if (actualCreateInfoStructureType) {
        ASSERT_EQ(actualCreateInfoStructureType, gvk::get_stype<ExpectedCreateInfoStructureType>());
        ExpectedCreateInfoStructureType actualCreateInfo { };
        gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &actualCreateInfoStructureType, (VkBaseOutStructure*)&actualCreateInfo);
        ASSERT_EQ(actualCreateInfoStructureType, gvk::get_stype<ExpectedCreateInfoStructureType>());
        ASSERT_EQ(actualCreateInfo.sType, gvk::get_stype<ExpectedCreateInfoStructureType>());
        auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
        EXPECT_EQ(expectedCreateInfo, actualCreateInfo) << std::endl
            << "================================================================================" << std::endl
            << gvk::to_string(gvk::get_stype<ExpectedCreateInfoStructureType>(), printerFlags) << std::endl
            << "expected " << gvk::to_string(expectedCreateInfo, printerFlags) << std::endl
            << "actual " << gvk::to_string(actualCreateInfo, printerFlags);
    }
}

inline void validate(const std::string& message, const std::map<GvkStateTrackedObject, ObjectRecord>& expectedRecords, const std::map<GvkStateTrackedObject, ObjectRecord>& actualRecords)
{
    EXPECT_EQ(expectedRecords.size(), actualRecords.size()) << message;
    for (const auto& expectedRecordItr : expectedRecords) {
        const auto& expectedStateTrackedObject = expectedRecordItr.first;
        const auto& expectedStateTrackedObjectRecord = expectedRecordItr.second;
        const auto& actualStateTrackedObjectItr = actualRecords.find(expectedStateTrackedObject);
        // TODO : Once tests are passing everywhere, remove this check entirely...
        // if (expectedStateTrackedObject.type != VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
            auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
            if (actualStateTrackedObjectItr == actualRecords.end()) {
                FAIL()
                << "================================================================================" << std::endl
                << message << std::endl
                << "Failed to find expected GvkStateTrackedObject " << gvk::to_string(expectedStateTrackedObject, printerFlags);
            }
            if (expectedStateTrackedObjectRecord != actualStateTrackedObjectItr->second) {
                FAIL()
                << "================================================================================" << std::endl
                << message << std::endl
                << "Mistmatch detected for GvkStateTrackedObject " << gvk::to_string(expectedStateTrackedObject, printerFlags);
            }
        // }
        switch (expectedStateTrackedObject.type) {
        case VK_OBJECT_TYPE_INSTANCE: {
            validate_state_tracked_create_info<VkInstanceCreateInfo>(expectedStateTrackedObjectRecord.mInstanceCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE: {
            // NOOP : No create info to validate
        } break;
        case VK_OBJECT_TYPE_DEVICE: {
            validate_state_tracked_create_info<VkDeviceCreateInfo>(expectedStateTrackedObjectRecord.mDeviceCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_QUEUE: {
            validate_state_tracked_create_info<VkDeviceQueueCreateInfo>(expectedStateTrackedObjectRecord.mDeviceQueueCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_COMMAND_POOL: {
            validate_state_tracked_create_info<VkCommandPoolCreateInfo>(expectedStateTrackedObjectRecord.mCommandPoolCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_COMMAND_BUFFER: {
            validate_state_tracked_create_info<VkCommandBufferAllocateInfo>(expectedStateTrackedObjectRecord.mCommandBufferAllocateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_DEVICE_MEMORY: {
            validate_state_tracked_create_info<VkMemoryAllocateInfo>(expectedStateTrackedObjectRecord.mMemoryAllocateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_BUFFER: {
            validate_state_tracked_create_info<VkBufferCreateInfo>(expectedStateTrackedObjectRecord.mBufferCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_IMAGE: {
            validate_state_tracked_create_info<VkImageCreateInfo>(expectedStateTrackedObjectRecord.mImageCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_IMAGE_VIEW: {
            validate_state_tracked_create_info<VkImageViewCreateInfo>(expectedStateTrackedObjectRecord.mImageViewCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: {
            validate_state_tracked_create_info<VkDescriptorSetLayoutCreateInfo>(expectedStateTrackedObjectRecord.mDescriptorSetLayoutCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_SHADER_MODULE: {
            validate_state_tracked_create_info<VkShaderModuleCreateInfo>(expectedStateTrackedObjectRecord.mShaderModuleCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_RENDER_PASS: {
            validate_state_tracked_create_info<VkRenderPassCreateInfo2>(expectedStateTrackedObjectRecord.mRenderPassCreateInfo2, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT: {
            validate_state_tracked_create_info<VkPipelineLayoutCreateInfo>(expectedStateTrackedObjectRecord.mPipelineLayoutCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_PIPELINE: {
            if (expectedStateTrackedObjectRecord.mComputePipelineCreateInfo->sType == gvk::get_stype<VkComputePipelineCreateInfo>()) {
                validate_state_tracked_create_info<VkComputePipelineCreateInfo>(expectedStateTrackedObjectRecord.mComputePipelineCreateInfo, actualStateTrackedObjectItr->first);
            } else if (expectedStateTrackedObjectRecord.mGraphicsPipelineCreateInfo->sType == gvk::get_stype<VkGraphicsPipelineCreateInfo>()) {
                validate_state_tracked_create_info<VkGraphicsPipelineCreateInfo>(expectedStateTrackedObjectRecord.mGraphicsPipelineCreateInfo, actualStateTrackedObjectItr->first);
            }
        } break;
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL: {
            validate_state_tracked_create_info<VkDescriptorPoolCreateInfo>(expectedStateTrackedObjectRecord.mDescriptorPoolCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET: {
            validate_state_tracked_create_info<VkDescriptorSetAllocateInfo>(expectedStateTrackedObjectRecord.mDescriptorSetAllocateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_FENCE: {
            validate_state_tracked_create_info<VkFenceCreateInfo>(expectedStateTrackedObjectRecord.mFenceCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_SEMAPHORE: {
            validate_state_tracked_create_info<VkSemaphoreCreateInfo>(expectedStateTrackedObjectRecord.mSemaphoreCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_FRAMEBUFFER: {
            validate_state_tracked_create_info<VkFramebufferCreateInfo>(expectedStateTrackedObjectRecord.mFramebufferCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR: {
            validate_state_tracked_create_info<VkSwapchainCreateInfoKHR>(expectedStateTrackedObjectRecord.mSwapchainCreateInfo, actualStateTrackedObjectItr->first);
        } break;
        case VK_OBJECT_TYPE_SURFACE_KHR: {
#ifdef VK_USE_PLATFORM_XLIB_KHR
            validate_state_tracked_create_info<VkXlibSurfaceCreateInfoKHR>(expectedStateTrackedObjectRecord.mXlibSurfaceCreateInfo, actualStateTrackedObjectItr->first);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
            validate_state_tracked_create_info<VkWin32SurfaceCreateInfoKHR>(expectedStateTrackedObjectRecord.mWin32SurfaceCreateInfo, actualStateTrackedObjectItr->first);
#endif
        } break;
        default:
        {
            FAIL() << "Unexpected VkStructureType";
        } break;
        }
    }
}

inline void create_memory_bound_image(const gvk::Context& context, const VkImageCreateInfo& imageCreateInfo, gvk::Image* pImage, gvk::DeviceMemory* pDeviceMemory)
{
    assert(pImage);
    assert(pDeviceMemory);

    ASSERT_EQ(gvk::Image::create(context.get_devices()[0], &imageCreateInfo, (const VkAllocationCallbacks*)nullptr, pImage), VK_SUCCESS);

    auto imageMemoryRequirementsInfo = gvk::get_default<VkImageMemoryRequirementsInfo2>();
    imageMemoryRequirementsInfo.image = *pImage;
    auto memoryRequirements = gvk::get_default<VkMemoryRequirements2>();
    const auto& dispatchTable = pImage->get<gvk::Device>().get<gvk::DispatchTable>();
    assert(dispatchTable.gvkGetImageMemoryRequirements2);
    dispatchTable.gvkGetImageMemoryRequirements2(pImage->get<gvk::Device>(), &imageMemoryRequirementsInfo, &memoryRequirements);
    auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    uint32_t memoryTypeCount = VK_MAX_MEMORY_TYPES;
    std::array<uint32_t, VK_MAX_MEMORY_TYPES> memoryTypeIndices;
    gvk::get_compatible_memory_type_indices(context.get_devices()[0].get<gvk::PhysicalDevice>(), memoryRequirements.memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, memoryTypeIndices.data());
    ASSERT_TRUE(1 <= memoryTypeCount);

    auto memoryAllocateInfo = gvk::get_default<VkMemoryAllocateInfo>();
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndices[0];
    memoryAllocateInfo.allocationSize = memoryRequirements.memoryRequirements.size;
    ASSERT_EQ(gvk::DeviceMemory::allocate(pImage->get<gvk::Device>(), &memoryAllocateInfo, nullptr, pDeviceMemory), VK_SUCCESS);

    auto bindImageMemoryInfo = gvk::get_default<VkBindImageMemoryInfo>();
    bindImageMemoryInfo.image = *pImage;
    bindImageMemoryInfo.memory = *pDeviceMemory;
    assert(dispatchTable.gvkBindImageMemory2);
    ASSERT_EQ(dispatchTable.gvkBindImageMemory2(pImage->get<gvk::Device>(), 1, &bindImageMemoryInfo), VK_SUCCESS);
}

struct RenderTargetValidationCreateInfo
{
    VkExtent2D extent { };
    VkFormat colorFormat { VK_FORMAT_UNDEFINED };
    VkFormat depthFormat { VK_FORMAT_UNDEFINED };
    VkSampleCountFlagBits sampleCount { VK_SAMPLE_COUNT_1_BIT };
};

inline void create_render_target(const gvk::Context& context, const RenderTargetValidationCreateInfo* pCreateInfo, gvk::RenderTarget* pRenderTarget)
{
    assert(pCreateInfo);
    assert(pRenderTarget);

    // MSAA VkAttachmentDescription2 and VkAttachmentReference2
    auto msaaAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
    msaaAttachmentDescription.format = pCreateInfo->colorFormat;
    msaaAttachmentDescription.samples = pCreateInfo->sampleCount;
    msaaAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    msaaAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    msaaAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    auto msaaAttachmentReference = gvk::get_default<VkAttachmentReference2>();
    msaaAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    msaaAttachmentReference.aspectMask = gvk::get_image_aspect_flags(pCreateInfo->colorFormat);

    // Color VkAttachmentDescription2 and VkAttachmentReference2
    auto colorAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
    colorAttachmentDescription.format = msaaAttachmentDescription.format;
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    auto colorAttachmentReference = msaaAttachmentReference;

    // Depth VkAttachmentDescription2 and VkAttachmentReference2
    auto depthAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
    depthAttachmentDescription.format = pCreateInfo->depthFormat;
    depthAttachmentDescription.samples = pCreateInfo->sampleCount;
    depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    auto depthAttachmentReference = gvk::get_default<VkAttachmentReference2>();
    depthAttachmentReference.layout = depthAttachmentDescription.finalLayout;
    depthAttachmentReference.aspectMask = gvk::get_image_aspect_flags(pCreateInfo->depthFormat);

    // Setup attachment descriptions and references
    uint32_t attachmentCount = 1;
    std::array<VkAttachmentDescription2, 3> attachmentDescriptions {
        msaaAttachmentDescription,
        colorAttachmentDescription,
        depthAttachmentDescription,
    };
    auto pAttachmentDescriptions = &attachmentDescriptions[1];
    if (VK_SAMPLE_COUNT_1_BIT < pCreateInfo->sampleCount) {
        pAttachmentDescriptions = &attachmentDescriptions[0];
        colorAttachmentReference.attachment = 1;
        ++attachmentCount;
    }
    if (pCreateInfo->depthFormat) {
        depthAttachmentReference.attachment = colorAttachmentReference.attachment + 1;
        ++attachmentCount;
    }

    // Setup VkSubpassDescription2
    auto subpassDescription = gvk::get_default<VkSubpassDescription2>();
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = VK_SAMPLE_COUNT_1_BIT < pCreateInfo->sampleCount ? &msaaAttachmentReference : &colorAttachmentReference;
    subpassDescription.pResolveAttachments = VK_SAMPLE_COUNT_1_BIT < pCreateInfo->sampleCount ? &colorAttachmentReference : nullptr;
    subpassDescription.pDepthStencilAttachment = depthAttachmentDescription.format ? &depthAttachmentReference : nullptr;

    // Setup VkSubpassDependency2 for a gvk::RenderPass with a single, 1 sample, color attachment
    std::array<VkSubpassDependency2, 2> subpassDependencies{
        VkSubpassDependency2{
            /* .sType           = */ gvk::get_stype<VkSubpassDependency2>(),
            /* .pNext           = */ nullptr,
            /* .srcSubpass      = */ VK_SUBPASS_EXTERNAL,
            /* .dstSubpass      = */ 0,
            /* .srcStageMask    = */ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            /* .dstStageMask    = */ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            /* .srcAccessMask   = */ 0,
            /* .dstAccessMask   = */ VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            /* .dependencyFlags = */ VK_DEPENDENCY_BY_REGION_BIT,
            /* .viewOffset      = */ 0,
        }
    };

    // Setup VkSubpassDependency2 for a gvk::RenderPass with a multisample color attachment and a resolve attachment
    if (VK_SAMPLE_COUNT_1_BIT < pCreateInfo->sampleCount) {
        subpassDependencies = {
            VkSubpassDependency2{
                /* .sType           = */ gvk::get_stype<VkSubpassDependency2>(),
                /* .pNext           = */ nullptr,
                /* .srcSubpass      = */ VK_SUBPASS_EXTERNAL,
                /* .dstSubpass      = */ 0,
                /* .srcStageMask    = */ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                /* .dstStageMask    = */ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                /* .srcAccessMask   = */ VK_ACCESS_MEMORY_READ_BIT,
                /* .dstAccessMask   = */ VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                /* .dependencyFlags = */ VK_DEPENDENCY_BY_REGION_BIT,
                /* .viewOffset      = */ 0,
            },
            VkSubpassDependency2{
                /* .sType           = */ gvk::get_stype<VkSubpassDependency2>(),
                /* .pNext           = */ nullptr,
                /* .srcSubpass      = */ 0,
                /* .dstSubpass      = */ VK_SUBPASS_EXTERNAL,
                /* .srcStageMask    = */ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                /* .dstStageMask    = */ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                /* .srcAccessMask   = */ VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                /* .dstAccessMask   = */ VK_ACCESS_MEMORY_READ_BIT,
                /* .dependencyFlags = */ VK_DEPENDENCY_BY_REGION_BIT,
                /* .viewOffset      = */ 0,
            }
        };
    }

    // Create gvk::RenderPass
    auto renderPassCreateInfo = gvk::get_default<VkRenderPassCreateInfo2>();
    renderPassCreateInfo.attachmentCount = attachmentCount;
    renderPassCreateInfo.pAttachments = pAttachmentDescriptions;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = VK_SAMPLE_COUNT_1_BIT < pCreateInfo->sampleCount ? 2 : 1;
    renderPassCreateInfo.pDependencies = subpassDependencies.data();
    gvk::RenderPass renderPass;
    ASSERT_EQ(gvk::RenderPass::create(context.get_devices()[0], &renderPassCreateInfo, nullptr, &renderPass), VK_SUCCESS);

    // Prepare VkFramebufferCreateInfo
    auto framebufferCreateInfo = gvk::get_default<VkFramebufferCreateInfo>();
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.width = pCreateInfo->extent.width;
    framebufferCreateInfo.height = pCreateInfo->extent.height;

    // Create gvk::RenderTarget
    auto renderTargetCreateInfo = gvk::get_default<gvk::RenderTarget::CreateInfo>();
    renderTargetCreateInfo.pFramebufferCreateInfo = &framebufferCreateInfo;
    ASSERT_EQ(gvk::RenderTarget::create(context.get_devices()[0], &renderTargetCreateInfo, nullptr, pRenderTarget), VK_SUCCESS);
}
