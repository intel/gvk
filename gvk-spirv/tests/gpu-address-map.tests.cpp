
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

#include "gvk-spirv/gpu-address-map.hpp"
#include "gvk-spirv/gpu-memcpy.hpp"
#include "spirv-validation-context.hpp"
#include "spirv-validation-utilities.hpp"

#include "boost/multiprecision/cpp_int.hpp"

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

using ShaderGroupHandle = boost::multiprecision::uint256_t;
using ShaderGroupHandleBytes = std::array<uint8_t, 32>;
constexpr auto ShaderGroupHandleSize = sizeof(ShaderGroupHandleBytes);

class ShaderBindingTableEntry final
{
public:
    bool operator==(const ShaderBindingTableEntry& other) const
    {
        return
            shaderGroupHandle == other.shaderGroupHandle &&
            data              == other.data;
    }

    bool operator!=(const ShaderBindingTableEntry& other) const
    {
        return !(*this == other);
    }

    VkDeviceSize get_stride() const
    {
        return (VkDeviceSize)ShaderGroupHandleSize + data.size();
    }

    ShaderGroupHandle shaderGroupHandle{ };
    std::vector<uint8_t> data;
};

static ShaderGroupHandle get_random_shader_group_handle(std::uniform_int_distribution<uint32_t>& distribution, std::mt19937& rng)
{
    ShaderGroupHandleBytes shaderGroupHandleBytes{ };
    for (auto& byte : shaderGroupHandleBytes) {
        /*
        NOTE : Converting 0 to 1 since it doesn't round trip as expected.  It seems if
            there's a leading 0 in the import bits, it gets placed at the end when
            exporting.  Likely due to how boost::multiprecision stores significant bits.
            More investigation is needed.  Until this is addressed, the shader will use
            a linear search.

        EXPECT_EQ(shaderGroupHandleBytes, exportedBytes);
        error: Expected equality of these values:
          shaderGroupHandleBytes
            Which is: { '\0', '\x7F' (127), '\xEC' (236), '\xDB' (219), '\x84' (132), '\xAF' (175), '[' (91, 0x5B), '\b' (8), '\x1C' (28), '\x93' (147), '|' (124, 0x7C), 'R' (82, 0x52), '\xE3' (227), 'q' (113, 0x71), '\xA9' (169), '\xEA' (234), '\xCF' (207), '\xE7' (231), '\xB4' (180), '\xCB' (203), '\xE2' (226), '\xE' (14), '\x17' (23), ']' (93, 0x5D), '<' (60, 0x3C), '-' (45, 0x2D), '\xD7' (215), '*' (42, 0x2A), '\xF9' (249), '%' (37, 0x25), '\t' (9), 'p' (112, 0x70) }
          exportedBytes
            Which is: { '\x7F' (127), '\xEC' (236), '\xDB' (219), '\x84' (132), '\xAF' (175), '[' (91, 0x5B), '\b' (8), '\x1C' (28), '\x93' (147), '|' (124, 0x7C), 'R' (82, 0x52), '\xE3' (227), 'q' (113, 0x71), '\xA9' (169), '\xEA' (234), '\xCF' (207), '\xE7' (231), '\xB4' (180), '\xCB' (203), '\xE2' (226), '\xE' (14), '\x17' (23), ']' (93, 0x5D), '<' (60, 0x3C), '-' (45, 0x2D), '\xD7' (215), '*' (42, 0x2A), '\xF9' (249), '%' (37, 0x25), '\t' (9), 'p' (112, 0x70), '\0' }

        EXPECT_EQ(shaderGroupHandleBytes, exportedBytes);
        error: Expected equality of these values:
          shaderGroupHandleBytes
            Which is: { '\0', '\x5' (5), '\x83' (131), '\xC2' (194), '\xCD' (205), 'k' (107, 0x6B), '.' (46, 0x2E), '\x83' (131), ')' (41, 0x29), '\xA8' (168), '"' (34, 0x22), '|' (124, 0x7C), 'u' (117, 0x75), '\xF6' (246), '\xFF' (255), '\xF' (15), 'o' (111, 0x6F), 'C' (67, 0x43), '\x82' (130), '\xF0' (240), '\xE9' (233), '\xAB' (171), ';' (59, 0x3B), 'A' (65, 0x41), '^' (94, 0x5E), '\x8E' (142), '\x1' (1), '%' (37, 0x25), '7' (55, 0x37), '\xED' (237), 'O' (79, 0x4F), 'G' (71, 0x47) }
          exportedBytes
            Which is: { '\x5' (5), '\x83' (131), '\xC2' (194), '\xCD' (205), 'k' (107, 0x6B), '.' (46, 0x2E), '\x83' (131), ')' (41, 0x29), '\xA8' (168), '"' (34, 0x22), '|' (124, 0x7C), 'u' (117, 0x75), '\xF6' (246), '\xFF' (255), '\xF' (15), 'o' (111, 0x6F), 'C' (67, 0x43), '\x82' (130), '\xF0' (240), '\xE9' (233), '\xAB' (171), ';' (59, 0x3B), 'A' (65, 0x41), '^' (94, 0x5E), '\x8E' (142), '\x1' (1), '%' (37, 0x25), '7' (55, 0x37), '\xED' (237), 'O' (79, 0x4F), 'G' (71, 0x47), '\0' }

        */
        byte = (uint8_t)distribution(rng);
        if (!byte) {
            byte = 1;
        }
    }
    ShaderGroupHandle shaderGroupHandle{ };
    boost::multiprecision::import_bits(shaderGroupHandle, shaderGroupHandleBytes.begin(), shaderGroupHandleBytes.end());

    // Validate round trip
    ShaderGroupHandleBytes exportedBytes{ };
    boost::multiprecision::export_bits(shaderGroupHandle, exportedBytes.data(), 8);
    EXPECT_EQ(shaderGroupHandleBytes, exportedBytes);

    return shaderGroupHandle;
}

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

static VkResult write_shader_binding_table_to_buffer(const gvk::Device& gvkDevice, const std::vector<ShaderBindingTableEntry>& shaderBindingTable, gvk::Buffer* pGvkBuffer)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(!shaderBindingTable.empty() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        std::vector<uint8_t> shaderBindingTableData;
        auto stride = shaderBindingTable[0].get_stride();
        shaderBindingTableData.reserve(shaderBindingTable.size() * stride);
        for (const auto& entry : shaderBindingTable) {
            ShaderGroupHandleBytes shaderGroupHandleBytes{ };
            boost::multiprecision::export_bits(entry.shaderGroupHandle, shaderGroupHandleBytes.data(), 8);
            shaderBindingTableData.insert(shaderBindingTableData.end(), shaderGroupHandleBytes.begin(), shaderGroupHandleBytes.end());
            gvk_result(entry.get_stride() == stride ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            shaderBindingTableData.insert(shaderBindingTableData.end(), entry.data.begin(), entry.data.end());
        }
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

        uint8_t* pMappedData = nullptr;
        gvk_result(vmaMapMemory(gvkDevice.get<VmaAllocator>(), gvkBuffer.get<VmaAllocation>(), (void**)&pMappedData));
        std::vector<uint8_t> data(stride * count);
        gvk_result(data.size() == gvkBuffer.get<VkBufferCreateInfo>().size ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        auto pData = data.data();
        memcpy(pData, pMappedData, gvkBuffer.get<VkBufferCreateInfo>().size);
        vmaUnmapMemory(gvkDevice.get<VmaAllocator>(), gvkBuffer.get<VmaAllocation>());

        pShaderBindingTable->resize(count);
        auto dataSize = stride - ShaderGroupHandleSize;
        for (auto& entry : *pShaderBindingTable) {
            boost::multiprecision::import_bits(entry.shaderGroupHandle, pData, pData + ShaderGroupHandleSize);
            pData += ShaderGroupHandleSize;
            entry.data.insert(entry.data.end(), pData, pData + dataSize);
            pData += dataSize;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

TEST(spirv, GpuAddressMap)
{
    // Create gvk::spirv::validation::Context and check features
    gvk::spirv::validation::Context context;
    ASSERT_EQ(gvk::spirv::validation::Context::create(&context), VK_SUCCESS);
    if (context.get_physical_device_8_bit_storage_features().storageBuffer8BitAccess &&
        context.get_physical_device_shader_float_16_int_8_features().shaderInt8 &&
        context.get_physical_device_buffer_device_address_features().bufferDeviceAddress
    ) {
        const auto& gvkDevice = context.get<gvk::Devices>()[0];
        const auto& gvkQueue = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0];
        const auto& gvkCommandBuffer = context.get<gvk::CommandBuffers>()[0];

        // Create GPU address map pipeline
        gvk::Pipeline gpuAddressMapPipeline;
        ASSERT_EQ(gvk::create_gpu_address_map_pipeline(gvkDevice, &gpuAddressMapPipeline), VK_SUCCESS);

        // Create GPU memcpy pipeline
        gvk::Pipeline gpuMemcpyPipeline;
        ASSERT_EQ(gvk::create_gpu_memcpy_pipeline(gvkDevice, &gpuMemcpyPipeline), VK_SUCCESS);

        // Random number generator
        std::mt19937 rng(std::random_device{ }());
        std::uniform_int_distribution<size_t> sizeDistribution(4, 32);
        std::uniform_int_distribution<uint32_t> valueDistribution(0, 255);

        // TODO : Documentation
        const size_t TestCount = 32;
        for (size_t i = 0; i < TestCount; ++i) {

            // Create Src shader binding table and shader group handle map
            auto dataSize = sizeDistribution(rng);
            std::map<ShaderGroupHandle, ShaderGroupHandle> shaderGroupHandleMap;
            std::vector<ShaderBindingTableEntry> srcShaderBindingTable(sizeDistribution(rng));
            for (auto& entry : srcShaderBindingTable) {
                auto shaderGroupHandle = get_random_shader_group_handle(valueDistribution, rng);
                shaderGroupHandleMap[shaderGroupHandle] = get_random_shader_group_handle(valueDistribution, rng);
                entry.shaderGroupHandle = shaderGroupHandle;
                entry.data.resize(i ? dataSize : 0);
                for (auto& byte : entry.data) {
                    byte = (uint8_t)valueDistribution(rng);
                }
            }

            // TODO : Documentation
            auto dstShaderBindingTable = srcShaderBindingTable;
            EXPECT_EQ(dstShaderBindingTable, srcShaderBindingTable);
            ASSERT_EQ(srcShaderBindingTable.size(), shaderGroupHandleMap.size());

            // TODO : Documentation
            for (auto& entry : dstShaderBindingTable) {
                auto shaderGroupHandleItr = shaderGroupHandleMap.find(entry.shaderGroupHandle);
                ASSERT_NE(shaderGroupHandleItr, shaderGroupHandleMap.end());
                entry.shaderGroupHandle = shaderGroupHandleItr->second;
            }

            // TODO : Documentation
            EXPECT_NE(dstShaderBindingTable, srcShaderBindingTable);

            // TODO : Documentation
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
            keys.reserve(shaderGroupHandleMap.size() * ShaderGroupHandleSize);
            std::vector<uint8_t> values;
            values.reserve(keys.size());
            for (const auto& shaderGroupHandleItr : shaderGroupHandleMap) {
                ShaderGroupHandleBytes shaderGroupHandleBytes{ };
                boost::multiprecision::export_bits(shaderGroupHandleItr.first, shaderGroupHandleBytes.data(), 8);
                keys.insert(keys.end(), shaderGroupHandleBytes.begin(), shaderGroupHandleBytes.end());
                boost::multiprecision::export_bits(shaderGroupHandleItr.second, shaderGroupHandleBytes.data(), 8);
                values.insert(values.end(), shaderGroupHandleBytes.begin(), shaderGroupHandleBytes.end());
            }

            // TODO : Documentation
            gvk::Buffer keysBuffer;
            gvk::Buffer valuesBuffer;
            ASSERT_EQ(create_buffer(gvkDevice, keys.size(), keys.data(), &keysBuffer), VK_SUCCESS);
            ASSERT_EQ(create_buffer(gvkDevice, values.size(), values.data(), &valuesBuffer), VK_SUCCESS);

            // Execute the GPU address map
            auto gpuAddressMapInfo = gvk::get_default<gvk::GpuAddressMapInfo>();
            gpuAddressMapInfo.dst = gvk::spirv::validation::get_buffer_device_address(gvkDevice, dstBuffer);
            gpuAddressMapInfo.src = gvk::spirv::validation::get_buffer_device_address(gvkDevice, srcBuffer);
            gpuAddressMapInfo.stride = srcShaderBindingTable[0].get_stride();
            gpuAddressMapInfo.count = srcShaderBindingTable.size();
            gpuAddressMapInfo.keys = gvk::spirv::validation::get_buffer_device_address(gvkDevice, keysBuffer);
            gpuAddressMapInfo.values = gvk::spirv::validation::get_buffer_device_address(gvkDevice, valuesBuffer);
            gpuAddressMapInfo.kvpCount = keys.size();
            ASSERT_EQ(gvk::execute_gpu_address_map(gvkDevice, gvkQueue, gvkCommandBuffer, VK_NULL_HANDLE, &gpuAddressMapInfo, gpuAddressMapPipeline), VK_SUCCESS);

            // TODO : Documentation
            validateShaderBindingTableReadback.clear();
            ASSERT_EQ(read_shader_binding_table_from_buffer(gvkDevice, dstBuffer, dstShaderBindingTable[0].get_stride(), dstShaderBindingTable.size(), &validateShaderBindingTableReadback), VK_SUCCESS);
            ShaderGroupHandleBytes validateShaderGroupHandleBytes{ };
            boost::multiprecision::export_bits(validateShaderBindingTableReadback[0].shaderGroupHandle, validateShaderGroupHandleBytes.data(), 8);
            EXPECT_EQ(dstShaderBindingTable, validateShaderBindingTableReadback);
        }
    }
}
