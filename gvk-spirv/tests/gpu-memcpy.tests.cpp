
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

#include "gvk-spirv/gpu-memcpy.hpp"
#include "spirv-validation-context.hpp"
#include "spirv-validation-utilities.hpp"

#include <cstdint>
#include <cstring>
#include <random>
#include <vector>

static VkResult create_buffer(const gvk::Device& gvkDevice, VkDeviceSize size, const uint8_t* pData, gvk::Buffer* pGvkBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(size ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        auto allocationCreateInfo = gvk::get_default<VmaAllocationCreateInfo>();
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &allocationCreateInfo, pGvkBuffer));

        if (pData) {
            uint8_t* pMappedData = nullptr;
            gvk_result(vmaMapMemory(gvkDevice.get<VmaAllocator>(), pGvkBuffer->get<VmaAllocation>(), (void**)&pMappedData));
            memcpy(pMappedData, pData, size);
            vmaUnmapMemory(gvkDevice.get<VmaAllocator>(), pGvkBuffer->get<VmaAllocation>());
        }
    } gvk_result_scope_end;
    return gvkResult;
}

static bool data_is_equal(const std::vector<uint8_t>& testData, const gvk::Buffer& gvkBuffer)
{
    bool buffersAreEqual = false;
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(gvkBuffer ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(testData.size() == gvkBuffer.get<VkBufferCreateInfo>().size ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        const auto& gvkDevice = gvkBuffer.get<gvk::Device>();

        uint8_t* pMappedData = nullptr;
        gvk_result(vmaMapMemory(gvkDevice.get<VmaAllocator>(), gvkBuffer.get<VmaAllocation>(), (void**)&pMappedData));
        buffersAreEqual = !memcmp(testData.data(), pMappedData, testData.size());
        vmaUnmapMemory(gvkDevice.get<VmaAllocator>(), gvkBuffer.get<VmaAllocation>());
    } gvk_result_scope_end;
    assert(gvkResult == VK_SUCCESS);
    return buffersAreEqual;
}

TEST(spirv, GpuMemcpy)
{
    // Create gvk::spirv::validation::Context and check features
    gvk::spirv::validation::Context context;
    ASSERT_EQ(gvk::spirv::validation::Context::create(&context), VK_SUCCESS);
    if (context.get_physical_device_8_bit_storage_features().storageBuffer8BitAccess &&
        context.get_physical_device_buffer_device_address_features().bufferDeviceAddress
    ) {
        const auto& gvkDevice = context.get<gvk::Devices>()[0];
        const auto& gvkQueue = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0];
        const auto& gvkCommandBuffer = context.get<gvk::CommandBuffers>()[0];

        // Create GPU memcpy pipeline
        gvk::Pipeline gpuMemcpyPipeline;
        ASSERT_EQ(gvk::create_gpu_memcpy_pipeline(gvkDevice, &gpuMemcpyPipeline), VK_SUCCESS);

        // Random number generator
        std::mt19937 rng(std::random_device{ }());
        std::uniform_int_distribution<size_t> sizeDistribution(4, 256); // Random buffer sizes from 4 bytes to 256 bytes
        std::uniform_int_distribution<uint32_t> valueDistribution(0, 255); // Random buffer values from 0 to 255

        // Generate test data
        const size_t TestCount = 32;
        for (size_t i = 0; i < TestCount; ++i) {

            // Create std::vector<uint8_t> with random size and populate it with random data
            std::vector<uint8_t> data(sizeDistribution(rng));
            for (auto& value : data) {
                value = (uint8_t)valueDistribution(rng);
            }

            // Create buffers and populate the src buffer with the randomized data
            gvk::Buffer dstBuffer;
            gvk::Buffer srcBuffer;
            ASSERT_EQ(create_buffer(gvkDevice, data.size(), nullptr, &dstBuffer), VK_SUCCESS);
            ASSERT_EQ(create_buffer(gvkDevice, data.size(), data.data(), &srcBuffer), VK_SUCCESS);

            // Validate buffers against data, expect dstBuffer unequal, srcBuffer equal
            EXPECT_FALSE(data_is_equal(data, dstBuffer));
            EXPECT_TRUE(data_is_equal(data, srcBuffer));

            // Execute the GPU memcpy
            auto gpuMemcpyInfo = gvk::get_default<gvk::GpuMemcpyInfo>();
            gpuMemcpyInfo.dst = gvk::spirv::validation::get_buffer_device_address(gvkDevice, dstBuffer);
            gpuMemcpyInfo.src = gvk::spirv::validation::get_buffer_device_address(gvkDevice, srcBuffer);
            gpuMemcpyInfo.size = data.size();
            ASSERT_EQ(gvk::execute_gpu_memcpy(gvkDevice, gvkQueue, gvkCommandBuffer, VK_NULL_HANDLE, &gpuMemcpyInfo, gpuMemcpyPipeline), VK_SUCCESS);

            // Validate buffers against data, expect both buffers equal
            EXPECT_TRUE(data_is_equal(data, dstBuffer));
            EXPECT_TRUE(data_is_equal(data, srcBuffer));
        }
    }
}
