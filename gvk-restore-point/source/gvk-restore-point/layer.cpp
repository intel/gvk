
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

#include "gvk-restore-point/layer.hpp"
#include "gvk-layer/registry.hpp"
#include "gvk-restore-point/applier.hpp"
#include "gvk-restore-point/creator.hpp"
#include "gvk-structures/auto.hpp"
#include "gvk-structures/defaults.hpp"
#include "gvk-structures/pnext.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-environment.hpp"

// TODO : Straighten out layer interfaces...
#define VK_LAYER_INTEL_gvk_state_tracker_hpp_IMPLEMENTATION
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#include "gvk-restore-point/utilities.hpp"

#include <cassert>
#include <fstream>
#include <unordered_set>
#include <vector>

namespace gvk {
namespace restore_point {

VkResult Layer::create_restore_point(VkInstance instance, const GvkRestorePointCreateInfo* pCreateInfo, GvkRestorePoint* pRestorePoint)
{
    assert(instance);
    assert(pCreateInfo);
    assert(pRestorePoint);

    // Allocate a handle for the GvkRestorePoint
    *pRestorePoint = new GvkRestorePoint_T;
    auto inserted = get_restore_points().insert(*pRestorePoint).second;
    (void)inserted;
    assert(inserted);

    // Setup CreateInfo
    CreateInfo createInfo { };
    createInfo.instance = instance;
    createInfo.gvkRestorePoint = *pRestorePoint;
    createInfo.threadCount = 0; // TODO : Enable user control...pCreateInfo->threadCount
    createInfo.pfnInitializeThreadCallback = pCreateInfo->pfnInitializeThreadCallback;
    createInfo.pfnAllocateResourceDataCallback = pCreateInfo->pfnAllocateResourceDataCallback;
    createInfo.pfnProcessResourceDataCallback = pCreateInfo->pfnProcessResourceDataCallback;

    // Set flags
    (*pRestorePoint)->createFlags = pCreateInfo->flags ? pCreateInfo->flags :
        GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT |
        GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT |
        GVK_RESTORE_POINT_CREATE_BUFFER_DATA_BIT |
        GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT |
        GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT;
    if ((*pRestorePoint)->createFlags & GVK_RESTORE_POINT_CREATE_DYNAMIC_DATA_BIT) {
        (*pRestorePoint)->createFlags |= GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT;
    }

    // Set path
    if (pCreateInfo->pPath) {
        createInfo.path = pCreateInfo->pPath;
    } else if (pCreateInfo->pwPath) {
        createInfo.path = pCreateInfo->pwPath;
    }
    if (createInfo.path.empty()) {
        createInfo.path = "gvk-restore-point";
    }

    // Create the GvkRestorePoint
    return Creator().create_restore_point(createInfo);
}

VkResult Layer::apply_restore_point(VkInstance instance, const GvkRestorePointApplyInfo* pApplyInfo, GvkRestorePoint restorePoint)
{
    assert(instance);
    assert(pApplyInfo);
    assert(restorePoint);

    // Setup ApplyInfo
    ApplyInfo applyInfo{ };
    applyInfo.flags = pApplyInfo->flags;
    applyInfo.vkInstance = instance;
    applyInfo.gvkRestorePoint = restorePoint;
    applyInfo.threadCount = pApplyInfo->threadCount;

    // Set path
    if (pApplyInfo->pPath) {
        applyInfo.path = pApplyInfo->pPath;
    } else if (pApplyInfo->pwPath) {
        applyInfo.path = pApplyInfo->pwPath;
    }

    // Loop over exclude objects, if handles are specified then those objects should
    //  be excluded from all GvkRestorePoint logic, if handles aren't specified then
    //  any objects of the specified type will be excluded
    assert(!pApplyInfo->excludeObjectCount == !pApplyInfo->pExcludeObjects);
    for (uint32_t i = 0; i < pApplyInfo->excludeObjectCount; ++i) {
        const auto& excludedObject = pApplyInfo->pExcludeObjects[i];
        assert(!excludedObject.handle == !excludedObject.dispatchableHandle);
        if (excludedObject.handle) {
            applyInfo.excludeObjects.insert(excludedObject);
        } else {
            applyInfo.excludeObjectTypes.insert(excludedObject.type);
        }
    }

    // Set destroy objects
    if (pApplyInfo->destroyObjectCount && pApplyInfo->pDestroyObjects) {
        auto pBegin = pApplyInfo->pDestroyObjects;
        auto pEnd = pApplyInfo->pDestroyObjects + pApplyInfo->destroyObjectCount;
        applyInfo.destroyObjects.insert(pBegin, pEnd);
    }

    // Set callbacks
    applyInfo.pfnInitializeThreadCallback = pApplyInfo->pfnInitializeThreadCallback;
    applyInfo.pfnProcessResourceDataCallback = pApplyInfo->pfnProcessResourceDataCallback;
    applyInfo.pfnProcessRestoredObjectCallback = pApplyInfo->pfnProcessRestoredObjectCallback;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    applyInfo.pfnProcessWin32SurfaceCreateInfoCallback = pApplyInfo->pfnProcessWin32SurfaceCreateInfoCallback;
#endif

    // Setup dispatch table.  Use pfnGetInstanceProcAddr if provided, otherwise load
    //  global entry points to set gvkGetInstanceProcAddr
    if (pApplyInfo->pfnGetInstanceProcAddr) {
        applyInfo.dispatchTable.gvkGetInstanceProcAddr = pApplyInfo->pfnGetInstanceProcAddr;
    } else {
        DispatchTable::load_global_entry_points(&applyInfo.dispatchTable);
    }
    DispatchTable::load_instance_entry_points(instance, &applyInfo.dispatchTable);

    // Disable this layer's hooks for the duration of restore point application.
    //  This layer sits beneath the state tracker.  The calls executed here use the
    //  application's dispatch table so that all layers treat them the same as calls
    //  coming from the workload, but we don't want those calls landing in restore
    //  point hooks.
    state_tracker::load_layer_entry_points();
    assert(layer::Registry::get().layers.size() == 1);
    layer::Registry::get().layers.back()->enabled = false;
    auto vkResult = !applyInfo.path.empty() ? Applier().apply_restore_point(applyInfo) : VK_ERROR_INITIALIZATION_FAILED;
    layer::Registry::get().layers.back()->enabled = true;
    return vkResult;
}

VkResult Layer::get_restore_point_manifest(VkInstance instance, GvkRestorePoint restorePoint, GvkRestorePointManifest* pManifest)
{
    (void)instance;
    assert(instance);
    assert(restorePoint);
    assert(pManifest);
    *pManifest = restorePoint->manifest;
    return VK_SUCCESS;
}

void Layer::destroy_restore_point(VkInstance instance, GvkRestorePoint restorePoint)
{
    (void)instance;
    assert(instance);
    delete restorePoint;
    auto erased = get_restore_points().erase(restorePoint);
    (void)erased;
    assert(erased);
}

} // namespace restore_point
} // namespace gvk

namespace gvk {
namespace layer {

void on_load(Registry& registry)
{
    registry.layers.push_back(std::make_unique<restore_point::Layer>());
}

} // namespace layer
} // namespace gvk

extern "C" {

VkResult VKAPI_CALL gvkCreateRestorePoint(VkInstance instance, const GvkRestorePointCreateInfo* pCreateInfo, GvkRestorePoint* pRestorePoint)
{
    return gvk::restore_point::Layer::create_restore_point(instance, pCreateInfo, pRestorePoint);
}

VkResult VKAPI_CALL gvkApplyRestorePoint(VkInstance instance, const GvkRestorePointApplyInfo* pApplyInfo, GvkRestorePoint restorePoint)
{
    return gvk::restore_point::Layer::apply_restore_point(instance, pApplyInfo, restorePoint);
}

VkResult gvkGetRestorePointManifest(VkInstance instance, GvkRestorePoint restorePoint, GvkRestorePointManifest* pManifest)
{
    return gvk::restore_point::Layer::get_restore_point_manifest(instance, restorePoint, pManifest);
}

void VKAPI_CALL gvkDestroyRestorePoint(VkInstance instance, GvkRestorePoint restorePoint)
{
    gvk::restore_point::Layer::destroy_restore_point(instance, restorePoint);
}

VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pNegotiateLayerInterface)
{
    assert(pNegotiateLayerInterface);
    pNegotiateLayerInterface->pfnGetInstanceProcAddr = gvk::layer::get_instance_proc_addr;
    pNegotiateLayerInterface->pfnGetPhysicalDeviceProcAddr = gvk::layer::get_physical_device_proc_addr;
    pNegotiateLayerInterface->pfnGetDeviceProcAddr = gvk::layer::get_device_proc_addr;
    return VK_SUCCESS;
}

} // extern "C"
