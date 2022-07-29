
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#include "gvk/detail/handle-utilities.hpp"
#include "gvk/generated/dispatch-table.hpp"
#include "gvk/generated/handles.hpp"
#include "gvk/structures.hpp"
#include "gvk/handles.hpp"

#include <cassert>

namespace gvk {

const QueueFamily& get_queue_family(const Device& device, uint32_t queueFamilyIndex)
{
    for (const auto& queueFamily : device.get<QueueFamilies>()) {
        if (queueFamily.index == queueFamilyIndex) {
            return queueFamily;
        }
    }
    static const QueueFamily sEmptyQueueFamily;
    return sEmptyQueueFamily;
}

VkResult Buffer::create(const Device& device, const VkBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, Buffer* pBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (device && pBufferCreateInfo && pAllocationCreateInfo && pBuffer) {
            pBuffer->reset();
            VkBuffer vkBuffer = VK_NULL_HANDLE;
            VmaAllocation vmaAllocation = VK_NULL_HANDLE;
            gvk_result(vmaCreateBuffer(device.get<VmaAllocator>(), pBufferCreateInfo, pAllocationCreateInfo, &vkBuffer, &vmaAllocation, nullptr));
            pBuffer->mVkBuffer = vkBuffer;
            pBuffer->mControlBlock.reset(detail::newref, vkBuffer);
            auto& controlBlock = pBuffer->mControlBlock.get_obj();
            controlBlock.mVkBuffer = vkBuffer;
            controlBlock.mDevice = device;
            controlBlock.mBufferCreateInfo = *pBufferCreateInfo;
            controlBlock.mVmaAllocation = vmaAllocation;
            gvk_result(detail::initialize_control_block(controlBlock));
        }
    } gvk_result_scope_end
    return gvkResult;
}

VkResult Image::create(const Device& device, const VkImageCreateInfo* pImageCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, Image* pImage)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (device && pImageCreateInfo && pAllocationCreateInfo && pImage) {
            pImage->reset();
            VkImage vkImage = VK_NULL_HANDLE;
            VmaAllocation vmaAllocation = VK_NULL_HANDLE;
            gvk_result(vmaCreateImage(device.get<VmaAllocator>(), pImageCreateInfo, pAllocationCreateInfo, &vkImage, &vmaAllocation, nullptr));
            pImage->mVkImage = vkImage;
            pImage->mControlBlock.reset(detail::newref, vkImage);
            auto& controlBlock = pImage->mControlBlock.get_obj();
            controlBlock.mVkImage = vkImage;
            controlBlock.mDevice = device;
            controlBlock.mImageCreateInfo = *pImageCreateInfo;
            controlBlock.mVmaAllocation = vmaAllocation;
            gvk_result(detail::initialize_control_block(controlBlock));
        }
    } gvk_result_scope_end
    return gvkResult;
}

namespace detail {

void* get_transient_storage(size_t size)
{
    thread_local std::vector<uint8_t> tlTransientStorage;
    tlTransientStorage.clear();
    tlTransientStorage.resize(size);
    return tlTransientStorage.data();
}

template <>
VkResult initialize_control_block<DeviceControlBlock>(DeviceControlBlock& controlBlock)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        const auto& deviceCreateInfo = *controlBlock.mDeviceCreateInfo;
        for (uint32_t queueCreateInfo_i = 0; queueCreateInfo_i < deviceCreateInfo.queueCreateInfoCount; ++queueCreateInfo_i) {
            const auto& deviceQueueCreateInfo = deviceCreateInfo.pQueueCreateInfos[queueCreateInfo_i];
            QueueFamily queueFamily{ };
            queueFamily.index = deviceQueueCreateInfo.queueFamilyIndex;
            queueFamily.queues.reserve(deviceQueueCreateInfo.queueCount);
            for (uint32_t queue_i = 0; queue_i < deviceQueueCreateInfo.queueCount; ++queue_i) {
                Queue queue;
                assert(gDispatchTable.gvkGetDeviceQueue);
                gDispatchTable.gvkGetDeviceQueue(controlBlock.mVkDevice, deviceQueueCreateInfo.queueFamilyIndex, queue_i, &queue.mVkQueue);
                queue.mControlBlock.reset(newref, queue.mVkQueue);
                auto& queueControlBlock = queue.mControlBlock.get_obj();
                queueControlBlock.mVkQueue = queue.mVkQueue;
                queueControlBlock.mVkDevice = controlBlock.mVkDevice;
                queueControlBlock.mDeviceQueueCreateInfo = deviceQueueCreateInfo;
                gvk_result(detail::initialize_control_block(queueControlBlock));
                queueFamily.queues.push_back(queue);
            }
            controlBlock.mQueueFamilies.push_back(queueFamily);
        }
        controlBlock.mInstance = controlBlock.mPhysicalDevice.get<Instance>();
        const auto& instanceCreateInfo = controlBlock.mInstance.get<VkInstanceCreateInfo>();

        // TODO : We need to *not* be loading VkDevice entry points here so that we can
        //  correctly support multi device workloads...probably Instance and Device end
        //  up with DispatchTable members...however that ends up happening, we need to
        //  keep VMA hooked up to GVK's dispatch table(s)...not super high priority atm.
        gvk::DispatchTable::load_device_entry_points(controlBlock.mVkDevice, &gvk::gDispatchTable);

        VmaVulkanFunctions vulkanFunctions{ };
        vulkanFunctions.vkGetInstanceProcAddr = gDispatchTable.gvkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = gDispatchTable.gvkGetDeviceProcAddr;
        vulkanFunctions.vkGetPhysicalDeviceProperties = gDispatchTable.gvkGetPhysicalDeviceProperties;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = gDispatchTable.gvkGetPhysicalDeviceMemoryProperties;
        vulkanFunctions.vkAllocateMemory = gDispatchTable.gvkAllocateMemory;
        vulkanFunctions.vkFreeMemory = gDispatchTable.gvkFreeMemory;
        vulkanFunctions.vkMapMemory = gDispatchTable.gvkMapMemory;
        vulkanFunctions.vkUnmapMemory = gDispatchTable.gvkUnmapMemory;
        vulkanFunctions.vkFlushMappedMemoryRanges = gDispatchTable.gvkFlushMappedMemoryRanges;
        vulkanFunctions.vkInvalidateMappedMemoryRanges = gDispatchTable.gvkInvalidateMappedMemoryRanges;
        vulkanFunctions.vkBindBufferMemory = gDispatchTable.gvkBindBufferMemory;
        vulkanFunctions.vkBindImageMemory = gDispatchTable.gvkBindImageMemory;
        vulkanFunctions.vkGetBufferMemoryRequirements = gDispatchTable.gvkGetBufferMemoryRequirements;
        vulkanFunctions.vkGetImageMemoryRequirements = gDispatchTable.gvkGetImageMemoryRequirements;
        vulkanFunctions.vkCreateBuffer = gDispatchTable.gvkCreateBuffer;
        vulkanFunctions.vkDestroyBuffer = gDispatchTable.gvkDestroyBuffer;
        vulkanFunctions.vkCreateImage = gDispatchTable.gvkCreateImage;
        vulkanFunctions.vkDestroyImage = gDispatchTable.gvkDestroyImage;
        vulkanFunctions.vkCmdCopyBuffer = gDispatchTable.gvkCmdCopyBuffer;
        #if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
        /// Fetch "vkGetBufferMemoryRequirements2" on Vulkan >= 1.1, fetch "vkGetBufferMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
        vulkanFunctions.vkGetBufferMemoryRequirements2KHR = gDispatchTable.gvkGetBufferMemoryRequirements2;
        /// Fetch "vkGetImageMemoryRequirements2" on Vulkan >= 1.1, fetch "vkGetImageMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
        vulkanFunctions.vkGetImageMemoryRequirements2KHR = gDispatchTable.gvkGetImageMemoryRequirements2;
        #endif
        #if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
        /// Fetch "vkBindBufferMemory2" on Vulkan >= 1.1, fetch "vkBindBufferMemory2KHR" when using VK_KHR_bind_memory2 extension.
        vulkanFunctions.vkBindBufferMemory2KHR = gDispatchTable.gvkBindBufferMemory2;
        /// Fetch "vkBindImageMemory2" on Vulkan >= 1.1, fetch "vkBindImageMemory2KHR" when using VK_KHR_bind_memory2 extension.
        vulkanFunctions.vkBindImageMemory2KHR = gDispatchTable.gvkBindImageMemory2;
        #endif
        #if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = gDispatchTable.gvkGetPhysicalDeviceMemoryProperties2;
        #endif
        #if VMA_VULKAN_VERSION >= 1003000
        /// Fetch from "vkGetDeviceBufferMemoryRequirements" on Vulkan >= 1.3, but you can also fetch it from "vkGetDeviceBufferMemoryRequirementsKHR" if you enabled extension VK_KHR_maintenance4.
        vulkanFunctions.vkGetDeviceBufferMemoryRequirements = gDispatchTable.gvkGetDeviceBufferMemoryRequirements;
        /// Fetch from "vkGetDeviceImageMemoryRequirements" on Vulkan >= 1.3, but you can also fetch it from "vkGetDeviceImageMemoryRequirementsKHR" if you enabled extension VK_KHR_maintenance4.
        vulkanFunctions.vkGetDeviceImageMemoryRequirements = gDispatchTable.gvkGetDeviceImageMemoryRequirements;
        #endif

        VmaAllocatorCreateInfo allocatorCreateInfo{ };
        allocatorCreateInfo.vulkanApiVersion = instanceCreateInfo.pApplicationInfo ? instanceCreateInfo.pApplicationInfo->apiVersion : VK_API_VERSION_1_3;
        allocatorCreateInfo.instance = controlBlock.mInstance;
        allocatorCreateInfo.physicalDevice = controlBlock.mPhysicalDevice;
        allocatorCreateInfo.device = controlBlock.mVkDevice;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
        gvk_result(vmaCreateAllocator(&allocatorCreateInfo, &controlBlock.mVmaAllocator));
    } gvk_result_scope_end
    return gvkResult;
}

template <>
VkResult initialize_control_block<FramebufferControlBlock>(FramebufferControlBlock& controlBlock)
{
    const auto& framebufferCreateInfo = *controlBlock.mFramebufferCreateInfo;
    if (framebufferCreateInfo.attachmentCount && framebufferCreateInfo.pAttachments) {
        controlBlock.mImageViews.reserve(framebufferCreateInfo.attachmentCount);
        for (uint32_t i = 0; i < framebufferCreateInfo.attachmentCount; ++i) {
            ImageView imageView = framebufferCreateInfo.pAttachments[i];
            if (imageView) {
                controlBlock.mImageViews.push_back(imageView);
            }
        }
    }
    return VK_SUCCESS;
}

template <>
VkResult initialize_control_block<InstanceControlBlock>(InstanceControlBlock& controlBlock)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto vkInstance = controlBlock.mVkInstance;
        gvk::DispatchTable::load_instance_entry_points(vkInstance, &gDispatchTable);
        uint32_t physicalDeviceCount = 0;
        assert(gDispatchTable.gvkEnumeratePhysicalDevices);
        gvk_result(gDispatchTable.gvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr));
        auto pVkPhysicalDevices = (VkPhysicalDevice*)detail::get_transient_storage(physicalDeviceCount * sizeof(VkPhysicalDevice));
        std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount);
        gvk_result(gDispatchTable.gvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, pVkPhysicalDevices));
        controlBlock.mPhysicalDevices.reserve(physicalDeviceCount);
        for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
            PhysicalDevice physicalDevice;
            physicalDevice.mVkPhysicalDevice = pVkPhysicalDevices[i];
            physicalDevice.mControlBlock.reset(newref, pVkPhysicalDevices[i]);
            auto& physicalDeviceControlBlock = physicalDevice.mControlBlock.get_obj();
            physicalDeviceControlBlock.mVkPhysicalDevice = pVkPhysicalDevices[i];
            physicalDeviceControlBlock.mVkInstance = vkInstance;
            controlBlock.mPhysicalDevices.push_back(physicalDevice);
            gvk_result(detail::initialize_control_block(physicalDeviceControlBlock));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

template <>
VkResult initialize_control_block<PipelineLayoutControlBlock>(PipelineLayoutControlBlock& controlBlock)
{
    const auto& pipelineLayoutCreateInfo = *controlBlock.mPipelineLayoutCreateInfo;
    if (pipelineLayoutCreateInfo.setLayoutCount && pipelineLayoutCreateInfo.pSetLayouts) {
        controlBlock.mDescriptorSetLayouts.resize(pipelineLayoutCreateInfo.setLayoutCount);
        for (uint32_t i = 0; i < pipelineLayoutCreateInfo.setLayoutCount; ++i) {
            controlBlock.mDescriptorSetLayouts[i] = pipelineLayoutCreateInfo.pSetLayouts[i];
        }
    }
    return VK_SUCCESS;
}

template <>
VkResult initialize_control_block<RenderPassControlBlock>(RenderPassControlBlock& controlBlock)
{
    if (controlBlock.mRenderPassCreateInfo->sType == VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO) {
        controlBlock.mRenderPassCreateInfo2 = convert<VkRenderPassCreateInfo, VkRenderPassCreateInfo2>(controlBlock.mRenderPassCreateInfo);
    } else if (controlBlock.mRenderPassCreateInfo2->sType == VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2) {
        controlBlock.mRenderPassCreateInfo = convert<VkRenderPassCreateInfo2, VkRenderPassCreateInfo>(controlBlock.mRenderPassCreateInfo2);
    }
    return VK_SUCCESS;
}

template <>
VkResult initialize_control_block<SwapchainKHRControlBlock>(SwapchainKHRControlBlock& controlBlock)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        uint32_t swapchainImageCount = 0;
        assert(gDispatchTable.gvkGetSwapchainImagesKHR);
        gvk_result(gDispatchTable.gvkGetSwapchainImagesKHR(controlBlock.mDevice, controlBlock.mVkSwapchainKHR, &swapchainImageCount, nullptr));
        auto pVkImages = (VkImage*)detail::get_transient_storage(swapchainImageCount * sizeof(VkImage));
        gvk_result(gDispatchTable.gvkGetSwapchainImagesKHR(controlBlock.mDevice, controlBlock.mVkSwapchainKHR, &swapchainImageCount, pVkImages));
        auto& images = controlBlock.mImages;
        images.resize(swapchainImageCount);
        const auto& swapchainCreateInfo = *controlBlock.mSwapchainCreateInfoKHR;
        auto imageCreateInfo = get_default<VkImageCreateInfo>();
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = swapchainCreateInfo.imageFormat;
        imageCreateInfo.extent.width = swapchainCreateInfo.imageExtent.width;
        imageCreateInfo.extent.height = swapchainCreateInfo.imageExtent.height;
        imageCreateInfo.arrayLayers = swapchainCreateInfo.imageArrayLayers;
        imageCreateInfo.usage = swapchainCreateInfo.imageUsage;
        imageCreateInfo.sharingMode = swapchainCreateInfo.imageSharingMode;
        imageCreateInfo.queueFamilyIndexCount = swapchainCreateInfo.queueFamilyIndexCount;
        imageCreateInfo.pQueueFamilyIndices = swapchainCreateInfo.pQueueFamilyIndices;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        for (uint32_t i = 0; i < swapchainImageCount; ++i) {
            images[i].mVkImage = pVkImages[i];
            images[i].mControlBlock.reset(newref, pVkImages[i]);
            auto& imageControlBlock = images[i].mControlBlock.get_obj();
            imageControlBlock.mVkImage = pVkImages[i];
            imageControlBlock.mDevice = controlBlock.mDevice;
            imageControlBlock.mVkSwapchainKHR = controlBlock.mVkSwapchainKHR;
            imageControlBlock.mImageCreateInfo = imageCreateInfo;
            gvk_result(detail::initialize_control_block(imageControlBlock));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

BufferControlBlock::~BufferControlBlock()
{
    if (mVmaAllocation) {
        vmaDestroyBuffer(mDevice.get<VmaAllocator>(), mVkBuffer, mVmaAllocation);
    } else {
        assert(gDispatchTable.gvkDestroyBuffer);
        gDispatchTable.gvkDestroyBuffer(mDevice, mVkBuffer, (mAllocator.pfnFree ? &mAllocator : nullptr));
    }
}

DeviceControlBlock::~DeviceControlBlock()
{
    if (mVmaAllocator) {
        vmaDestroyAllocator(mVmaAllocator);
    }
    assert(gDispatchTable.gvkDeviceWaitIdle);
    gDispatchTable.gvkDeviceWaitIdle(mVkDevice);
    assert(gDispatchTable.gvkDestroyDevice);
    gDispatchTable.gvkDestroyDevice(mVkDevice, (mAllocator.pfnFree ? &mAllocator : nullptr));
}

ImageControlBlock::~ImageControlBlock()
{
    if (!mVkSwapchainKHR) {
        if (mVmaAllocation) {
            vmaDestroyImage(mDevice.get<VmaAllocator>(), mVkImage, mVmaAllocation);
        } else {
            assert(gDispatchTable.gvkDestroyImage);
            gDispatchTable.gvkDestroyImage(mDevice, mVkImage, (mAllocator.pfnFree ? &mAllocator : nullptr));
        }
    }
}

} // namespace detail
} // namespace gvk
