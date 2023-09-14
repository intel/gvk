
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

#include "gvk-dispatch-table.hpp"
#include "gvk-handles/detail/handle-utilities.hpp"
#include "gvk-handles/generated/handles.hpp"
#include "gvk-handles/handles.hpp"
#include "gvk-structures/defaults.hpp"

#ifdef GVK_COMPILER_MSVC
#pragma warning(push, 0)
#endif // GVK_COMPILER_MSVC
#ifdef GVK_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif // GVK_COMPILER_GCC
#ifdef GVK_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wnullability-extension"
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif // GVK_COMPILER_CLANG

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#ifdef GVK_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // GVK_COMPILER_CLANG
#ifdef GVK_COMPILER_GCC
#pragma GCC diagnostic pop
#endif // GVK_COMPILER_GCC
#ifdef GVK_COMPILER_MSVC
#pragma warning(pop)
#endif // GVK_COMPILER_MSVC

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

VkResult Instance::create_unmanaged(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, const DispatchTable* pDispatchTable, VkInstance vkInstance, Instance* pGvkInstance)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        assert(pCreateInfo);
        assert(pDispatchTable);
        assert(vkInstance);
        assert(pGvkInstance);
        *pGvkInstance = vkInstance;
        if (!*pGvkInstance) {
            pGvkInstance->mReference.reset(gvk::newref, vkInstance);
            auto& controlBlock = pGvkInstance->mReference.get_obj();
            controlBlock.mVkInstance = vkInstance;
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mInstanceCreateInfo = *pCreateInfo;
            controlBlock.mUnmanaged = true;
            controlBlock.mDispatchTable = *pDispatchTable;
            gvk_result(gvk::detail::initialize_control_block(*pGvkInstance));
        } else {
            gvkResult = VK_SUCCESS;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Device::create_unmanaged(const PhysicalDevice& physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, const DispatchTable* pDispatchTable, VkDevice vkDevice, Device* pGvkDevice)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        assert(pCreateInfo);
        assert(pDispatchTable);
        assert(vkDevice);
        assert(pGvkDevice);
        *pGvkDevice = vkDevice;
        if (!*pGvkDevice) {
            pGvkDevice->mReference.reset(gvk::newref, vkDevice);
            auto& controlBlock = pGvkDevice->mReference.get_obj();
            controlBlock.mVkDevice = vkDevice;
            controlBlock.mPhysicalDevice = physicalDevice;
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mDeviceCreateInfo = *pCreateInfo;
            controlBlock.mUnmanaged = true;
            controlBlock.mDispatchTable = *pDispatchTable;
            gvk_result(gvk::detail::initialize_control_block(*pGvkDevice));
        } else {
            gvkResult = VK_SUCCESS;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Buffer::create(const Device& device, const VkBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, Buffer* pBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (device && pBufferCreateInfo && pAllocationCreateInfo && pBuffer) {
            pBuffer->reset();
            VkBuffer vkBuffer = VK_NULL_HANDLE;
            VmaAllocation vmaAllocation = VK_NULL_HANDLE;
            gvk_result(vmaCreateBuffer(device.get<VmaAllocator>(), pBufferCreateInfo, pAllocationCreateInfo, &vkBuffer, &vmaAllocation, nullptr));
            pBuffer->mReference.reset(newref, { device, vkBuffer });
            auto& bufferControlBlock = pBuffer->mReference.get_obj();
            bufferControlBlock.mVkBuffer = vkBuffer;
            bufferControlBlock.mDevice = device;
            bufferControlBlock.mBufferCreateInfo = *pBufferCreateInfo;
            bufferControlBlock.mVmaAllocation = vmaAllocation;
            bufferControlBlock.mVmaAllocationCreateInfo = *pAllocationCreateInfo;
            gvk_result(detail::initialize_control_block(bufferControlBlock));
        }
    } gvk_result_scope_end;
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
            pImage->mReference.reset(newref, { device, vkImage });
            auto& imageControlBlock = pImage->mReference.get_obj();
            imageControlBlock.mVkImage = vkImage;
            imageControlBlock.mDevice = device;
            imageControlBlock.mImageCreateInfo = *pImageCreateInfo;
            imageControlBlock.mVmaAllocation = vmaAllocation;
            imageControlBlock.mVmaAllocationCreateInfo = *pAllocationCreateInfo;
            gvk_result(detail::initialize_control_block(imageControlBlock));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult DeferredOperationKHR::create(const Device& device, const VkAllocationCallbacks* pAllocator, DeferredOperationKHR* pDeferredOperation)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        assert(pDeferredOperation);
        const auto& dispatchTable = device.get<DispatchTable>();
        assert(dispatchTable.gvkCreateDeferredOperationKHR);
        VkDeferredOperationKHR vkDeferredOperationKHR = VK_NULL_HANDLE;
        auto pVkDeferredOperationKHR = &vkDeferredOperationKHR;
        gvk_result(dispatchTable.gvkCreateDeferredOperationKHR(device, pAllocator, pVkDeferredOperationKHR));
        pDeferredOperation->mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkDeferredOperationKHR>(device, *pVkDeferredOperationKHR));
        auto& controlBlock = pDeferredOperation->mReference.get_obj();
        controlBlock.mVkDeferredOperationKHR = *pVkDeferredOperationKHR;
        controlBlock.mDevice = device;
        controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
        gvk_result(gvk::detail::initialize_control_block(*pDeferredOperation));
    } gvk_result_scope_end;
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
VkResult initialize_control_block<Instance>(Instance& instance)
{
    auto& instanceControlBlock = instance.mReference.get_obj();
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (!instanceControlBlock.mUnmanaged) {
            DispatchTable::load_global_entry_points(&instanceControlBlock.mDispatchTable);
            DispatchTable::load_instance_entry_points(instanceControlBlock.mVkInstance, &instanceControlBlock.mDispatchTable);
        }
        uint32_t physicalDeviceCount = 0;
        assert(instanceControlBlock.mDispatchTable.gvkEnumeratePhysicalDevices);
        gvk_result(instanceControlBlock.mDispatchTable.gvkEnumeratePhysicalDevices(instanceControlBlock.mVkInstance, &physicalDeviceCount, nullptr));
        auto pVkPhysicalDevices = (VkPhysicalDevice*)detail::get_transient_storage(physicalDeviceCount * sizeof(VkPhysicalDevice));
        std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount);
        gvk_result(instanceControlBlock.mDispatchTable.gvkEnumeratePhysicalDevices(instanceControlBlock.mVkInstance, &physicalDeviceCount, pVkPhysicalDevices));
        instanceControlBlock.mPhysicalDevices.resize(physicalDeviceCount);
        for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
            instanceControlBlock.mPhysicalDevices[i].mReference.reset(newref, pVkPhysicalDevices[i]);
            auto& physicalDeviceControlBlock = instanceControlBlock.mPhysicalDevices[i].mReference.get_obj();
            physicalDeviceControlBlock.mVkPhysicalDevice = pVkPhysicalDevices[i];
            physicalDeviceControlBlock.mVkInstance = instanceControlBlock.mVkInstance;
            physicalDeviceControlBlock.mDispatchTable = instanceControlBlock.mDispatchTable;
            gvk_result(detail::initialize_control_block(physicalDeviceControlBlock));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

template <>
VkResult initialize_control_block<Device>(Device& device)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto& deviceControlBlock = device.mReference.get_obj();
        deviceControlBlock.mInstance = deviceControlBlock.mPhysicalDevice.get<VkInstance>();
        const auto& physicalDeviceDispatchTable = deviceControlBlock.mPhysicalDevice.get<DispatchTable>();
        deviceControlBlock.mDispatchTable.gvkGetDeviceProcAddr = physicalDeviceDispatchTable.gvkGetDeviceProcAddr;
        if (!deviceControlBlock.mUnmanaged) {
            DispatchTable::load_device_entry_points(deviceControlBlock.mVkDevice, &deviceControlBlock.mDispatchTable);
        }
        const auto& deviceCreateInfo = *deviceControlBlock.mDeviceCreateInfo;
        for (uint32_t queueCreateInfo_i = 0; queueCreateInfo_i < deviceCreateInfo.queueCreateInfoCount; ++queueCreateInfo_i) {
            const auto& deviceQueueCreateInfo = deviceCreateInfo.pQueueCreateInfos[queueCreateInfo_i];
            QueueFamily queueFamily { };
            queueFamily.index = deviceQueueCreateInfo.queueFamilyIndex;
            queueFamily.queues.resize(deviceQueueCreateInfo.queueCount);
            for (uint32_t queue_i = 0; queue_i < deviceQueueCreateInfo.queueCount; ++queue_i) {
                VkQueue vkQueue = VK_NULL_HANDLE;
                assert(deviceControlBlock.mDispatchTable.gvkGetDeviceQueue);
                deviceControlBlock.mDispatchTable.gvkGetDeviceQueue(deviceControlBlock.mVkDevice, deviceQueueCreateInfo.queueFamilyIndex, queue_i, &vkQueue);
                queueFamily.queues[queue_i].mReference.reset(newref, vkQueue);
                auto& queueControlBlock = queueFamily.queues[queue_i].mReference.get_obj();
                queueControlBlock.mVkQueue = vkQueue;
                queueControlBlock.mVkDevice = deviceControlBlock.mVkDevice;
                queueControlBlock.mDeviceQueueCreateInfo = deviceQueueCreateInfo;
                queueControlBlock.mDispatchTable = deviceControlBlock.mDispatchTable;
                gvk_result(detail::initialize_control_block(queueControlBlock));
            }
            deviceControlBlock.mQueueFamilies.push_back(queueFamily);
        }

        VmaVulkanFunctions vulkanFunctions { };
        vulkanFunctions.vkGetInstanceProcAddr = deviceControlBlock.mInstance.get<DispatchTable>().gvkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = deviceControlBlock.mDispatchTable.gvkGetDeviceProcAddr;
        vulkanFunctions.vkGetPhysicalDeviceProperties = physicalDeviceDispatchTable.gvkGetPhysicalDeviceProperties;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = physicalDeviceDispatchTable.gvkGetPhysicalDeviceMemoryProperties;
        vulkanFunctions.vkAllocateMemory = deviceControlBlock.mDispatchTable.gvkAllocateMemory;
        vulkanFunctions.vkFreeMemory = deviceControlBlock.mDispatchTable.gvkFreeMemory;
        vulkanFunctions.vkMapMemory = deviceControlBlock.mDispatchTable.gvkMapMemory;
        vulkanFunctions.vkUnmapMemory = deviceControlBlock.mDispatchTable.gvkUnmapMemory;
        vulkanFunctions.vkFlushMappedMemoryRanges = deviceControlBlock.mDispatchTable.gvkFlushMappedMemoryRanges;
        vulkanFunctions.vkInvalidateMappedMemoryRanges = deviceControlBlock.mDispatchTable.gvkInvalidateMappedMemoryRanges;
        vulkanFunctions.vkBindBufferMemory = deviceControlBlock.mDispatchTable.gvkBindBufferMemory;
        vulkanFunctions.vkBindImageMemory = deviceControlBlock.mDispatchTable.gvkBindImageMemory;
        vulkanFunctions.vkGetBufferMemoryRequirements = deviceControlBlock.mDispatchTable.gvkGetBufferMemoryRequirements;
        vulkanFunctions.vkGetImageMemoryRequirements = deviceControlBlock.mDispatchTable.gvkGetImageMemoryRequirements;
        vulkanFunctions.vkCreateBuffer = deviceControlBlock.mDispatchTable.gvkCreateBuffer;
        vulkanFunctions.vkDestroyBuffer = deviceControlBlock.mDispatchTable.gvkDestroyBuffer;
        vulkanFunctions.vkCreateImage = deviceControlBlock.mDispatchTable.gvkCreateImage;
        vulkanFunctions.vkDestroyImage = deviceControlBlock.mDispatchTable.gvkDestroyImage;
        vulkanFunctions.vkCmdCopyBuffer = deviceControlBlock.mDispatchTable.gvkCmdCopyBuffer;
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
        /// Fetch "vkGetBufferMemoryRequirements2" on Vulkan >= 1.1, fetch "vkGetBufferMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
        vulkanFunctions.vkGetBufferMemoryRequirements2KHR = deviceControlBlock.mDispatchTable.gvkGetBufferMemoryRequirements2;
        /// Fetch "vkGetImageMemoryRequirements2" on Vulkan >= 1.1, fetch "vkGetImageMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
        vulkanFunctions.vkGetImageMemoryRequirements2KHR = deviceControlBlock.mDispatchTable.gvkGetImageMemoryRequirements2;
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
        /// Fetch "vkBindBufferMemory2" on Vulkan >= 1.1, fetch "vkBindBufferMemory2KHR" when using VK_KHR_bind_memory2 extension.
        vulkanFunctions.vkBindBufferMemory2KHR = deviceControlBlock.mDispatchTable.gvkBindBufferMemory2;
        /// Fetch "vkBindImageMemory2" on Vulkan >= 1.1, fetch "vkBindImageMemory2KHR" when using VK_KHR_bind_memory2 extension.
        vulkanFunctions.vkBindImageMemory2KHR = deviceControlBlock.mDispatchTable.gvkBindImageMemory2;
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = physicalDeviceDispatchTable.gvkGetPhysicalDeviceMemoryProperties2;
#endif
#if VMA_VULKAN_VERSION >= 1003000
        /// Fetch from "vkGetDeviceBufferMemoryRequirements" on Vulkan >= 1.3, but you can also fetch it from "vkGetDeviceBufferMemoryRequirementsKHR" if you enabled extension VK_KHR_maintenance4.
        vulkanFunctions.vkGetDeviceBufferMemoryRequirements = deviceControlBlock.mDispatchTable.gvkGetDeviceBufferMemoryRequirements;
        /// Fetch from "vkGetDeviceImageMemoryRequirements" on Vulkan >= 1.3, but you can also fetch it from "vkGetDeviceImageMemoryRequirementsKHR" if you enabled extension VK_KHR_maintenance4.
        vulkanFunctions.vkGetDeviceImageMemoryRequirements = deviceControlBlock.mDispatchTable.gvkGetDeviceImageMemoryRequirements;
#endif

        const auto& instanceCreateInfo = deviceControlBlock.mInstance.get<VkInstanceCreateInfo>();
        VmaAllocatorCreateInfo allocatorCreateInfo { };
        allocatorCreateInfo.vulkanApiVersion = instanceCreateInfo.pApplicationInfo ? instanceCreateInfo.pApplicationInfo->apiVersion : VK_API_VERSION_1_3;
        allocatorCreateInfo.instance = deviceControlBlock.mInstance;
        allocatorCreateInfo.physicalDevice = deviceControlBlock.mPhysicalDevice;
        allocatorCreateInfo.device = deviceControlBlock.mVkDevice;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
        gvk_result(vmaCreateAllocator(&allocatorCreateInfo, &deviceControlBlock.mVmaAllocator));
    } gvk_result_scope_end;
    return gvkResult;
}

template <>
VkResult initialize_control_block<CommandBuffer>(CommandBuffer& commandBuffer)
{
    auto& commandBufferControlBlock = commandBuffer.mReference.get_obj();
    // NOTE : This initializes the command buffer's dispatch table.  This is only
    //  actually necessary for command buffers that are created within a layer, but
    //  it shouldn't hurt to do unconditionally since the command buffer dispatch
    //  table should should already be set to the device's anyway...if this
    //  assumption causes a problem there'll need to be a check for whether or not
    //  this code is executing from within a layer.
    // NOTE : See the following for more info regarding dispatchable handles...
    //  https://vulkan.lunarg.com/doc/view/latest/linux/vkspec.html#fundamentals-objectmodel-overview
    //  https://renderdoc.org/vulkan-layer-guide.html
    *(void**)commandBufferControlBlock.mVkCommandBuffer = *(void**)commandBufferControlBlock.mDevice.get<VkDevice>();
    return VK_SUCCESS;
}

template <>
VkResult initialize_control_block<Framebuffer>(Framebuffer& framebuffer)
{
    auto& framebufferControlBlock = framebuffer.mReference.get_obj();
    const auto& framebufferCreateInfo = *framebufferControlBlock.mFramebufferCreateInfo;
    if (framebufferCreateInfo.attachmentCount && framebufferCreateInfo.pAttachments) {
        framebufferControlBlock.mImageViews.reserve(framebufferCreateInfo.attachmentCount);
        for (uint32_t i = 0; i < framebufferCreateInfo.attachmentCount; ++i) {
            ImageView imageView = HandleId<VkDevice, VkImageView>(framebufferControlBlock.mDevice, framebufferCreateInfo.pAttachments[i]);
            if (imageView) {
                framebufferControlBlock.mImageViews.push_back(imageView);
            }
        }
    }
    return VK_SUCCESS;
}

template <>
VkResult initialize_control_block<PipelineLayout>(PipelineLayout& pipelineLayout)
{
    auto& pipelineLayoutControlBlock = pipelineLayout.mReference.get_obj();
    const auto& pipelineLayoutCreateInfo = *pipelineLayoutControlBlock.mPipelineLayoutCreateInfo;
    if (pipelineLayoutCreateInfo.setLayoutCount && pipelineLayoutCreateInfo.pSetLayouts) {
        pipelineLayoutControlBlock.mDescriptorSetLayouts.resize(pipelineLayoutCreateInfo.setLayoutCount);
        for (uint32_t i = 0; i < pipelineLayoutCreateInfo.setLayoutCount; ++i) {
            pipelineLayoutControlBlock.mDescriptorSetLayouts[i] = HandleId<VkDevice, VkDescriptorSetLayout>(pipelineLayoutControlBlock.mDevice, pipelineLayoutCreateInfo.pSetLayouts[i]);
        }
    }
    return VK_SUCCESS;
}

template <>
VkResult initialize_control_block<ShaderEXT>(ShaderEXT& shader)
{
    auto& shaderControlBlock = shader.mReference.get_obj();
    const VkShaderCreateInfoEXT& shaderCreateInfo = shaderControlBlock.mShaderCreateInfoEXT;
    if (shaderCreateInfo.setLayoutCount && shaderCreateInfo.pSetLayouts) {
        shaderControlBlock.mDescriptorSetLayouts.resize(shaderCreateInfo.setLayoutCount);
        for (uint32_t i = 0; i < shaderCreateInfo.setLayoutCount; ++i) {
            shaderControlBlock.mDescriptorSetLayouts[i] = HandleId<VkDevice, VkDescriptorSetLayout>(shaderControlBlock.mDevice, shaderCreateInfo.pSetLayouts[i]);
        }
    }
    return VK_SUCCESS;
}

template <>
VkResult initialize_control_block<SurfaceKHR>(SurfaceKHR& surface)
{
    auto& surfaceControlBlock = surface.mReference.get_obj();
    const VkDisplaySurfaceCreateInfoKHR& displaySurfaceCreateInfo = surfaceControlBlock.mDisplaySurfaceCreateInfoKHR;
    if (displaySurfaceCreateInfo.sType == VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR) {
        for (const auto& physicalDevice : surfaceControlBlock.mInstance.get<PhysicalDevices>()) {
            surfaceControlBlock.mDisplayModeKHR = HandleId<VkPhysicalDevice, VkDisplayModeKHR>(physicalDevice, displaySurfaceCreateInfo.displayMode);
        }
    }
    return VK_SUCCESS;
}

template <>
VkResult initialize_control_block<SwapchainKHR>(SwapchainKHR& swapchain)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto& swapchainControlBlock = swapchain.mReference.get_obj();
        swapchainControlBlock.mSurfaceKHR = HandleId<VkInstance, VkSurfaceKHR>(swapchainControlBlock.mDevice.get<Instance>(), swapchainControlBlock.mSwapchainCreateInfoKHR->surface);
        uint32_t swapchainImageCount = 0;
        const auto& dispatchTable = swapchainControlBlock.mDevice.get<DispatchTable>();
        assert(dispatchTable.gvkGetSwapchainImagesKHR);
        gvk_result(dispatchTable.gvkGetSwapchainImagesKHR(swapchainControlBlock.mDevice, swapchainControlBlock.mVkSwapchainKHR, &swapchainImageCount, nullptr));
        auto pVkImages = (VkImage*)detail::get_transient_storage(swapchainImageCount * sizeof(VkImage));
        gvk_result(dispatchTable.gvkGetSwapchainImagesKHR(swapchainControlBlock.mDevice, swapchainControlBlock.mVkSwapchainKHR, &swapchainImageCount, pVkImages));
        auto& images = swapchainControlBlock.mImages;
        images.resize(swapchainImageCount);
        const auto& swapchainCreateInfo = *swapchainControlBlock.mSwapchainCreateInfoKHR;
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
            images[i].mReference.reset(newref, HandleId<VkDevice, VkImage>(swapchainControlBlock.mDevice, pVkImages[i]));
            auto& imageControlBlock = images[i].mReference.get_obj();
            imageControlBlock.mVkImage = pVkImages[i];
            imageControlBlock.mDevice = swapchainControlBlock.mDevice;
            imageControlBlock.mVkSwapchainKHR = swapchainControlBlock.mVkSwapchainKHR;
            imageControlBlock.mImageCreateInfo = imageCreateInfo;
            gvk_result(detail::initialize_control_block(imageControlBlock));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace detail

Instance::ControlBlock::~ControlBlock()
{
    if (!mUnmanaged) {
        DispatchTable dispatchTable { };
        dispatchTable = mDispatchTable;
        assert(dispatchTable.gvkDestroyInstance);
        dispatchTable.gvkDestroyInstance(mVkInstance, (mAllocationCallbacks.pfnFree ? &mAllocationCallbacks : nullptr));
    }
}

Device::ControlBlock::~ControlBlock()
{
    if (mVmaAllocator) {
        vmaDestroyAllocator(mVmaAllocator);
    }
    if (!mUnmanaged) {
        assert(mDispatchTable.gvkDeviceWaitIdle);
        mDispatchTable.gvkDeviceWaitIdle(mVkDevice);
        assert(mDispatchTable.gvkDestroyDevice);
        mDispatchTable.gvkDestroyDevice(mVkDevice, (mAllocationCallbacks.pfnFree ? &mAllocationCallbacks : nullptr));
    }
}

Buffer::ControlBlock::~ControlBlock()
{
    assert(mDevice);
    if (mVmaAllocation) {
        vmaDestroyBuffer(mDevice.get<VmaAllocator>(), mVkBuffer, mVmaAllocation);
    } else {
        const auto& dispatchTable = mDevice.get<DispatchTable>();
        assert(dispatchTable.gvkDestroyBuffer);
        dispatchTable.gvkDestroyBuffer(mDevice, mVkBuffer, (mAllocationCallbacks.pfnFree ? &mAllocationCallbacks : nullptr));
    }
}

Image::ControlBlock::~ControlBlock()
{
    if (!mVkSwapchainKHR) {
        assert(mDevice);
        if (mVmaAllocation) {
            vmaDestroyImage(mDevice.get<VmaAllocator>(), mVkImage, mVmaAllocation);
        } else {
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            assert(dispatchTable.gvkDestroyImage);
            dispatchTable.gvkDestroyImage(mDevice, mVkImage, (mAllocationCallbacks.pfnFree ? &mAllocationCallbacks : nullptr));
        }
    }
}

} // namespace gvk
