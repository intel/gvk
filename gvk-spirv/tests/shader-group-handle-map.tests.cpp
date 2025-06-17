
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
#include "gvk-spirv/shader-group-handle-map.hpp"
#include "spirv-validation-context.hpp"
#include "spirv-validation-utilities.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <random>
#include <vector>

class ShaderBindingTableEntry final
{
public:
    bool operator==(const ShaderBindingTableEntry& other) const
    {
        return handle == other.handle && data == other.data;
    }

    bool operator!=(const ShaderBindingTableEntry& other) const
    {
        return !(*this == other);
    }

    VkDeviceSize get_stride() const
    {
        return handle.size() + data.size();
    }

    std::vector<uint8_t> handle;
    std::vector<uint8_t> data;
};

static VkDeviceSize get_shader_group_handle_size(const gvk::Device& gvkDevice)
{
    auto physicalDeviceRayTracingProperties = gvk::get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    auto physicalDeviceProperties2 = gvk::get_default<VkPhysicalDeviceProperties2>();
    physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
    gvkDevice.get<gvk::PhysicalDevice>().GetPhysicalDeviceProperties2(&physicalDeviceProperties2);
    return physicalDeviceRayTracingProperties.shaderGroupHandleSize;
}

static std::vector<uint8_t> get_random_shader_group_handle(const gvk::Device& gvkDevice, std::uniform_int_distribution<uint32_t>& distribution, std::mt19937& rng)
{
    std::vector<uint8_t> shaderGroupHandle(get_shader_group_handle_size(gvkDevice));
    for (auto& byte : shaderGroupHandle) {
        byte = (uint8_t)distribution(rng);
    }
    return shaderGroupHandle;
}

static VkResult create_buffer(const gvk::Device& gvkDevice, VkDeviceSize size, const uint8_t* pData, gvk::Buffer* pGvkBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(size ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Create a buffer of the specified size
        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        auto allocationCreateInfo = gvk::get_default<VmaAllocationCreateInfo>();
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &allocationCreateInfo, pGvkBuffer));

        // If data was provided, write that data to the buffer
        if (pData) {
            uint8_t* pMappedData = nullptr;
            gvk_result(vmaMapMemory(gvkDevice.get<VmaAllocator>(), pGvkBuffer->get<VmaAllocation>(), (void**)&pMappedData));
            memcpy(pMappedData, pData, size);
            vmaUnmapMemory(gvkDevice.get<VmaAllocator>(), pGvkBuffer->get<VmaAllocation>());
        }
    } gvk_result_scope_end;
    return gvkResult;
}

static VkResult write_shader_binding_table_to_buffer(const gvk::Device& gvkDevice, const std::vector<ShaderBindingTableEntry>& shaderBindingTable, gvk::Buffer* pGvkBuffer)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(!shaderBindingTable.empty() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Loop through shader binding table entries and write each entry's handle and
        //  data to a std::vector<>
        std::vector<uint8_t> shaderBindingTableData;
        auto stride = shaderBindingTable[0].get_stride();
        shaderBindingTableData.reserve(shaderBindingTable.size() * stride);
        for (const auto& entry : shaderBindingTable) {
            shaderBindingTableData.insert(shaderBindingTableData.end(), entry.handle.begin(), entry.handle.end());
            gvk_result(entry.get_stride() == stride ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            shaderBindingTableData.insert(shaderBindingTableData.end(), entry.data.begin(), entry.data.end());
        }

        // Create a buffer and intialize it with the shader binding table data
        gvk_result((shaderBindingTableData.size() == shaderBindingTable.size() * stride) ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(create_buffer(gvkDevice, shaderBindingTableData.size(), shaderBindingTableData.data(), pGvkBuffer));
    } gvk_result_scope_end;
    return gvkResult;
}

static VkResult read_shader_binding_table_from_buffer(const gvk::Device& gvkDevice, const gvk::Buffer& gvkBuffer, VkDeviceSize stride, VkDeviceSize count, std::vector<ShaderBindingTableEntry>* pShaderBindingTable)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(gvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pShaderBindingTable ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        pShaderBindingTable->clear();

        // Map the given buffer and the contents into a std::vector<>
        uint8_t* pMappedData = nullptr;
        gvk_result(vmaMapMemory(gvkDevice.get<VmaAllocator>(), gvkBuffer.get<VmaAllocation>(), (void**)&pMappedData));
        gvk_result(stride * count == gvkBuffer.get<VkBufferCreateInfo>().size ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Populate shader binding table
        pShaderBindingTable->resize(count);
        auto shaderGroupHandleSize = get_shader_group_handle_size(gvkDevice);
        auto dataSize = stride - shaderGroupHandleSize;
        for (auto& entry : *pShaderBindingTable) {

            // Populate handle
            entry.handle.resize(shaderGroupHandleSize);
            memcpy(entry.handle.data(), pMappedData, shaderGroupHandleSize);
            pMappedData += shaderGroupHandleSize;

            // Populate data
            entry.data.resize(dataSize);
            memcpy(entry.data.data(), pMappedData, dataSize);
            pMappedData += dataSize;
        }

        // Unmap the buffer
        vmaUnmapMemory(gvkDevice.get<VmaAllocator>(), gvkBuffer.get<VmaAllocation>());
    } gvk_result_scope_end;
    return gvkResult;
}

TEST(spirv, ShaderGroupHandleMap)
{
    // Create gvk::spirv::validation::Context and check features
    gvk::spirv::validation::Context context;
    ASSERT_EQ(gvk::spirv::validation::Context::create(&context), VK_SUCCESS);
    if (get_shader_group_handle_size(context.get<gvk::Devices>()[0]) &&
        context.get_physical_device_8_bit_storage_features().storageBuffer8BitAccess &&
        context.get_physical_device_buffer_device_address_features().bufferDeviceAddress &&
        context.get_physical_device_shader_float_16_int_8_features().shaderInt8
    ) {
        const auto& gvkDevice = context.get<gvk::Devices>()[0];
        const auto& gvkQueue = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0];
        const auto& gvkCommandBuffer = context.get<gvk::CommandBuffers>()[0];

        // Create GPU address map pipeline
        gvk::Pipeline gpuAddressMapPipeline;
        ASSERT_EQ(gvk::create_shader_group_handle_map_pipeline(gvkDevice, &gpuAddressMapPipeline), VK_SUCCESS);

        // Create GPU memcpy pipeline
        gvk::Pipeline gpuMemcpyPipeline;
        ASSERT_EQ(gvk::create_gpu_memcpy_pipeline(gvkDevice, &gpuMemcpyPipeline), VK_SUCCESS);

        // Random number generator
        std::mt19937 rng(std::random_device{ }());
        std::uniform_int_distribution<size_t> sizeDistribution(4, 32);
        std::uniform_int_distribution<uint32_t> valueDistribution(0, 255);

        // Loop TestCount times, each time creating a synthetic shader binding table
        //  witth randomly sized handles and data, and populated with randomized data
        const size_t TestCount = 32;
        for (size_t i = 0; i < TestCount; ++i) {

            // Create Src shader binding table and shader group handle map
            auto dataSize = sizeDistribution(rng);
            std::map<std::vector<uint8_t>, std::vector<uint8_t>> keyValuePairs;
            std::vector<ShaderBindingTableEntry> srcShaderBindingTable(sizeDistribution(rng));
            for (auto& entry : srcShaderBindingTable) {
                auto shaderGroupHandle = get_random_shader_group_handle(gvkDevice, valueDistribution, rng);
                keyValuePairs[shaderGroupHandle] = get_random_shader_group_handle(gvkDevice, valueDistribution, rng);
                entry.handle = shaderGroupHandle;
                entry.data.resize(i ? dataSize : 0);
                for (auto& byte : entry.data) {
                    byte = (uint8_t)valueDistribution(rng);
                }
            }

            // Validate that ShaderBindingTable copmares for equality correctly
            auto dstShaderBindingTable = srcShaderBindingTable;
            EXPECT_EQ(dstShaderBindingTable, srcShaderBindingTable);

            // Validate that the keyValuePairs std::map<> and srcShaderBindingTable have the
            //  same number of entries
            ASSERT_EQ(srcShaderBindingTable.size(), keyValuePairs.size());

            // Loop over dstShaderBindingTable, which is equal to srcShaderBindingTable
            //  before this loop.  Look up the handle, and replace it with the handle
            //  value from the keyValuePairs std::map<>.
            for (auto& entry : dstShaderBindingTable) {
                auto shaderGroupHandleItr = keyValuePairs.find(entry.handle);
                ASSERT_NE(shaderGroupHandleItr, keyValuePairs.end());
                entry.handle = shaderGroupHandleItr->second;
            }

            // Validate dstShaderBindingTable and srcShaderBindingTable are no longer equal
            EXPECT_NE(dstShaderBindingTable, srcShaderBindingTable);

            // Create two buffers, write srcShaderBindingTable to srcBuffer and leave
            //  dstBuffer empty.
            // NOTE : That dstShaderBindingTable _is not_ written to a buffer...it's used to
            //  compare the result of the shader group handle mapping at the end of the test
            gvk::Buffer srcBuffer;
            gvk::Buffer dstBuffer;
            ASSERT_EQ(write_shader_binding_table_to_buffer(gvkDevice, srcShaderBindingTable, &srcBuffer), VK_SUCCESS);
            ASSERT_EQ(create_buffer(gvkDevice, srcBuffer.get<VkBufferCreateInfo>().size, nullptr, &dstBuffer), VK_SUCCESS);

            // Validate the shader binding table was correctly written to the src buffer
            std::vector<ShaderBindingTableEntry> validateShaderBindingTableReadback;
            ASSERT_EQ(read_shader_binding_table_from_buffer(gvkDevice, srcBuffer, srcShaderBindingTable[0].get_stride(), srcShaderBindingTable.size(), &validateShaderBindingTableReadback), VK_SUCCESS);
            EXPECT_EQ(srcShaderBindingTable, validateShaderBindingTableReadback);

            // Copy the src buffer to the dst buffer
            auto gpuMemcpyInfo = gvk::get_default<gvk::GpuMemcpyInfo>();
            gpuMemcpyInfo.dst = gvk::spirv::validation::get_buffer_device_address(gvkDevice, dstBuffer);
            gpuMemcpyInfo.src = gvk::spirv::validation::get_buffer_device_address(gvkDevice, srcBuffer);
            gpuMemcpyInfo.size = srcBuffer.get<VkBufferCreateInfo>().size;
            ASSERT_EQ(gvk::execute_gpu_memcpy(gvkDevice, gvkQueue, gvkCommandBuffer, VK_NULL_HANDLE, &gpuMemcpyInfo, gpuMemcpyPipeline), VK_SUCCESS);

            // Validate the shader binding table was correctly copied to the dst buffer
            validateShaderBindingTableReadback.clear();
            ASSERT_EQ(read_shader_binding_table_from_buffer(gvkDevice, dstBuffer, srcShaderBindingTable[0].get_stride(), srcShaderBindingTable.size(), &validateShaderBindingTableReadback), VK_SUCCESS);
            EXPECT_EQ(srcShaderBindingTable, validateShaderBindingTableReadback);

            // Create the shader group handle map data
            std::vector<uint8_t> keys;
            keys.reserve(keyValuePairs.size() * get_shader_group_handle_size(gvkDevice));
            std::vector<uint8_t> values;
            values.reserve(keys.size());
            for (const auto& shaderGroupHandleItr : keyValuePairs) {
                keys.insert(keys.end(), shaderGroupHandleItr.first.begin(), shaderGroupHandleItr.first.end());
                values.insert(values.end(), shaderGroupHandleItr.second.begin(), shaderGroupHandleItr.second.end());
            }

            // Create the gvk::ShaderGroupHandleMap
            auto shaderGroupHandleMapCreateInfo = gvk::get_default<gvk::ShaderGroupHandleMapCreateInfo>();
            shaderGroupHandleMapCreateInfo.pShaderGroupHandleKeys = keys.data();
            shaderGroupHandleMapCreateInfo.pShaderGroupHandleValues = values.data();
            shaderGroupHandleMapCreateInfo.shaderGroupHandleCount = (uint32_t)keyValuePairs.size();
            gvk::ShaderGroupHandleMap shaderGroupHandleMap;
            ASSERT_EQ(gvk::create_shader_group_handle_map(gvkDevice, &shaderGroupHandleMapCreateInfo, &shaderGroupHandleMap), VK_SUCCESS);

            // Execute shader group map, this will run through dstBuffer and replace the
            //  srcBuffer shader group key handles with the value handles
            auto gpuAddressMapInfo = gvk::get_default<gvk::GpuAddressMapInfo>();
            gpuAddressMapInfo.dst = gvk::spirv::validation::get_buffer_device_address(gvkDevice, dstBuffer);
            gpuAddressMapInfo.src = gvk::spirv::validation::get_buffer_device_address(gvkDevice, srcBuffer);
            gpuAddressMapInfo.stride = srcShaderBindingTable[0].get_stride();
            gpuAddressMapInfo.count = srcShaderBindingTable.size();
            gpuAddressMapInfo.keys = shaderGroupHandleMap.keys;
            gpuAddressMapInfo.values = shaderGroupHandleMap.values;
            gpuAddressMapInfo.kvpCount = shaderGroupHandleMap.kvpCount;
            ASSERT_EQ(gvk::execute_shader_group_handle_map(gvkDevice, gvkQueue, gvkCommandBuffer, VK_NULL_HANDLE, &gpuAddressMapInfo, gpuAddressMapPipeline), VK_SUCCESS);

            // Validate that contents of dstBuffer are equivalent to dstShaderBindingTable
            validateShaderBindingTableReadback.clear();
            ASSERT_EQ(read_shader_binding_table_from_buffer(gvkDevice, dstBuffer, dstShaderBindingTable[0].get_stride(), dstShaderBindingTable.size(), &validateShaderBindingTableReadback), VK_SUCCESS);
            EXPECT_EQ(dstShaderBindingTable, validateShaderBindingTableReadback);
        }
    }
}
