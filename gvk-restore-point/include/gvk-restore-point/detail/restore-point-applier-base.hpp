
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

#pragma once

#include "gvk-defines.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-environment.hpp"
#include "gvk-handles.hpp"
#include "gvk-runtime.hpp"
#include "gvk-structures.hpp"
#include "gvk-restore-point/detail/copy-engine.hpp"
#include "gvk-restore-point/restore-point-info.hpp"

#include "gvk-command-structures.hpp"
#include "gvk-restore-point/generated/restore-info.h"
#include "gvk-restore-point/generated/restore-info-enumerations-to-string.hpp"
#include "gvk-restore-point/generated/restore-info-structure-comparison-operators.hpp"
#include "gvk-restore-point/generated/restore-info-structure-create-copy.hpp"
#include "gvk-restore-point/generated/restore-info-structure-deserialization.hpp"
#include "gvk-restore-point/generated/restore-info-structure-destroy-copy.hpp"
#include "gvk-restore-point/generated/restore-info-structure-get-stype.hpp"
#include "gvk-restore-point/generated/restore-info-structure-serialization.hpp"
#include "gvk-restore-point/generated/restore-info-structure-to-string.hpp"
#include "gvk-structures/generated/core-structure-enumerate-handles.hpp"
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <set>

namespace asio {
class thread_pool;
} // namespace asio

namespace gvk {
namespace detail {

class RestorePointApplierBase
{
public:
    using CapturedHandle = uint64_t;
    using RestoredHandle = uint64_t;

    virtual ~RestorePointApplierBase() = 0;
    CapturedHandle get_captured_handle(RestoredHandle restoredHandle) const;
    RestoredHandle get_restored_handle(CapturedHandle capturedHandle) const;
    GvkStateTrackedObject get_object(VkObjectType objectType, uint32_t objectCount, const GvkStateTrackedObject* pObjects);
    const std::vector<GvkStateTrackedObject>& get_current_objects() const;

protected:
    template <typename StructureType>
    inline void update_handles(const StructureType* pStructure)
    {
        assert(pStructure);
        detail::enumerate_structure_handles(
            *pStructure,
            [&](VkObjectType, const uint64_t& capturedHandle)
            {
                const_cast<uint64_t&>(capturedHandle) = get_restored_handle(capturedHandle);
            }
        );
    }

    template <typename StructureType>
    inline void update_cmd_handles(const StructureType* pStructure)
    {
        assert(pStructure);
        switch (pStructure->sType) {
        case GVK_COMMAND_STRUCTURE_TYPE_BEGIN_COMMAND_BUFFER: {
            detail::enumerate_structure_handles(
                *pStructure,
                [&](VkObjectType objectType, const uint64_t& capturedHandle)
                {
                    switch (objectType) {
                    case VK_OBJECT_TYPE_FRAMEBUFFER: {
                        auto restoredHandleItr = mRestoredHandles.find(capturedHandle);
                        const_cast<uint64_t&>(capturedHandle) = restoredHandleItr != mRestoredHandles.end() ? restoredHandleItr->second : 0;
                    } break;
                    default: {
                        const_cast<uint64_t&>(capturedHandle) = get_restored_handle(capturedHandle);
                    } break;
                    }
                }
            );
        } break;
        default: {
            detail::enumerate_structure_handles(
                *pStructure,
                [&](VkObjectType, const uint64_t& capturedHandle)
                {
                    const_cast<uint64_t&>(capturedHandle) = get_restored_handle(capturedHandle);
                }
            );
        } break;
        }
    }

    template <typename RestoreInfoType>
    inline Auto<RestoreInfoType> get_restore_info(const std::string& handleTypeName, uint64_t capturedHandle)
    {
        auto path = (mApplyInfo.path / handleTypeName / to_hex_string(capturedHandle)).replace_extension(".info");
        std::ifstream file(path, std::ios::binary);
        assert(file.is_open());
        Auto<RestoreInfoType> restoreInfo;
        deserialize(file, nullptr, restoreInfo);
        return restoreInfo;
    }

    std::pair<CommandBuffer, Fence> get_thread_command_buffer(const Device& gvkDevice);
    void register_restored_handle(VkObjectType objectType, CapturedHandle capturedHandle, RestoredHandle restoredHandle);
    virtual VkResult process_object(const GvkStateTrackedObject& stateTrackedObject) = 0;
    virtual VkResult restore_device_memory_bindings(const GvkStateTrackedObject& stateTrackedObject);
    virtual VkResult restore_image_layouts(GvkStateTrackedObject stateTrackedObject);
    virtual VkResult restore_device_memory_data(const GvkStateTrackedObject& stateTrackedObject);
    virtual VkResult restore_descriptor_bindings(const GvkStateTrackedObject& stateTrackedObject);
    virtual VkResult restore_command_buffer_cmds(const GvkStateTrackedObject& stateTrackedObject) = 0;

    DispatchTable mDispatchTable;
    DispatchTable mDynamicDispatchTable;
    Auto<GvkRestorePointManifest> mManifest;
    restore_point::ApplyInfo mApplyInfo;

    std::unordered_map<CapturedHandle, RestoredHandle> mCapturedHandles;
    std::unordered_map<RestoredHandle, CapturedHandle> mRestoredHandles;
    std::unordered_set<HandleId<uint64_t, uint64_t>> mProcessedHandles;
    std::set<GvkStateTrackedObject> mRestoredBuffers;
    std::set<GvkStateTrackedObject> mRestoredImages;
    std::set<GvkStateTrackedObject> mRestoredDeviceMemories;
    std::vector<GvkStateTrackedObject> mRestoredDescriptorSets;
    std::vector<GvkStateTrackedObject> mRestoredCommandBuffers;
    std::vector<GvkStateTrackedObject> mCurrentObjects;

    std::map<GvkStateTrackedObject, std::set<Auto<VkBindBufferMemoryInfo>>> mPendingBufferMemoryBinds;
    std::map<GvkStateTrackedObject, std::set<Auto<VkBindImageMemoryInfo>>> mPendingImageMemoryBinds;

    CopyEngine mCopyEngine;
    CommandBuffer mCommandBuffer;
    std::mutex mThreadCommandBuffersMutex;
    std::map<Device, std::unordered_map<std::thread::id, std::pair<CommandBuffer, Fence>>> mThreadCommandBuffers;
    asio::thread_pool* mpThreadPool { nullptr };
    std::set<Instance> mInstances;
    std::set<Device> mDevices;

    std::set<uint64_t> mProcessedIds;
};

} // namespace detail
} // namespace gvk
