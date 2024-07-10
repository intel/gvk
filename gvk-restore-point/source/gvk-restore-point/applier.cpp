
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

#include "gvk-restore-point/applier.hpp"
#include "gvk-environment.hpp"
#include "gvk-format-info.hpp"
#include "gvk-layer.hpp"
#include "gvk-restore-info.hpp"
#include "gvk-structures.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-command-structures/generated/execute-command-structure.hpp"
#include "gvk-restore-point/generated/update-structure-handles.hpp"
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#include <fstream>
#include <vector>

namespace gvk {
namespace restore_point {

VkResult Applier::apply_restore_point(const ApplyInfo& applyInfo)
{
    mLog.set_instance(applyInfo.vkInstance);
    mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    mLog << "Entered gvk::restore_point::Applier::apply_restore_point()" << layer::Log::Flush;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        mApplyInfo = applyInfo;
        std::ifstream infoFile(mApplyInfo.path / "GvkRestorePointManifest.info", std::ios::binary);
        gvk_result(infoFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        deserialize(infoFile, nullptr, mApplyInfo.gvkRestorePoint->manifest);
        const auto& manifest = mApplyInfo.gvkRestorePoint->manifest;

        // Get the application dispatch table.  Its useful for VkPhysicalDevice calls
        //  using application VkPhysicalDevice handles (see gvk-layer/registry.cpp
        //  create_physical_device_mappings() for more info).
        // NOTE : Any Calls made against this disptach table will go through all loaded
        //  layers, including the state tracker and this layer.
        DispatchTable::load_global_entry_points(&mApplicationDispatchTable);
        DispatchTable::load_instance_entry_points(mApplyInfo.vkInstance, &mApplicationDispatchTable);

        // If GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT is set all restoration calls should
        //  be emitted, so clear the objectMap so every object is restored.
        if (mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT) {
            // TODO : Stash and unstash the object map so it's in the correct state after
            //  all synthetic calls are emitted.
            mApplyInfo.gvkRestorePoint->objectMap.clear();
        }

        // Clear the objectRestorationSubmitted set
        mApplyInfo.gvkRestorePoint->objectRestorationSubmitted.clear();

        // Prepare index arrays to look up objects that need specialized restoration
        uint32_t instanceIndex = 0;
        std::vector<uint32_t> deviceIndices;
        std::vector<uint32_t> commandBufferIndices;
        std::vector<uint32_t> deviceMemoryIndices;
        std::vector<uint32_t> bufferIndices;
        std::vector<uint32_t> imageIndices;
        std::vector<uint32_t> descriptorSetIndices;
        std::vector<uint32_t> accelerationStructureIndices;
        for (uint32_t i = 0; i < manifest->objectCount; ++i) {
            const auto& object = manifest->pObjects[i];
            if (mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT) {
                mApplyInfo.gvkRestorePoint->stateRestorationRequired.insert(object);
            }
            switch (object.type) {
            case VK_OBJECT_TYPE_INSTANCE: {
                gvk_result(!instanceIndex ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                instanceIndex = i;
                mApplyInfo.gvkRestorePoint->stateRestorationRequired.insert(object);
            } break;
            case VK_OBJECT_TYPE_PHYSICAL_DEVICE: {
                mApplyInfo.gvkRestorePoint->stateRestorationRequired.insert(object);
            } break;
            case VK_OBJECT_TYPE_DEVICE: {
                deviceIndices.push_back(i);
                mApplyInfo.gvkRestorePoint->stateRestorationRequired.insert(object);
                if (!(mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT)) {
                    gvk_result(mApplyInfo.dispatchTable.gvkDeviceWaitIdle((VkDevice)object.handle));
                }
                Auto<GvkDeviceRestoreInfo> restoreInfo;
                gvk_result(read_object_restore_info(mApplyInfo.path, "VkDevice", to_hex_string(object.handle), restoreInfo));
                mDeviceRestoreInfos[(VkDevice)object.handle] = restoreInfo;
            } break;
            case VK_OBJECT_TYPE_DEVICE_MEMORY: {
                deviceMemoryIndices.push_back(i);
                if (mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT) {
                    mApplyInfo.gvkRestorePoint->mappingRestorationRequired.insert(object);
                    if (mApplyInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_DEVICE_MEMORY_DATA_BIT) {
                        mApplyInfo.gvkRestorePoint->dataRestorationRequired.insert(object);
                    }
                }
            } break;
            case VK_OBJECT_TYPE_BUFFER: {
                bufferIndices.push_back(i);
                if ((mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT) && (mApplyInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_BUFFER_DATA_BIT)) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.insert(object);
                }
            } break;
            case VK_OBJECT_TYPE_IMAGE: {
                imageIndices.push_back(i);
#if 0
                // TODO : Really need to seperate data and layout
                if ((mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT) && (mApplyInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT)) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.insert(object);
                }
#else
                if ((mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT)) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.insert(object);
                }
#endif
            } break;
            case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: {
                accelerationStructureIndices.push_back(i);
                if ((mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT) && (mApplyInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT)) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.insert(object);
                }
            } break;
            case VK_OBJECT_TYPE_DESCRIPTOR_SET: {
                descriptorSetIndices.push_back(i);
                if (mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.insert(object);
                }
            } break;
            case VK_OBJECT_TYPE_COMMAND_BUFFER: {
                commandBufferIndices.push_back(i);
                if (mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.insert(object);
                }
            } break;
            case VK_OBJECT_TYPE_EVENT:
            case VK_OBJECT_TYPE_FENCE:
            case VK_OBJECT_TYPE_SEMAPHORE: {
                mApplyInfo.gvkRestorePoint->stateRestorationRequired.insert(object);
            } break;
            default: {
            } break;
            }
        }

        // Restore objects
        for (uint32_t i = 0; i < manifest->objectCount; ++i) {
            const auto& object = manifest->pObjects[i];
            if (!mApplyInfo.excluded(object)) {
                gvk_result(restore_object(object));
            }
        }

        // Restore object states
        for (uint32_t i = 0; i < manifest->objectCount; ++i) {
            const auto& object = manifest->pObjects[i];
            if (!mApplyInfo.excluded(object)) {
                gvk_result(restore_object_state(manifest->pObjects[i]));
                gvk_result(BasicApplier::restore_object_name(manifest->pObjects[i]));
            }
        }

        if (mApplyInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_BUFFER_DATA_BIT) {
            // Restore buffer data
            for (const auto& i : bufferIndices) {
                const auto& object = manifest->pObjects[i];
                auto itr = mApplyInfo.gvkRestorePoint->dataRestorationRequired.find(object);
                if (itr != mApplyInfo.gvkRestorePoint->dataRestorationRequired.end()) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.erase(itr);
                    gvk_result(restore_VkBuffer_data(object));
                }
            }
        }

        if (mApplyInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT) {
            // Restore image data
            for (const auto& i : imageIndices) {
#if 0
                // TODO : restore_VkImage_data() is also updating layouts...need to break that logic apart
                restore_VkImage_data(manifest->pObjects[i]);
                // restore_VkImage_layouts(manifest->pObjects[i]);
                // restore_VkImage_data_and_layouts(manifest->pObjects[i]);?
#else
                const auto& object = manifest->pObjects[i];
                auto itr = mApplyInfo.gvkRestorePoint->dataRestorationRequired.find(object);
                if (itr != mApplyInfo.gvkRestorePoint->dataRestorationRequired.end()) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.erase(itr);
                    gvk_result(restore_VkImage_data(object));
                }
#endif
            }
        } else {
            for (const auto& i : imageIndices) {
                const auto& object = manifest->pObjects[i];
                auto itr = mApplyInfo.gvkRestorePoint->dataRestorationRequired.find(object);
                if (itr != mApplyInfo.gvkRestorePoint->dataRestorationRequired.end()) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.erase(itr);
                    gvk_result(restore_VkImage_layouts(object));
                }
            }
        }

        if (mApplyInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT) {
            // Restore acceleration structure data
            AccelerationStructureSerializationResources accelerationStructureSerializationResources;
            for (const auto& i : accelerationStructureIndices) {
                const auto& object = manifest->pObjects[i];
                auto itr = mApplyInfo.gvkRestorePoint->dataRestorationRequired.find(object);
                if (itr != mApplyInfo.gvkRestorePoint->dataRestorationRequired.end()) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.erase(itr);
                    gvk_result(restore_VkAccelerationStructureKHR_data(object, accelerationStructureSerializationResources));
                }
            }
            accelerationStructureSerializationResources.reset();
        }

        if (mApplyInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_DEVICE_MEMORY_DATA_BIT) {
            // Restore device memory data
            for (const auto& i : deviceMemoryIndices) {
                const auto& object = manifest->pObjects[i];
                auto itr = mApplyInfo.gvkRestorePoint->dataRestorationRequired.find(object);
                if (itr != mApplyInfo.gvkRestorePoint->dataRestorationRequired.end()) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.erase(itr);
                    gvk_result(restore_VkDeviceMemory_data(object));
                }
            }
        }

        // Make sure all transfers are complete before moving on to the next steps
        for (auto itr : mDeviceRestoreInfos) {
            gvk_result(mApplyInfo.dispatchTable.gvkDeviceWaitIdle(itr.first));
        }

        // Restore device memory mappings
        for (const auto& i : deviceMemoryIndices) {
            const auto& object = manifest->pObjects[i];
            auto itr = mApplyInfo.gvkRestorePoint->mappingRestorationRequired.find(object);
            if (itr != mApplyInfo.gvkRestorePoint->mappingRestorationRequired.end()) {
                mApplyInfo.gvkRestorePoint->mappingRestorationRequired.erase(itr);
                gvk_result(restore_VkDeviceMemory_mapping(object));
            }
        }

        // Restore descriptor set bindings
        for (const auto& i : descriptorSetIndices) {
            const auto& object = manifest->pObjects[i];
            auto itr = mApplyInfo.gvkRestorePoint->dataRestorationRequired.find(object);
            if (itr != mApplyInfo.gvkRestorePoint->dataRestorationRequired.end()) {
                mApplyInfo.gvkRestorePoint->dataRestorationRequired.erase(itr);
                gvk_result(restore_VkDescriptorSet_bindings(object));
            }
        }

        // Restore command buffer cmds
        for (const auto& i : commandBufferIndices) {
            const auto& object = manifest->pObjects[i];
            if (!mApplyInfo.excluded(object)) {
                auto itr = mApplyInfo.gvkRestorePoint->dataRestorationRequired.find(object);
                if (itr != mApplyInfo.gvkRestorePoint->dataRestorationRequired.end()) {
                    mApplyInfo.gvkRestorePoint->dataRestorationRequired.erase(itr);
                    gvk_result(restore_VkCommandBuffer_cmds(object));
                }
            }
        }

        // Destroy transient objects
        // TODO : Object destruction should happen in reverse dependency order
        // TODO : Should mApplyInfo.destroyObjects be destroyed at the start?
        mApplyInfo.gvkRestorePoint->objectDestructionRequired.insert(mApplyInfo.destroyObjects.begin(), mApplyInfo.destroyObjects.end());
        for (const auto& object : mApplyInfo.gvkRestorePoint->objectDestructionRequired) {
            if (!mApplyInfo.excluded(object)) {
                // TODO : vkDeviceWaitIdle before VkFence, VkSemaphore, VkEvent
                destroy_object(object);
            }
        }
        mApplyInfo.gvkRestorePoint->objectDestructionRequired.clear();
        mApplyInfo.gvkRestorePoint->objectDestructionSubmitted.clear();
        mApplyInfo.gvkRestorePoint->createdObjects.clear();
    } gvk_result_scope_end;
    mResult = gvkResult;
    mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    mLog << "Leaving gvk::restore_point::Applier::apply_restore_point() " << gvk::to_string(mResult, Printer::Default & ~Printer::EnumValue) << layer::Log::Flush;
    return gvkResult;
}

VkResult Applier::register_restored_object(const GvkStateTrackedObject& capturedObject, const GvkStateTrackedObject& restoredObject)
{
    auto inserted = mApplyInfo.gvkRestorePoint->objectMap.register_object_restoration(capturedObject, restoredObject);
    if (inserted && mApplyInfo.pfnProcessRestoredObjectCallback) {
        mApplyInfo.pfnProcessRestoredObjectCallback(&capturedObject, &restoredObject);
    }
    return inserted ? VK_SUCCESS : VK_ERROR_UNKNOWN;
}

void Applier::register_restored_object_destruction(const GvkStateTrackedObject& restoredObject)
{
    // TODO : Fire callback here to notify upper layers?
    mApplyInfo.gvkRestorePoint->objectMap.register_object_destruction(restoredObject);
}

GvkStateTrackedObject Applier::get_restored_object(const GvkStateTrackedObject& restorePointObject)
{
    return mApplyInfo.gvkRestorePoint->objectMap.get_restored_object(restorePointObject);
}

VkResult Applier::restore_object(const GvkStateTrackedObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (!is_valid(mApplyInfo.gvkRestorePoint->objectMap.get_restored_object(restorePointObject)) &&
            mApplyInfo.gvkRestorePoint->objectRestorationSubmitted.insert(restorePointObject).second) {
            gvk_result(BasicApplier::restore_object(restorePointObject));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_object_state(const GvkStateTrackedObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        auto itr = mApplyInfo.gvkRestorePoint->stateRestorationRequired.find(restorePointObject);
        if (itr != mApplyInfo.gvkRestorePoint->stateRestorationRequired.end()) {
            mApplyInfo.gvkRestorePoint->stateRestorationRequired.erase(itr);
            gvk_result(BasicApplier::restore_object_state(restorePointObject));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_object_name(const GvkStateTrackedObject& restorePointObject, uint32_t dependencyCount, const GvkStateTrackedObject* pDependencies, const char* pName)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        auto vkDevice = (VkDevice)restorePointObject.dispatchableHandle;
        switch (restorePointObject.type) {
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
        case VK_OBJECT_TYPE_SURFACE_KHR:
        case VK_OBJECT_TYPE_INSTANCE: {
        // NOTE : vkSetDebugUtilsObjectNameEXT() always takes a VkDevice, even when the
        //  object having its name set is not a VkDevice child.  Need some logic to
        //  deal with this, but it's super low priority...in FA, debug names are very
        //  useful for knowing what resources are being inspected, which isn't really
        //  necessary for keeping track of VkInstance or Vl=kPhysicalDevice
#if 0
            vkDevice = *mDevices.begin();
#else
            pName = nullptr;
#endif
        } break;
        case VK_OBJECT_TYPE_DISPLAY_KHR: {
#if 0
            vkPhysicalDevice = get_dependency<VkPhysicalDevice>(dependencyCount, pDependencies);
#else
            pName = nullptr;
#endif
        } break;
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: {
#if 0
            vkPhysicalDevice = get_dependency<VkPhysicalDevice>(dependencyCount, pDependencies);
#else
            pName = nullptr;
#endif
        } break;
        case VK_OBJECT_TYPE_QUEUE: {
            vkDevice = get_dependency<VkDevice>(dependencyCount, pDependencies);
        } break;
        case VK_OBJECT_TYPE_COMMAND_BUFFER: {
            vkDevice = get_dependency<VkDevice>(dependencyCount, pDependencies);
        } break;
        default: {
            // NOOP :
        } break;
        }
        if (vkPhysicalDevice) {
            for (const auto& gvkDevice : mDevices) {
                if (gvkDevice.get<PhysicalDevice>() == vkPhysicalDevice) {
                    vkDevice = gvkDevice;
                    break;
                }
            }
        }
        auto debugUtilsObjectNameInfoEXT = get_default<VkDebugUtilsObjectNameInfoEXT>();
        debugUtilsObjectNameInfoEXT.objectType = restorePointObject.type;
        debugUtilsObjectNameInfoEXT.objectHandle = get_restored_object(restorePointObject).handle;
        debugUtilsObjectNameInfoEXT.pObjectName = pName;
        vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)vkDevice, (uint64_t)vkDevice }).handle;
        // TODO : Need to check if the extension is loaded and set nullptr for anything
        //  that didn't have a name set at capture time
        if (pName) {
            gvk_result(mApplyInfo.dispatchTable.gvkSetDebugUtilsObjectNameEXT(vkDevice, &debugUtilsObjectNameInfoEXT));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_object_data(const GvkStateTrackedObject& capturedObject)
{
    (void)capturedObject;
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

void Applier::destroy_object(const GvkStateTrackedObject& restorePointObject)
{
    assert(is_valid(restorePointObject));
    if (mApplyInfo.gvkRestorePoint->objectDestructionSubmitted.insert(restorePointObject).second) {
        BasicApplier::destroy_object(restorePointObject);
    }
}

} // namespace state_tracker
} // namespace gvk
