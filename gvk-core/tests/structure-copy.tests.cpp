
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

#define _CRT_SECURE_NO_WARNINGS

#include "gvk/structures.hpp"
#include "gvk/spir-v.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

#include <array>
#include <cstdlib>

class ValidationAllocator
{
public:
    inline ValidationAllocator()
    {
        mAllocationCallbacks.pUserData = this;
        mAllocationCallbacks.pfnAllocation = validate_allocation;
        mAllocationCallbacks.pfnFree = validate_free;
    }

    inline ~ValidationAllocator()
    {
        if (!mAllocations.empty()) {
            ADD_FAILURE();
        }
    }

    inline const VkAllocationCallbacks& get_allocation_callbacks() const
    {
        return mAllocationCallbacks;
    }

    inline static void* validate_allocation(void* pUserData, size_t size, size_t, VkSystemAllocationScope)
    {
        auto pMemory = malloc(size);
        ((ValidationAllocator*)pUserData)->mAllocations.insert(pMemory);
        return pMemory;
    }

    inline static void validate_free(void* pUserData, void* pMemory)
    {
        ((ValidationAllocator*)pUserData)->mAllocations.erase(pMemory);
        free(pMemory);
    }

private:
    VkAllocationCallbacks mAllocationCallbacks{ };
    std::set<void*> mAllocations;
};

TEST(create_structure_copy, Basic)
{
    VkExtent3D extent3d{ };
    extent3d.width = 256;
    extent3d.height = 512;
    extent3d.depth = 1024;
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(extent3d, &allocator.get_allocation_callbacks());
    EXPECT_EQ(extent3d, copy);
}

TEST(create_structure_copy, Union)
{
    VkClearColorValue clearColorValue{ };
    clearColorValue.float32[0] = 0.5f;
    clearColorValue.float32[1] = 0.5f;
    clearColorValue.float32[2] = 0.5f;
    clearColorValue.float32[3] = 1;
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(clearColorValue, &allocator.get_allocation_callbacks());
    EXPECT_EQ(clearColorValue, copy);
}

TEST(create_structure_copy, StructureWithUnionMember)
{
    VkClearAttachment clearAttachment{ };
    clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAttachment.colorAttachment = 0;
    clearAttachment.clearValue.color.float32[0] = 0.5f;
    clearAttachment.clearValue.color.float32[1] = 0.5f;
    clearAttachment.clearValue.color.float32[2] = 0.5f;
    clearAttachment.clearValue.color.float32[3] = 1;
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(clearAttachment, &allocator.get_allocation_callbacks());
    EXPECT_EQ(clearAttachment, copy);
}

TEST(create_structure_copy, StructureWithStaticallySizedArrayMember)
{
    // FROM : VkPhysicalDeviceMemoryProperties values taken from https://vulkan.gpuinfo.org
    VkPhysicalDeviceMemoryProperties intelHD530MemoryProperties{ };
    intelHD530MemoryProperties.memoryTypeCount = 2;
    intelHD530MemoryProperties.memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    intelHD530MemoryProperties.memoryTypes[0].heapIndex = 0;
    intelHD530MemoryProperties.memoryTypes[1].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    intelHD530MemoryProperties.memoryTypes[1].heapIndex = 1;
    intelHD530MemoryProperties.memoryHeapCount = 2;
    intelHD530MemoryProperties.memoryHeaps[0].size = 5104173056;
    intelHD530MemoryProperties.memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
    intelHD530MemoryProperties.memoryHeaps[1].size = 1079741824;
    intelHD530MemoryProperties.memoryHeaps[1].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(intelHD530MemoryProperties, &allocator.get_allocation_callbacks());
    EXPECT_EQ(intelHD530MemoryProperties, copy);
}

TEST(create_structure_copy, StructureWithDynamicallySizedArrayMember)
{
    VkImageCreateInfo imageCreateInfo{ };
    std::array<uint32_t, 4> queueFamilyIndices{ 1, 4, 6, 8 };
    imageCreateInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
    imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(imageCreateInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(imageCreateInfo, copy);
    queueFamilyIndices[0] = 4;
    queueFamilyIndices[1] = 7;
    queueFamilyIndices[2] = 9;
    queueFamilyIndices[3] = 12;
    EXPECT_NE(imageCreateInfo, copy);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}

TEST(create_structure_copy, StructureWithStaticallySizedStringMember)
{
    VkPhysicalDeviceProperties intelHD530Properties{ };
    strcpy(intelHD530Properties.deviceName, "Intel(R) HD Graphics 530");
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(intelHD530Properties, &allocator.get_allocation_callbacks());
    EXPECT_EQ(intelHD530Properties, copy);
}

TEST(create_structure_copy, StructureWithDynamicallySizedStringMember)
{
    VkApplicationInfo applicationInfo{ };
    applicationInfo.pApplicationName = "Intel GPA";
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(applicationInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(applicationInfo, copy);
    applicationInfo.pApplicationName = "Intel GPA Player";
    EXPECT_NE(applicationInfo, copy);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}

TEST(create_structure_copy, StructureWithArrayOfStringsMember)
{
    VkInstanceCreateInfo instanceCreateInfo { };
    std::array<const char*, 4> enabledLayerNames {
        "VK_LAYER_0",
        "VK_LAYER_1",
        "VK_LAYER_2",
        "VK_LAYER_3"
    };
    instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayerNames.size();
    instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(instanceCreateInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(instanceCreateInfo, copy);
    enabledLayerNames[2] = "VK_LAYER_4";
    enabledLayerNames[3] = "VK_LAYER_5";
    EXPECT_NE(instanceCreateInfo, copy);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}

TEST(create_structure_copy, StructureWithPNextMember)
{
    VkMemoryAllocateInfo memoryAllocateInfo{ };
    VkImportMemoryHostPointerInfoEXT importMemoryHostPointerInfo{ };
    importMemoryHostPointerInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
    importMemoryHostPointerInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    float hostAllocation;
    importMemoryHostPointerInfo.pHostPointer = &hostAllocation;
    memoryAllocateInfo.pNext = &importMemoryHostPointerInfo;
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(memoryAllocateInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(memoryAllocateInfo, copy);
    importMemoryHostPointerInfo.pHostPointer = nullptr;
    EXPECT_NE(memoryAllocateInfo, copy);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}

TEST(create_structure_copy, StructureWithEmbeddedStructureWithDynamicArray)
{
    const size_t QueueCount = 6;
    const size_t DeviceQueueCreateInfoCount = 4;
    std::array<std::array<float, QueueCount>, DeviceQueueCreateInfoCount> queuePrioritesCollection;
    std::array<VkDeviceQueueCreateInfo, DeviceQueueCreateInfoCount> deviceQueueCreateInfosCollection { };
    for (size_t deviceQueueCreateInfo_i = 0; deviceQueueCreateInfo_i < deviceQueueCreateInfosCollection.size(); ++deviceQueueCreateInfo_i) {
        auto& deviceQueueCreateInfo = deviceQueueCreateInfosCollection[deviceQueueCreateInfo_i];
        auto& queuePriorites = queuePrioritesCollection[deviceQueueCreateInfo_i];
        for (size_t queuePriority_i = 0; queuePriority_i < queuePriorites.size(); ++queuePriority_i) {
            queuePriorites[queuePriority_i] = (deviceQueueCreateInfo_i + queuePriority_i) * 3.14f;
        }
        deviceQueueCreateInfo.queueCount = (uint32_t)queuePriorites.size();
        deviceQueueCreateInfo.pQueuePriorities = queuePriorites.data();
    }
    VkDeviceCreateInfo deviceCreateInfo { };
    deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfosCollection.size();
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfosCollection.data();
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(deviceCreateInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(deviceCreateInfo, copy);
    queuePrioritesCollection[2][2] = 0;
    EXPECT_NE(deviceCreateInfo, copy);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}

TEST(create_structure_copy, StructureWithChainedPNexts)
{
    // NOTE : This setup is in no way correct usage of the Vulkan API, it is
    //  merely to validate that chaining pNext members works correctly...
    VkMemoryAllocateInfo memoryAllocateInfo { };
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = 262144;
    VkBufferCreateInfo bufferCreateInfo { };
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = &memoryAllocateInfo;
    bufferCreateInfo.size = 262144;
    VkImageCreateInfo imageCreateInfo { };
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = &bufferCreateInfo;
    imageCreateInfo.extent = { 512, 512, 1 };
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(imageCreateInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(imageCreateInfo, copy);
    // Altering memoryAllocateInfo should cause the comparison to be inequal...
    memoryAllocateInfo.allocationSize = 1048576;
    EXPECT_NE(imageCreateInfo, copy);
    // Revert and validate...
    memoryAllocateInfo.allocationSize = 262144;
    EXPECT_EQ(imageCreateInfo, copy);
    // Altering bufferCreateInfo should cause the comparison to be inequal...
    bufferCreateInfo.size = 1048576;
    EXPECT_NE(imageCreateInfo, copy);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}

TEST(create_structure_copy, StructuresWithPointersToStructures)
{
    VkApplicationInfo applicationInfo{ };
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Intel GPA";
    applicationInfo.applicationVersion = 0;
    VkInstanceCreateInfo instanceCreateInfo{ };
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(instanceCreateInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(instanceCreateInfo, copy);
    applicationInfo.applicationVersion = 1;
    EXPECT_NE(instanceCreateInfo, copy);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}

TEST(create_structure_copy, VkPipelineMultisampleStateCreateInfo)
{
    VkPipelineMultisampleStateCreateInfo pipelineMultiSampleCreateInfo { };
    std::vector<VkSampleMask> sampleMask(VK_SAMPLE_COUNT_64_BIT / 32);
    pipelineMultiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT;
    pipelineMultiSampleCreateInfo.pSampleMask = sampleMask.data();
    for (size_t i = 0; i < sampleMask.size(); ++i) {
        sampleMask[i] = (uint32_t)((i + 1) * 3);
    }
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(pipelineMultiSampleCreateInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(pipelineMultiSampleCreateInfo, copy);
    sampleMask.front() = 0;
    EXPECT_NE(pipelineMultiSampleCreateInfo, copy);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}

TEST(create_structure_copy, VkShaderModuleCreateInfo)
{
    std::array<uint32_t, 8> spirv{
        8, 16, 32, 64, 128, 256, 512, 1024
    };
    VkShaderModuleCreateInfo shaderModuleCreateInfo { };
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = (uint32_t)spirv.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = spirv.data();
    ValidationAllocator allocator;
    auto copy = gvk::detail::create_structure_copy(shaderModuleCreateInfo, &allocator.get_allocation_callbacks());
    EXPECT_EQ(shaderModuleCreateInfo, copy);
    shaderModuleCreateInfo.codeSize /= 2;
    EXPECT_NE(shaderModuleCreateInfo, copy);
    auto codeSize = (uint32_t)spirv.size() * sizeof(uint32_t);
    EXPECT_EQ(copy.codeSize, codeSize);
    auto diff = memcmp(shaderModuleCreateInfo.pCode, spirv.data(), codeSize);
    EXPECT_EQ(diff, 0);
    gvk::detail::destroy_structure_copy(copy, &allocator.get_allocation_callbacks());
}
