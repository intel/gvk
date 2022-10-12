
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

#include "gvk/generated/comparison-operators.hpp"
#include "gvk/spir-v.hpp"

#include "gtest/gtest.h"

#include <array>

/*
The following tests validate that generated comparison operators for Vulkan structures work correctly
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

TEST(ComparisonOperators, Basic)
{
    VkExtent3D extent3D0{ };
    extent3D0.width = 256;
    extent3D0.height = 512;
    extent3D0.depth = 1024;
    auto extent3D1 = extent3D0;
    EXPECT_EQ(extent3D0, extent3D1);

    extent3D1.width = 16;
    extent3D1.height = 32;
    extent3D1.depth = 64;
    EXPECT_NE(extent3D0, extent3D1);
}

TEST(ComparisonOperators, Union)
{
    VkClearColorValue clearColorValue0{ };
    clearColorValue0.float32[0] = 0.5f;
    clearColorValue0.float32[1] = 0.5f;
    clearColorValue0.float32[2] = 0.5f;
    clearColorValue0.float32[3] = 1;
    auto clearColorValue1 = clearColorValue0;
    EXPECT_EQ(clearColorValue0, clearColorValue1);

    clearColorValue1.uint32[0] = 255;
    clearColorValue1.uint32[0] = 255;
    clearColorValue1.uint32[0] = 255;
    clearColorValue1.uint32[0] = 128;
    EXPECT_NE(clearColorValue0, clearColorValue1);
}

TEST(ComparisonOperators, UnionWithUnionMember)
{
    VkClearValue clearValue0{ };
    clearValue0.color.float32[0] = 0.5f;
    clearValue0.color.float32[1] = 0.5f;
    clearValue0.color.float32[2] = 0.5f;
    clearValue0.color.float32[3] = 1;
    auto clearValue1 = clearValue0;
    EXPECT_EQ(clearValue0, clearValue1);

    clearValue1.color.uint32[0] = 255;
    clearValue1.color.uint32[0] = 255;
    clearValue1.color.uint32[0] = 255;
    clearValue1.color.uint32[0] = 128;
    EXPECT_NE(clearValue0, clearValue1);

    clearValue1 = clearValue0;
    EXPECT_EQ(clearValue0, clearValue1);

    clearValue1.depthStencil.depth = 1;
    clearValue1.depthStencil.stencil = 1;
    EXPECT_NE(clearValue0, clearValue1);
}

TEST(ComparisonOperators, StructureWithUnionMember)
{
    VkClearAttachment clearAttachment0{ };
    clearAttachment0.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAttachment0.colorAttachment = 0;
    clearAttachment0.clearValue.color.float32[0] = 0.5f;
    clearAttachment0.clearValue.color.float32[1] = 0.5f;
    clearAttachment0.clearValue.color.float32[2] = 0.5f;
    clearAttachment0.clearValue.color.float32[3] = 1;
    auto clearAttachment1 = clearAttachment0;
    EXPECT_EQ(clearAttachment0, clearAttachment1);

    clearAttachment1.clearValue.color.uint32[0] = 255;
    clearAttachment1.clearValue.color.uint32[0] = 255;
    clearAttachment1.clearValue.color.uint32[0] = 255;
    clearAttachment1.clearValue.color.uint32[0] = 128;
    EXPECT_NE(clearAttachment0, clearAttachment1);
}

TEST(ComparisonOperators, StructureWithStaticallySizedArrayMember)
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
    auto intelHD530SkylakeMemoryPropertiesAlt = intelHD530MemoryProperties;
    EXPECT_EQ(intelHD530MemoryProperties, intelHD530SkylakeMemoryPropertiesAlt);
    intelHD530SkylakeMemoryPropertiesAlt.memoryTypeCount = 1;
    intelHD530SkylakeMemoryPropertiesAlt.memoryHeapCount = 1;
    EXPECT_NE(intelHD530MemoryProperties, intelHD530SkylakeMemoryPropertiesAlt);

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
    EXPECT_NE(intelHD530MemoryProperties, nvidiaGTX1070MemoryProperties);
}

TEST(ComparisonOperators, StructureWithDynamicallySizedArrayMember)
{
    // For this test we're creating two different arrays with the same
    //  values so that we can validate that the comparison is performed on the
    //  contents of the arrays and not on the pointers themselves...
    VkImageCreateInfo imageCreateInfo0{ };
    std::array<uint32_t, 4> queueFamilyIndices0{ 1, 4, 6, 8 };
    imageCreateInfo0.queueFamilyIndexCount = (uint32_t)queueFamilyIndices0.size();
    imageCreateInfo0.pQueueFamilyIndices = queueFamilyIndices0.data();

    VkImageCreateInfo imageCreateInfo1{ };
    auto queueFamilyIndices1 = queueFamilyIndices0;
    imageCreateInfo1.queueFamilyIndexCount = (uint32_t)queueFamilyIndices1.size();
    imageCreateInfo1.pQueueFamilyIndices = queueFamilyIndices1.data();
    EXPECT_EQ(imageCreateInfo0, imageCreateInfo1);

    // ...Now we're changing one of the arrays to validate that we get a not
    //  equal when we compare the two structures.
    queueFamilyIndices1[0] = 4;
    queueFamilyIndices1[1] = 7;
    queueFamilyIndices1[2] = 9;
    queueFamilyIndices1[3] = 12;
    EXPECT_NE(imageCreateInfo0, imageCreateInfo1);
}

TEST(ComparisonOperators, StructureWithStaticallySizedStringMember)
{
    VkPhysicalDeviceProperties intelHD530Properties{ };
    strcpy(intelHD530Properties.deviceName, "Intel(R) HD Graphics 530");
    auto intelHD530PropertiesAlt = intelHD530Properties;
    EXPECT_EQ(intelHD530Properties, intelHD530PropertiesAlt);

    strcpy(intelHD530PropertiesAlt.deviceName, "----");
    EXPECT_NE(intelHD530Properties, intelHD530PropertiesAlt);

    VkPhysicalDeviceProperties nvidiaGTX1070Properties{ };
    strcpy(nvidiaGTX1070Properties.deviceName, "GeForce GTX 1070");
    EXPECT_NE(intelHD530Properties, nvidiaGTX1070Properties);
}

TEST(ComparisonOperators, StructureWithDynamicallySizedStringMember)
{
    VkApplicationInfo applicationInfo0{ };
    applicationInfo0.pApplicationName = "Intel GPA";

    VkApplicationInfo applicationInfo1{ };
    applicationInfo1.pApplicationName = "Intel GPA";
    EXPECT_EQ(applicationInfo0, applicationInfo1);

    applicationInfo1.pApplicationName = "Intel GPA Player";
    EXPECT_NE(applicationInfo0, applicationInfo1);

    applicationInfo1.pApplicationName = nullptr;
    EXPECT_NE(applicationInfo0, applicationInfo1);
}

TEST(ComparisonOperators, StructureWithArrayOfStringsMember)
{
    VkInstanceCreateInfo instanceCreateInfo0 { };
    std::array<const char*, 4> enabledLayerNames0 {
        "VK_LAYER_0",
        "VK_LAYER_1",
        "VK_LAYER_2",
        "VK_LAYER_3"
    };
    instanceCreateInfo0.enabledLayerCount = (uint32_t)enabledLayerNames0.size();
    instanceCreateInfo0.ppEnabledLayerNames = enabledLayerNames0.data();

    VkInstanceCreateInfo instanceCreateInfo1 { };
    std::array<const char*, 4> enabledLayerNames1 {
        "VK_LAYER_0",
        "VK_LAYER_1",
        "VK_LAYER_2",
        "VK_LAYER_3"
    };
    instanceCreateInfo1.enabledLayerCount = (uint32_t)enabledLayerNames1.size();
    instanceCreateInfo1.ppEnabledLayerNames = enabledLayerNames1.data();
    EXPECT_EQ(instanceCreateInfo0, instanceCreateInfo1);

    enabledLayerNames1[2] = "VK_LAYER_4";
    enabledLayerNames1[3] = "VK_LAYER_5";
    EXPECT_NE(instanceCreateInfo0, instanceCreateInfo1);
}

TEST(ComparisonOperators, StructureWithPNextMember)
{
    VkMemoryAllocateInfo memoryAllocateInfo0 { };
    VkImportMemoryHostPointerInfoEXT importMemoryHostPointerInfo0 { };
    importMemoryHostPointerInfo0.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
    importMemoryHostPointerInfo0.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    float hostAllocation0;
    importMemoryHostPointerInfo0.pHostPointer = &hostAllocation0;
    memoryAllocateInfo0.pNext = &importMemoryHostPointerInfo0;

    VkMemoryAllocateInfo memoryAllocateInfo1 { };
    VkImportMemoryHostPointerInfoEXT importMemoryHostPointerInfo1 { };
    importMemoryHostPointerInfo1.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
    importMemoryHostPointerInfo1.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    importMemoryHostPointerInfo1.pHostPointer = &hostAllocation0;
    memoryAllocateInfo1.pNext = &importMemoryHostPointerInfo1;
    EXPECT_EQ(memoryAllocateInfo0, memoryAllocateInfo1);

    float hostAllocation1;
    importMemoryHostPointerInfo1.pHostPointer = &hostAllocation1;
    EXPECT_NE(memoryAllocateInfo0, memoryAllocateInfo1);
}

TEST(ComparisonOperators, StructureWithDynamicArrayOfStructureWithDynamicArray)
{
    const size_t QueueCount = 6;
    const size_t DeviceQueueCreateInfoCount = 4;
    std::array<std::array<float, QueueCount>, DeviceQueueCreateInfoCount> queuePriorites0;
    std::array<VkDeviceQueueCreateInfo, DeviceQueueCreateInfoCount> deviceQueueCreateInfos0 { };
    std::array<std::array<float, QueueCount>, DeviceQueueCreateInfoCount> queuePriorites1;
    std::array<VkDeviceQueueCreateInfo, DeviceQueueCreateInfoCount> deviceQueueCreateInfos1 { };
    auto createDeviceCreateInfo =
    [](
        std::array<std::array<float, QueueCount>, DeviceQueueCreateInfoCount>& queuePrioritiesCollection,
        std::array<VkDeviceQueueCreateInfo, DeviceQueueCreateInfoCount>& deviceQueueCreateInfosCollection
    )
    {
        for (size_t deviceQueueCreateInfo_i = 0; deviceQueueCreateInfo_i < deviceQueueCreateInfosCollection.size(); ++deviceQueueCreateInfo_i) {
            auto& deviceQueueCreateInfo = deviceQueueCreateInfosCollection[deviceQueueCreateInfo_i];
            auto& queuePriorites = queuePrioritiesCollection[deviceQueueCreateInfo_i];
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
    auto deviceCreateInfo0 = createDeviceCreateInfo(queuePriorites0, deviceQueueCreateInfos0);
    auto deviceCreateInfo1 = createDeviceCreateInfo(queuePriorites1, deviceQueueCreateInfos1);
    EXPECT_EQ(deviceCreateInfo0, deviceCreateInfo1);
    queuePriorites0[2][2] = 0;
    EXPECT_NE(deviceCreateInfo0, deviceCreateInfo1);
}

TEST(ComparisonOperators, StructureWithPointerToStructure)
{
    VkApplicationInfo applicationInfo0 { };
    applicationInfo0.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo0.pApplicationName = "Intel GPA";
    applicationInfo0.applicationVersion = 0;
    VkInstanceCreateInfo instanceCreateInfo0 { };
    instanceCreateInfo0.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo0.pApplicationInfo = &applicationInfo0;

    VkApplicationInfo applicationInfo1 { };
    applicationInfo1.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo1.pApplicationName = "Intel GPA";
    applicationInfo1.applicationVersion = 0;
    VkInstanceCreateInfo instanceCreateInfo1 { };
    instanceCreateInfo1.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo1.pApplicationInfo = &applicationInfo1;

    EXPECT_EQ(instanceCreateInfo0, instanceCreateInfo1);
    applicationInfo1.applicationVersion = 1;
    EXPECT_NE(instanceCreateInfo0, instanceCreateInfo1);
}

TEST(ComparisonOperators, VkPipelineMultisampleStateCreateInfo)
{
    VkPipelineMultisampleStateCreateInfo pipelineMultiSampleCreateInfo0 { };
    std::vector<VkSampleMask> sampleMask0(VK_SAMPLE_COUNT_64_BIT / 32);
    pipelineMultiSampleCreateInfo0.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT;
    pipelineMultiSampleCreateInfo0.pSampleMask = sampleMask0.data();
    for (size_t i = 0; i < sampleMask0.size(); ++i) {
        sampleMask0[i] = (uint32_t)((i + 1) * 3);
    }

    VkPipelineMultisampleStateCreateInfo pipelineMultiSampleCreateInfo1 { };
    std::vector<VkSampleMask> sampleMask1(VK_SAMPLE_COUNT_64_BIT / 32);
    pipelineMultiSampleCreateInfo1.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT;
    pipelineMultiSampleCreateInfo1.pSampleMask = sampleMask1.data();
    for (size_t i = 0; i < sampleMask1.size(); ++i) {
        sampleMask1[i] = (uint32_t)((i + 1) * 3);
    }

    // First we validate that our structrues are equal...
    EXPECT_EQ(pipelineMultiSampleCreateInfo0, pipelineMultiSampleCreateInfo1);
    // ...Then we'll break the first VkSampleMask and verify that our structures
    //  are no longer equal...
    auto value = sampleMask0.front();
    sampleMask0.front() = 0;
    EXPECT_NE(pipelineMultiSampleCreateInfo0, pipelineMultiSampleCreateInfo1);
    // ...Then we'll set them back to equal and validate...
    sampleMask0.front() = value;
    EXPECT_EQ(pipelineMultiSampleCreateInfo0, pipelineMultiSampleCreateInfo1);
    // ...Finally we'll break our second VkSampleMask and validate thet our
    //  structures are inequal...
    sampleMask0.back() = 0;
    EXPECT_NE(pipelineMultiSampleCreateInfo0, pipelineMultiSampleCreateInfo1);
}

TEST(ComparisonOperators, VkShaderModuleCreateInfo)
{
    std::array<uint32_t, 8> spirv0{
        8, 16, 32, 64, 128, 256, 512, 1024
    };
    auto shaderModuleCreateInfo0 = gvk::get_default<VkShaderModuleCreateInfo>();
    shaderModuleCreateInfo0.codeSize = (uint32_t)spirv0.size() * sizeof(uint32_t);
    shaderModuleCreateInfo0.pCode = spirv0.data();

    std::array<uint32_t, 8> spirv1{
        8, 16, 32, 64, 128, 256, 512, 1024
    };
    auto shaderModuleCreateInfo1 = gvk::get_default<VkShaderModuleCreateInfo>();
    shaderModuleCreateInfo1.codeSize = (uint32_t)spirv1.size() * sizeof(uint32_t);
    shaderModuleCreateInfo1.pCode = spirv1.data();

    EXPECT_EQ(shaderModuleCreateInfo0, shaderModuleCreateInfo1);
    shaderModuleCreateInfo0.codeSize /= 2;
    EXPECT_NE(shaderModuleCreateInfo0, shaderModuleCreateInfo1);
    shaderModuleCreateInfo0.codeSize = (uint32_t)spirv0.size() * sizeof(uint32_t);
    EXPECT_EQ(shaderModuleCreateInfo0, shaderModuleCreateInfo1);
    shaderModuleCreateInfo1.codeSize /= 2;
    EXPECT_NE(shaderModuleCreateInfo0, shaderModuleCreateInfo1);
}
