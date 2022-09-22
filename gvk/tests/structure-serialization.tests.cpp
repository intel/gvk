
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

#include "gvk/serialization.hpp"
#include "gvk/spir-v.hpp"
#include "gvk/structures.hpp"
#include "gvk/to-string.hpp"

#include "gtest/gtest.h"

#include <array>
#include <sstream>

/*
The following tests validate that generated serialization/deserialization functions work correctly
Test cases cover the following:
    Basic members
    Union types
    Static arrays
    Dynamic arrays
    Static strings
    Dynamic strings
    Arrays of strings
    pNext
    Structures with dynamic array members of structures with dynamic array members
    Structures with pointers to structures

    (special case members)
    VkPipelineMultisampleStateCreateInfo
    VkShaderModuleCreateInfo
*/

template <typename VulkanStructureType>
inline void validate_structure_serialization(const VulkanStructureType& obj)
{
    std::stringstream strStrm(std::ios::binary | std::ios::in | std::ios::out);
    gvk::serialize(strStrm, obj);
    gvk::Auto<VulkanStructureType> deserialized;
    gvk::deserialize(strStrm, nullptr, deserialized);
    if (obj != deserialized) {
        FAIL()
            << "===============================================================================" << std::endl
            << gvk::to_string(obj) << std::endl
            << "-------------------------------------------------------------------------------" << std::endl
            << gvk::to_string(deserialized) << std::endl
            << "===============================================================================" << std::endl
            << std::endl;
    }
}

TEST(Serialization, Basic)
{
    validate_structure_serialization(
        VkExtent3D{
            .width = 256,
            .height = 512,
            .depth = 1024,
        }
    );
}

TEST(Serialization, Union)
{
    VkClearColorValue clearColorValue{ };
    clearColorValue.float32[0] = 0.5f;
    clearColorValue.float32[1] = 0.5f;
    clearColorValue.float32[2] = 0.5f;
    clearColorValue.float32[3] = 1;
    validate_structure_serialization(clearColorValue);
}

TEST(Serialization, UnionWithUnionMember)
{
    VkClearValue clearValue{ };
    clearValue.color.float32[0] = 0.5f;
    clearValue.color.float32[1] = 0.5f;
    clearValue.color.float32[2] = 0.5f;
    clearValue.color.float32[3] = 1;
    validate_structure_serialization(clearValue);
}

TEST(Serialization, StructureWithUnionMember)
{
    VkClearAttachment clearAttachment{ };
    clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAttachment.colorAttachment = 0;
    clearAttachment.clearValue.color.float32[0] = 0.5f;
    clearAttachment.clearValue.color.float32[1] = 0.5f;
    clearAttachment.clearValue.color.float32[2] = 0.5f;
    clearAttachment.clearValue.color.float32[3] = 1;
    validate_structure_serialization(clearAttachment);
}

TEST(Serialization, StructureWithStaticallySizedArrayMember)
{
    // FROM : VkPhysicalDeviceMemoryProperties values taken from https://vulkan.gpuinfo.org
    VkPhysicalDeviceMemoryProperties intelHD530MemoryProperties { };
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
    validate_structure_serialization(intelHD530MemoryProperties);

    VkPhysicalDeviceMemoryProperties nvidiaGTX1070MemoryProperties { };
    nvidiaGTX1070MemoryProperties.memoryTypeCount = 11;
    nvidiaGTX1070MemoryProperties.memoryTypes[0].propertyFlags = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[0].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryTypes[1].propertyFlags = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[1].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryTypes[2].propertyFlags = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[2].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryTypes[3].propertyFlags = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[3].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryTypes[4].propertyFlags = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[4].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryTypes[5].propertyFlags = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[5].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryTypes[6].propertyFlags = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[6].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryTypes[7].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    nvidiaGTX1070MemoryProperties.memoryTypes[7].heapIndex = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[8].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    nvidiaGTX1070MemoryProperties.memoryTypes[8].heapIndex = 0;
    nvidiaGTX1070MemoryProperties.memoryTypes[9].propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    nvidiaGTX1070MemoryProperties.memoryTypes[9].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryTypes[10].propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    nvidiaGTX1070MemoryProperties.memoryTypes[10].heapIndex = 1;
    nvidiaGTX1070MemoryProperties.memoryHeapCount = 2;
    nvidiaGTX1070MemoryProperties.memoryHeaps[0].size = 8480882688;
    nvidiaGTX1070MemoryProperties.memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
    nvidiaGTX1070MemoryProperties.memoryHeaps[1].size = 17012817920;
    nvidiaGTX1070MemoryProperties.memoryHeaps[1].flags = 0;
    validate_structure_serialization(nvidiaGTX1070MemoryProperties);
}

TEST(Serialization, StructureWithDynamicallySizedArrayMember)
{
    VkImageCreateInfo imageCreateInfo{ };
    std::array<uint32_t, 4> queueFamilyIndices{ 1, 4, 6, 8 };
    imageCreateInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
    imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    validate_structure_serialization(imageCreateInfo);
}

TEST(Serialization, StructureWithStaticallySizedStringMember)
{
    VkPhysicalDeviceProperties intelHD530Properties{ };
    strcpy(intelHD530Properties.deviceName, "Intel(R) HD Graphics 530");
    validate_structure_serialization(intelHD530Properties);
}

TEST(Serialization, StructureWithDynamicallySizedStringMember)
{
    VkApplicationInfo applicationInfo{ };
    applicationInfo.pApplicationName = "Intel GPA";
    validate_structure_serialization(applicationInfo);
}

TEST(Serialization, StructureWithArrayOfStringsMember)
{
    VkInstanceCreateInfo instanceCreateInfo { };
    std::array<const char*, 4> enabledLayerNames0 {
        "VK_LAYER_0",
        "VK_LAYER_1",
        "VK_LAYER_2",
        "VK_LAYER_3"
    };
    instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayerNames0.size();
    instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames0.data();
    validate_structure_serialization(instanceCreateInfo);
}

TEST(Serialization, StructureWithPNextMember)
{
    VkMemoryAllocateInfo memoryAllocateInfo { };
    VkImportMemoryHostPointerInfoEXT importMemoryHostPointerInfo { };
    importMemoryHostPointerInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
    importMemoryHostPointerInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    memoryAllocateInfo.pNext = &importMemoryHostPointerInfo;
    validate_structure_serialization(importMemoryHostPointerInfo);
}

TEST(Serialization, StructureWithDynamicArrayOfStructureWithDynamicArray)
{
    const size_t QueueCount = 6;
    const size_t DeviceQueueCreateInfoCount = 4;
    std::array<std::array<float, QueueCount>, DeviceQueueCreateInfoCount> queuePriorites;
    std::array<VkDeviceQueueCreateInfo, DeviceQueueCreateInfoCount> deviceQueueCreateInfos { };
    auto createDeviceCreateInfo =
    [](
        std::array<std::array<float, QueueCount>, DeviceQueueCreateInfoCount>& queuePrioritesCollection,
        std::array<VkDeviceQueueCreateInfo, DeviceQueueCreateInfoCount>& deviceQueueCreateInfosCollection
    )
    {
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
        return deviceCreateInfo;
    };
    auto deviceCreateInfo = createDeviceCreateInfo(queuePriorites, deviceQueueCreateInfos);
    validate_structure_serialization(deviceCreateInfo);
}

TEST(Serialization, StructureWithPointerToStructure)
{
    VkApplicationInfo applicationInfo { };
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Intel GPA";
    applicationInfo.applicationVersion = 0;
    VkInstanceCreateInfo instanceCreateInfo { };
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    validate_structure_serialization(instanceCreateInfo);
}

TEST(Serialization, VkPipelineMultisampleStateCreateInfo)
{
    VkPipelineMultisampleStateCreateInfo pipelineMultiSampleCreateInfo { };
    std::vector<VkSampleMask> sampleMask0(VK_SAMPLE_COUNT_64_BIT / 32);
    pipelineMultiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT;
    pipelineMultiSampleCreateInfo.pSampleMask = sampleMask0.data();
    for (size_t i = 0; i < sampleMask0.size(); ++i) {
        sampleMask0[i] = (uint32_t)((i + 1) * 3);
    }
    validate_structure_serialization(pipelineMultiSampleCreateInfo);
}

TEST(Serialization, VkShaderModuleCreateInfo)
{
    std::array<uint32_t, 8> spirv{
        8, 16, 32, 64, 128, 256, 512, 1024
    };
    auto shaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
    shaderModuleCreateInfo.codeSize = (uint32_t)spirv.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = spirv.data();
    validate_structure_serialization(shaderModuleCreateInfo);
}
