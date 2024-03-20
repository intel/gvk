
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
    mLog.set_instance(applyInfo.instance);
    mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    mLog << "Entered gvk::restore_point::Applier::apply_restore_point()" << Log::Flush;
    mRestorePointObjects.clear();
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        mApplyInfo = applyInfo;
        std::ifstream infoFile(mApplyInfo.path / "GvkRestorePointManifest.info", std::ios::binary);
        gvk_result(infoFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        Auto<GvkRestorePointManifest> manifest;
        deserialize(infoFile, nullptr, manifest);

        ///////////////////////////////////////////////////////////////////////////////
        if (mApplyInfo.repeating_HACK) {
            assert(mApplyInfo.pLayerInfo);
            auto& layerInfo = *mApplyInfo.pLayerInfo;
            (void)layerInfo;
            mRestoredObjects.clear();
            mRestoredObjectStates.clear();
            mProcessedObjects.clear();
            mRestorePointObjects.clear();

            std::vector<GvkRestorePointObject> restorePointImages;
            for (uint32_t i = 0; i < manifest->objectCount; ++i) {
                const auto& object = manifest->pObjects[i];
                switch (object.type) {
                case VK_OBJECT_TYPE_DEVICE: {
                    gvk_result(mApplyInfo.dispatchTable.gvkDeviceWaitIdle((VkDevice)object.handle));
                } break;
                case VK_OBJECT_TYPE_IMAGE: {
                    restorePointImages.push_back(object);
                } break;
                default: {
                } break;
                }
            }
            #if 0 // TODO : Need to figure out how upper layers should create persistent objects
            // Destroy created objects not present in manifest
            std::vector<GvkRestorePointObject> createdDescriptorSets;
            for (const auto& createdObject : layerInfo.createdObjects) {
                // TODO : Object destruction should happen in reverse dependency order
                // switch (createdObject.type) {
                // // case VK_OBJECT_TYPE_DESCRIPTOR_SET: {
                // //     createdDescriptorSets.push_back(createdObject);
                // // } break;
                // default: {
                    destroy_object(createdObject);
                // } break;
                // }
            }
            #endif
            // destroy_VkDescriptorSets(createdDescriptorSets);
            // Restore objects
            for (uint32_t i = 0; i < manifest->objectCount; ++i) {
                const auto& object = (const GvkStateTrackedObject&)manifest->pObjects[i];
                if (!mApplyInfo.excludedObjects.count(object)) {
                    gvk_result(restore_object(manifest->pObjects[i]));
                }
            }
            // Restore object states
            for (uint32_t i = 0; i < manifest->objectCount; ++i) {
                const auto& object = (const GvkStateTrackedObject&)manifest->pObjects[i];
                if (!mApplyInfo.excludedObjects.count(object)) {
                    gvk_result(restore_object_state(manifest->pObjects[i]));
                    gvk_result(BasicApplier::restore_object_name(manifest->pObjects[i]));
                }
            }
            // Restore Buffer data
            // Restore Image data
            // Restore Image layouts
            for (const auto& restorePointImage : restorePointImages) {
                gvk_result(restore_VkImage_layouts(restorePointImage));
            }
            for (auto& copyEngineItr : mCopyEngines) {
                copyEngineItr.second.wait();
            }
            // Restore DeviceMemory data
            // Restore DeviceMemory mapping
            // Restore DescriptorSet bindings
            // Restore CommandBuffer cmds
            ///////////////////////////////////////////////////////////////////////////////
            // Destroy transient objects
            ///////////////////////////////////////////////////////////////////////////////
        } else
        ///////////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////////////
        // Restore objects
        // Restore Buffer data
        // Restore Image data
        // Restore Image layouts
        // Restore DeviceMemory data
        // Restore DeviceMemory mapping
        // Restore DescriptorSet bindings
        // Restore CommandBuffer cmds
        // Process transient objects
        ///////////////////////////////////////////////////////////////////////////////
        // Restore objects
        // Restore Buffer data*
        // Restore Image data*
        // Restore Image layouts
        // Restore DeviceMemory data
        // Restore DeviceMemory mappings
        // Restore DescriptorSet bindings
        // Restore CommandBuffer cmds
        // Process transient objects
        ///////////////////////////////////////////////////////////////////////////////
        if (string::to_lower(get_env_var("DIRECT_MEMORY")) == "true") {
            ///////////////////////////////////////////////////////////////////////////////
            std::vector<GvkRestorePointObject> capturedCommandBuffers;
            std::vector<GvkRestorePointObject> capturedDeviceMemories;
            std::vector<GvkRestorePointObject> capturedBuffers;
            std::vector<GvkRestorePointObject> capturedImages;
            std::vector<GvkRestorePointObject> capturedDescriptorSets;
            for (uint32_t i = 0; i < manifest->objectCount; ++i) {
                const auto& capturedObject = manifest->pObjects[i];
                gvk_result(process_object(capturedObject));
                switch (capturedObject.type) {
                case VK_OBJECT_TYPE_COMMAND_BUFFER: {
                    capturedCommandBuffers.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_DEVICE_MEMORY: {
                    capturedDeviceMemories.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_BUFFER: {
                    capturedBuffers.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_IMAGE: {
                    capturedImages.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_DESCRIPTOR_SET: {
                    capturedDescriptorSets.push_back(capturedObject);
                } break;
                default: {
                } break;
                }
            }
            for (uint32_t i = 0; i < manifest->objectCount; ++i) {
                gvk_result(BasicApplier::restore_object_name(manifest->pObjects[i]));
            }
            // TODO : Minize copies on repeat restore...VkBuffer, VkImage, VkAccelerationStructure
            for (const auto& capturedImage : capturedImages) {
                // TODO : Need to be able to call process_VkImage_data() for repeat, but it's
                //  a doing a bit too much at the moment...it also transitions layouts for
                //  swapchain images...this is because the data transfer codepath uses the
                //  copy engine barriers to do the transition.  These bits of logic should be
                //  untangled from each other.
                // process_VkImage_data(capturedImage);
                process_VkImage_layouts(capturedImage);
            }
            for (const auto& capturedDeviceMemory : capturedDeviceMemories) {
                process_VkDeviceMemory_data(capturedDeviceMemory);
                process_VkDeviceMemory_mapping(capturedDeviceMemory);
            }
            for (const auto& capturedDescriptorSet : capturedDescriptorSets) {
                process_VkDescriptorSet_bindings(capturedDescriptorSet);
            }
            for (const auto& capturedCommandBuffer : capturedCommandBuffers) {
                process_VkCommandBuffer_cmds(capturedCommandBuffer);
            }
            process_transient_objects();
            ///////////////////////////////////////////////////////////////////////////////
        } else {
            ///////////////////////////////////////////////////////////////////////////////
            std::vector<GvkRestorePointObject> capturedCommandBuffers;
            std::vector<GvkRestorePointObject> capturedDeviceMemories;
            std::vector<GvkRestorePointObject> capturedBuffers;
            std::vector<GvkRestorePointObject> capturedImages;
            std::vector<GvkRestorePointObject> capturedAccelerationStructures;
            std::vector<GvkRestorePointObject> capturedDescriptorSets;
            for (uint32_t i = 0; i < manifest->objectCount; ++i) {
                const auto& capturedObject = manifest->pObjects[i];
                gvk_result(process_object(capturedObject));
                switch (capturedObject.type) {
                case VK_OBJECT_TYPE_COMMAND_BUFFER: {
                    capturedCommandBuffers.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_DEVICE_MEMORY: {
                    capturedDeviceMemories.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_BUFFER: {
                    capturedBuffers.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_IMAGE: {
                    capturedImages.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: {
                    capturedAccelerationStructures.push_back(capturedObject);
                } break;
                case VK_OBJECT_TYPE_DESCRIPTOR_SET: {
                    capturedDescriptorSets.push_back(capturedObject);
                } break;
                default: {
                } break;
                }
            }
            for (uint32_t i = 0; i < manifest->objectCount; ++i) {
                gvk_result(BasicApplier::restore_object_name(manifest->pObjects[i]));
            }
            for (const auto& capturedBuffer : capturedBuffers) {
                process_VkBuffer_data(capturedBuffer);
            }
            for (const auto& capturedImage : capturedImages) {
                // TODO : Layout transition is bundled with data upload logic...this is "okish"
                //  but the codepath to transition layout without a data transfer (ie for
                //  swapchain images or the direct memory codepath) should be modular.
                process_VkImage_data(capturedImage);
                // process_VkImage_layouts(capturedImage);
            }
            for (const auto& capturedDeviceMemory : capturedDeviceMemories) {
                process_VkDeviceMemory_mapping(capturedDeviceMemory);
            }

            apply_VkAccelerationStructure_restore_point(capturedAccelerationStructures);

            for (const auto& capturedDescriptorSet : capturedDescriptorSets) {
                process_VkDescriptorSet_bindings(capturedDescriptorSet);
            }
            for (const auto& capturedCommandBuffer : capturedCommandBuffers) {
                process_VkCommandBuffer_cmds(capturedCommandBuffer);
            }
            process_transient_objects();
            ///////////////////////////////////////////////////////////////////////////////
        }
    } gvk_result_scope_end;
    if (mApplyInfo.pLayerInfo) {
        mApplyInfo.pLayerInfo->createdObjects.clear();
    }
    // mApplyInfo.pLayerInfo->destroyedObjects.clear();
    mRestorePointObjects.clear();
    mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    mLog << "Leaving gvk::restore_point::Applier::apply_restore_point() " << gvk::to_string(mResult, Printer::Default & ~Printer::EnumValue) << Log::Flush;
    return gvkResult;
}

VkResult Applier::register_restored_object_ex(const GvkRestorePointObject& capturedObject, const GvkRestorePointObject& restoredObject)
{
    auto inserted = mRestorePointObjects.insert({ capturedObject, restoredObject }).second;
    if (inserted && mApplyInfo.pfnProcessRestoredObjectCallback) {
        mApplyInfo.pfnProcessRestoredObjectCallback((const GvkStateTrackedObject*)&capturedObject, (const GvkStateTrackedObject*)&restoredObject);
    }
    return inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

VkResult Applier::restore_object(const GvkRestorePointObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRestoredObjects.insert(restorePointObject).second) {
            assert(!mRestorePointObjects.count(restorePointObject));
            // TODO : Deferred application needs to treat everything as "restore required"
            GvkStateTrackedObjectInfo stateTrackedObjectInfo{ };
            gvkGetStateTrackedObjectInfo((GvkStateTrackedObject*)&restorePointObject, &stateTrackedObjectInfo);
            if (stateTrackedObjectInfo.flags & GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT) {
                mRestorePointObjects.insert({ restorePointObject, restorePointObject });
            } else {
                gvk_result(BasicApplier::restore_object(restorePointObject));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_object_state(const GvkRestorePointObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRestoredObjectStates.insert(restorePointObject).second) {
            gvk_result(BasicApplier::restore_object_state(restorePointObject));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_object_name(const GvkRestorePointObject& restorePointObject, uint32_t dependencyCount, const GvkRestorePointObject* pDependencies, const char* pName)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        auto vkDevice = (VkDevice)restorePointObject.dispatchableHandle;
        switch (restorePointObject.type) {
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
        case VK_OBJECT_TYPE_SURFACE_KHR:
        case VK_OBJECT_TYPE_INSTANCE: {
            // TODO : restore_VkDevice() is empty...need to setup unmanaged Device.
            //  Really need to collapse deferred vs repeat codepaths.
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

VkResult Applier::process_transient_objects()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk
