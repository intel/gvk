
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

#include "gvk-state-tracker/state-tracker.hpp"
#include "gvk-layer/registry.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

// NOTE : Cache the info arg so any changes made by this layer, or layers down
//  the chain, can be reverted before returning control to the application.
thread_local VkAccelerationStructureCreateInfoKHR tlApplicationAccelerationStructureCreateInfo;
VkResult StateTracker::pre_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure, VkResult gvkResult)
{
    assert(pCreateInfo);
    tlApplicationAccelerationStructureCreateInfo = *pCreateInfo;
    return BasicStateTracker::pre_vkCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure, gvkResult);
}

VkResult StateTracker::post_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure, VkResult gvkResult)
{
    assert(pCreateInfo);
    gvkResult = BasicStateTracker::post_vkCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure, gvkResult);
    if (gvkResult == VK_SUCCESS && pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR) {
        AccelerationStructureKHR gvkAccelerationStructure({ device, *pAccelerationStructure });
        assert(gvkAccelerationStructure);
        assert(gvkAccelerationStructure.mReference);
        auto& accelerationStructureControlBlock = gvkAccelerationStructure.mReference.get_obj();
        auto& accelerationStructureCreateInfo = const_cast<VkAccelerationStructureCreateInfoKHR&>(*accelerationStructureControlBlock.mAccelerationStructureCreateInfoKHR);

        const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(device));
        assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
        const auto& dispatchTable = dispatchTableItr->second;

        auto accelerationStructureDeviceAddressInfo = get_default<VkAccelerationStructureDeviceAddressInfoKHR>();
        accelerationStructureDeviceAddressInfo.accelerationStructure = *pAccelerationStructure;
        auto deviceAddress = dispatchTable.gvkGetAccelerationStructureDeviceAddressKHR(device, &accelerationStructureDeviceAddressInfo);
        assert(!accelerationStructureCreateInfo.deviceAddress || accelerationStructureCreateInfo.deviceAddress == deviceAddress);
        accelerationStructureCreateInfo.deviceAddress = deviceAddress;
    }
    *const_cast<VkAccelerationStructureCreateInfoKHR*>(pCreateInfo) = tlApplicationAccelerationStructureCreateInfo;
    return gvkResult;
}

VkResult StateTracker::pre_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, VkResult gvkResult)
{
    (void)device;
    (void)deferredOperation;
    (void)infoCount;
    (void)pInfos;
    (void)ppBuildRangeInfos;
    return gvkResult;
}

VkResult StateTracker::post_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, VkResult gvkResult)
{
    (void)device;
    (void)deferredOperation;
    (void)infoCount;
    (void)pInfos;
    (void)ppBuildRangeInfos;
    return gvkResult;
}

VkResult StateTracker::process_build_acceleration_structures(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
    assert(false && "TODO : Acceleration structure history");

    (void)device;
    (void)deferredOperation;
    (void)infoCount;
    (void)pInfos;
    (void)ppBuildRangeInfos;

    const auto& deviceAddressTracker = Device(device).get<DeviceAddressTracker>();
    (void)deviceAddressTracker;
    for (uint32_t info_i = 0; info_i < infoCount; ++info_i) {
        auto& buildGeometryInfo = pInfos[info_i];
        auto pBuildRangeInfo = ppBuildRangeInfos[info_i];
        AccelerationStructureKHR accelerationStructure({ device, buildGeometryInfo.dstAccelerationStructure });
        assert(accelerationStructure);
        auto& controlBlock = accelerationStructure.mReference.get_obj();

        std::set<Buffer> buffers;
        auto getBuffer = [&](VkDeviceAddress deviceAddress)
        {
            (void)deviceAddress;
            #if 0
            // TODO : Find VkBuffer using its device address and create dependency
            Buffer buffer({ device, deviceAddressTracker.get_buffer(deviceAddress) });
            if (buffer) {
                buffers.insert(buffer);
            }
            #endif
        };

        getBuffer(buildGeometryInfo.scratchData.deviceAddress);
        controlBlock.mBuildGeometryInfo = buildGeometryInfo;
        controlBlock.mBuildRangeInfos.reserve(buildGeometryInfo.geometryCount);
        for (uint32_t geomtry_i = 0; geomtry_i < buildGeometryInfo.geometryCount; ++geomtry_i) {
            controlBlock.mBuildRangeInfos.push_back(pBuildRangeInfo[geomtry_i]);
            auto pGeometry = buildGeometryInfo.pGeometries ? &buildGeometryInfo.pGeometries[geomtry_i] : buildGeometryInfo.ppGeometries[geomtry_i];
            switch (pGeometry->geometryType) {
            case VK_GEOMETRY_TYPE_TRIANGLES_KHR: {
                const auto& triangles = pGeometry->geometry.triangles;
                assert(!triangles.pNext && "Unserviced pNext; gvk maintenance required");
                getBuffer(triangles.vertexData.deviceAddress);
                getBuffer(triangles.indexData.deviceAddress);
                getBuffer(triangles.transformData.deviceAddress);
            } break;
            case VK_GEOMETRY_TYPE_AABBS_KHR: {
                const auto& aabbs = pGeometry->geometry.aabbs;
                assert(!aabbs.pNext && "Unserviced pNext; gvk maintenance required");
                getBuffer(aabbs.data.deviceAddress);
            } break;
            case VK_GEOMETRY_TYPE_INSTANCES_KHR: {
                const auto& instances = pGeometry->geometry.instances;
                assert(!instances.pNext && "Unserviced pNext; gvk maintenance required");
                getBuffer(instances.data.deviceAddress);
            } break;
            default: {
                assert(false && "Unserviced VkGeometryTypeKHR; gvk maintenance required");
            } break;
            }
        }
        controlBlock.mBuildBuffers.clear();
        for (const auto& buffer : buffers) {
            controlBlock.mBuildBuffers.push_back(buffer);
        }
    }
    return VK_SUCCESS;
}

} // namespace state_tracker
} // namespace gvk
