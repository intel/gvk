
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

static VkResult create_buffer(const gvk::Device& gvkDevice, VkDeviceSize size, const uint8_t* pData, gvk::Buffer* pGvkBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(size ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pData ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        auto allocationCreateInfo = gvk::get_default<VmaAllocationCreateInfo>();
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &allocationCreateInfo, pGvkBuffer));

        uint8_t* pMappedData = nullptr;
        gvk_result(vmaMapMemory(gvkDevice.get<VmaAllocator>(), pGvkBuffer->get<VmaAllocation>(), (void**)&pMappedData));
        memcpy(pMappedData, pData, size);
        vmaUnmapMemory(gvkDevice.get<VmaAllocator>(), pGvkBuffer->get<VmaAllocation>());
    } gvk_result_scope_end;
    return gvkResult;
}

static VkResult create_acceleration_structure_buffer(const gvk::Device& gvkDevice, const VkAccelerationStructureBuildSizesInfoKHR* pBuildSizeInfo, gvk::Buffer* pGvkBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pBuildSizeInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = pBuildSizeInfo->accelerationStructureSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
        auto allocationCreateInfo = gvk::get_default<VmaAllocationCreateInfo>();
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &allocationCreateInfo, pGvkBuffer));
    } gvk_result_scope_end;
    return gvkResult;
}

static VkResult create_scratch_buffer(const gvk::Device& gvkDevice, VkDeviceSize size, gvk::Buffer* pGvkBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(size ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        auto allocationCreateInfo = gvk::get_default<VmaAllocationCreateInfo>();
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &allocationCreateInfo, pGvkBuffer));
    } gvk_result_scope_end;
    return gvkResult;
}

static uint64_t get_buffer_device_address(const gvk::Device& gvkDevice, const gvk::Buffer& gvkBuffer)
{
    assert(gvkDevice);
    assert(gvkBuffer);
    auto bufferDeviceAddressInfo = gvk::get_default<VkBufferDeviceAddressInfoKHR>();
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = gvkBuffer;
    return gvkDevice.GetBufferDeviceAddressKHR(&bufferDeviceAddressInfo);
}

VkResult create_acceleration_structure(const gvk::Device& gvkDevice, const gvk::Queue& gvkQueue, const gvk::CommandBuffer& gvkCommandBuffer, gvk::AccelerationStructureKHR* pGvkAccelerationStructure)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(gvkQueue ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(gvkCommandBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkAccelerationStructure ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        struct Vertex {
            float position[3];
        };
        std::vector<Vertex> vertices {
            { {  1.0f,  1.0f, 0.0f } },
            { { -1.0f,  1.0f, 0.0f } },
            { {  0.0f, -1.0f, 0.0f } }
        };

        std::vector<uint32_t> indices { 0, 1, 2 };

        VkTransformMatrixKHR transformMatrix {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
        };

        gvk::Buffer vertexBuffer;
        gvk_result(create_buffer(gvkDevice, vertices.size() * sizeof(Vertex), (const uint8_t*)vertices.data(), &vertexBuffer));
        gvk::Buffer indexBuffer;
        gvk_result(create_buffer(gvkDevice, indices.size() * sizeof(uint32_t), (const uint8_t*)indices.data(), &indexBuffer));
        gvk::Buffer transformBuffer;
        gvk_result(create_buffer(gvkDevice, sizeof(VkTransformMatrixKHR), (const uint8_t*)&transformMatrix, &transformBuffer));

        VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{ };
        VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{ };
        VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{ };

        vertexBufferDeviceAddress.deviceAddress = get_buffer_device_address(gvkDevice, vertexBuffer);
        indexBufferDeviceAddress.deviceAddress = get_buffer_device_address(gvkDevice, indexBuffer);
        transformBufferDeviceAddress.deviceAddress = get_buffer_device_address(gvkDevice, transformBuffer);

        auto accelerationStructureGeometry = gvk::get_default<VkAccelerationStructureGeometryKHR>();
        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
        accelerationStructureGeometry.geometry.triangles.maxVertex = 2;
        accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
        accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
        accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
        accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
        accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

        auto accelerationStructureBuildGeometryInfo = gvk::get_default<VkAccelerationStructureBuildGeometryInfoKHR>();
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.geometryCount = 1;
        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

        const uint32_t triangleCount = 1;
        auto accelerationStructureBuildSizesInfo = gvk::get_default<VkAccelerationStructureBuildSizesInfoKHR>();
        gvkDevice.GetAccelerationStructureBuildSizesKHR(
            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &accelerationStructureBuildGeometryInfo,
            &triangleCount,
            &accelerationStructureBuildSizesInfo
        );

        gvk::Buffer accelerationStructureBuffer;
        gvk_result(create_acceleration_structure_buffer(gvkDevice, &accelerationStructureBuildSizesInfo, &accelerationStructureBuffer));

        auto accelerationStructureCreateInfo = gvk::get_default<VkAccelerationStructureCreateInfoKHR>();
        accelerationStructureCreateInfo.buffer = accelerationStructureBuffer;
        accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        gvk_result(gvk::AccelerationStructureKHR::create(gvkDevice, &accelerationStructureCreateInfo, nullptr, pGvkAccelerationStructure));

        gvk::Buffer scratchBuffer;
        gvk_result(create_scratch_buffer(gvkDevice, accelerationStructureBuildSizesInfo.buildScratchSize, &scratchBuffer));

        accelerationStructureBuildGeometryInfo = gvk::get_default<VkAccelerationStructureBuildGeometryInfoKHR>();
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationStructureBuildGeometryInfo.dstAccelerationStructure = *pGvkAccelerationStructure;
        accelerationStructureBuildGeometryInfo.geometryCount = 1;
        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
        accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = get_buffer_device_address(gvkDevice, scratchBuffer);

        auto accelerationStructureBuildRangeInfo = gvk::get_default<VkAccelerationStructureBuildRangeInfoKHR>();
        accelerationStructureBuildRangeInfo.primitiveCount = triangleCount;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

        gvk_result(gvk::execute_immediately(gvkDevice, gvkQueue, gvkCommandBuffer, VK_NULL_HANDLE,
            [&](auto)
            {
                gvkCommandBuffer.CmdBuildAccelerationStructuresKHR(1, &accelerationStructureBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
            }
        ));
    } gvk_result_scope_end;
    return gvkResult;
}

TEST(AccelerationStructure, Placeholder)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    if (context.get_physical_device_acceleration_structure_features().accelerationStructure) {

        // Create acceleration structure
        gvk::AccelerationStructureKHR gvkAccelerationStructure;
        ASSERT_EQ(create_acceleration_structure(
            context.get<gvk::Devices>()[0],
            gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0],
            context.get<gvk::CommandBuffers>()[0],
            &gvkAccelerationStructure
        ), VK_SUCCESS);

        // Hit the state tracker to get acceleration structure info
        // Expect the result to be untouched since placeholderValue is left as 0
        auto stateTrackedAcclerationStructure = gvk::get_state_tracked_object(gvkAccelerationStructure);
        auto accelerationStructureGeometryRequestInfo = gvk::get_default<GvkAccelerationStructureGeometryRequestInfo>();
        auto acclerationstructureGeometryResultInfo = gvk::get_default<GvkAcclerationstructureGeometryResultInfo>();
        gvkGetStateTrackedAccelerationStructureGeometryInfo(&stateTrackedAcclerationStructure, &accelerationStructureGeometryRequestInfo, &acclerationstructureGeometryResultInfo);
        EXPECT_EQ(acclerationstructureGeometryResultInfo.placeholderDataSize, 0);

        // After setting placeholderValue to 1, expect placeholderDataSize to be equal
        //  to VkAccelerationStructureCreateInfoKHR::size
        accelerationStructureGeometryRequestInfo.placeholderValue = 1;
        gvkGetStateTrackedAccelerationStructureGeometryInfo(&stateTrackedAcclerationStructure, &accelerationStructureGeometryRequestInfo, &acclerationstructureGeometryResultInfo);
        EXPECT_EQ(acclerationstructureGeometryResultInfo.placeholderDataSize, gvkAccelerationStructure.get<VkAccelerationStructureCreateInfoKHR>().size);
    }
}
