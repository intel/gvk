
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

#include "state-tracker-test-utilities.hpp"

#include <random>
#include <unordered_set>

TEST(DeviceAddressTracker, Placeholder)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    if (context.get_physical_device_buffer_device_address_features().bufferDeviceAddress) {
        const auto& gvkDevice = context.get<gvk::Devices>()[0];

        // TODO : Documentation
        uint32_t bindingCount = 0;
        gvkGetStateTrackedBufferDeviceAddressBindings(gvkDevice, 11111111, &bindingCount, nullptr);
        EXPECT_EQ(bindingCount, 0);

        // TODO : Documentation
        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = 64;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        std::vector<gvk::Buffer> gvkBuffers(3);
        for (auto& gvkBuffer : gvkBuffers) {
            ASSERT_EQ(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, nullptr, &gvkBuffer), VK_SUCCESS);
            bufferCreateInfo.size *= 2;
        }

        // TODO : Documentation
        VkDeviceSize totalMemoryRequirementsSize = 0;
        std::vector<VkMemoryRequirements> memoryRequirements;
        for (const auto& gvkBuffer : gvkBuffers) {
            memoryRequirements.push_back(gvk::get_default<VkMemoryRequirements>());
            gvkDevice.GetBufferMemoryRequirements(gvkBuffer, &memoryRequirements.back());
            totalMemoryRequirementsSize += memoryRequirements.back().size;
            if (1 < memoryRequirements.size()) {
                EXPECT_EQ(memoryRequirements[memoryRequirements.size() - 1].alignment, memoryRequirements.back().alignment);
                EXPECT_EQ(memoryRequirements[memoryRequirements.size() - 1].memoryTypeBits, memoryRequirements.back().memoryTypeBits);
            }
        }

        // TODO : Documentation
        auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t memoryTypeCount = VK_MAX_MEMORY_TYPES;
        std::array<uint32_t, VK_MAX_MEMORY_TYPES> memoryTypeIndices;
        gvk::get_compatible_memory_type_indices(gvkDevice.get<gvk::PhysicalDevice>(), memoryRequirements[0].memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, memoryTypeIndices.data());
        ASSERT_TRUE(1 <= memoryTypeCount);

        // TODO : Documentation
        auto memoryAllocateFlagsInfo = gvk::get_default<VkMemoryAllocateFlagsInfo>();
        memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        auto memoryAllocateInfo = gvk::get_default<VkMemoryAllocateInfo>();
        memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndices[0];
        memoryAllocateInfo.allocationSize = totalMemoryRequirementsSize;
        gvk::DeviceMemory gvkDeviceMemory = VK_NULL_HANDLE;
        ASSERT_EQ(gvk::DeviceMemory::allocate(gvkDevice, &memoryAllocateInfo, nullptr, &gvkDeviceMemory), VK_SUCCESS);

        // TODO : Documentation
        ASSERT_EQ(gvkDevice.BindBufferMemory(gvkBuffers[0], gvkDeviceMemory, 0), VK_SUCCESS);
        ASSERT_EQ(gvkDevice.BindBufferMemory(gvkBuffers[1], gvkDeviceMemory, 0), VK_SUCCESS);
        ASSERT_EQ(gvkDevice.BindBufferMemory(gvkBuffers[2], gvkDeviceMemory, gvkBuffers[1].get<VkBufferCreateInfo>().size), VK_SUCCESS);

        // TODO : Documentation
        auto bufferDeviceAddressInfo = gvk::get_default<VkBufferDeviceAddressInfo>();
        bufferDeviceAddressInfo.buffer = gvkBuffers[0];
        auto bufferDeviceAddress = gvkDevice.GetBufferDeviceAddressKHR(&bufferDeviceAddressInfo);

        // TODO : Documentation
        bindingCount = 0;
        gvkGetStateTrackedBufferDeviceAddressBindings(gvkDevice, bufferDeviceAddress, &bindingCount, nullptr);
        ASSERT_EQ(bindingCount, 2);
        std::vector<VkBindBufferMemoryInfo> bindings(bindingCount);
        gvkGetStateTrackedBufferDeviceAddressBindings(gvkDevice, bufferDeviceAddress, &bindingCount, bindings.data());
        std::unordered_set<VkBuffer> expectedBuffers { gvkBuffers[0], gvkBuffers[1] };
        for (const auto& binding : bindings) {
            EXPECT_EQ(expectedBuffers.erase(binding.buffer), 1);
            EXPECT_EQ(binding.memory, gvkDeviceMemory);
            EXPECT_EQ(binding.memoryOffset, 0);
        }
        EXPECT_TRUE(expectedBuffers.empty());

        // TODO : Documentation
        bufferDeviceAddress += gvkBuffers[0].get<VkBufferCreateInfo>().size / 2;
        bindingCount = 0;
        gvkGetStateTrackedBufferDeviceAddressBindings(gvkDevice, bufferDeviceAddress, &bindingCount, nullptr);
        ASSERT_EQ(bindingCount, 2);
        bindings.clear();
        bindings.resize(bindingCount);
        gvkGetStateTrackedBufferDeviceAddressBindings(gvkDevice, bufferDeviceAddress, &bindingCount, bindings.data());
        expectedBuffers = { gvkBuffers[0], gvkBuffers[1] };
        for (const auto& binding : bindings) {
            EXPECT_EQ(expectedBuffers.erase(binding.buffer), 1);
            EXPECT_EQ(binding.memory, gvkDeviceMemory);
            EXPECT_EQ(binding.memoryOffset, 0);
        }
        EXPECT_TRUE(expectedBuffers.empty());

        // TODO : Documentation
        bufferDeviceAddress += gvkBuffers[0].get<VkBufferCreateInfo>().size / 2;
        bufferDeviceAddressInfo.buffer = gvkBuffers[1];
        EXPECT_EQ(bufferDeviceAddress, gvkDevice.GetBufferDeviceAddressKHR(&bufferDeviceAddressInfo) + gvkBuffers[0].get<VkBufferCreateInfo>().size);
        bindingCount = 0;
        gvkGetStateTrackedBufferDeviceAddressBindings(gvkDevice, bufferDeviceAddress, &bindingCount, nullptr);
        ASSERT_EQ(bindingCount, 1);
        bindings.clear();
        bindings.resize(bindingCount);
        gvkGetStateTrackedBufferDeviceAddressBindings(gvkDevice, bufferDeviceAddress, &bindingCount, bindings.data());
        expectedBuffers = { gvkBuffers[1] };
        for (const auto& binding : bindings) {
            EXPECT_EQ(expectedBuffers.erase(binding.buffer), 1);
            EXPECT_EQ(binding.memory, gvkDeviceMemory);
            EXPECT_EQ(binding.memoryOffset, 0);
        }
        EXPECT_TRUE(expectedBuffers.empty());

        // TODO : vkDestroyBuffer()
        // TODO : vkFreeMemory()
    }
}
