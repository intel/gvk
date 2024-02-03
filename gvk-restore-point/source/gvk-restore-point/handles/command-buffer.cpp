
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
#include "gvk-restore-point/creator.hpp"
#include "gvk-restore-point/generated/update-structure-handles.hpp"
#include "gvk-command-structures/generated/command-structure-enumerate-handles.hpp"

namespace gvk {
namespace restore_point {

#if 0
// NOTE : Implemented in "build/gvk-restore-point/source/gvk-restore-point/generated/creator-process-command-buffer.cpp"
VkResult Creator::process_VkCommandBuffer(GvkCommandBufferRestoreInfo& restoreInfo)
{
    return BasicCreator::process_VkCommandBuffer(restorePointCreateInfo, restoreInfo);
}
#endif

#if 0
// NOTE : Implemented in "build/gvk-restore-point/source/gvk-restore-point/generated/creator-process-command-buffer.cpp"
VkResult Applier::process_VkCommandBuffer_cmds(const GvkRestorePointObject& capturedCommandBuffer)
{
    (void)capturedCommandBuffer;
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}
#endif

VkResult Applier::process_GvkCommandStructureAllocateCommandBuffers(const GvkRestorePointObject& restorePointObject, const GvkCommandBufferRestoreInfo& restoreInfo, GvkCommandStructureAllocateCommandBuffers& commandStructure)
{
    // TODO : Reset after processing
    (void)restorePointObject;
    (void)restoreInfo;
    assert(commandStructure.pAllocateInfo);
    const_cast<VkCommandBufferAllocateInfo*>(commandStructure.pAllocateInfo)->commandBufferCount = 1;
    return VK_SUCCESS;
}

void update_command_structure_handles(const std::map<GvkRestorePointObject, GvkRestorePointObject>& restorePointObjects, const GvkCommandStructureBeginCommandBuffer& commandStructure)
{
    GvkRestorePointObject dispatchableRestorePointObject{ };
    dispatchableRestorePointObject.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
    dispatchableRestorePointObject.handle = (uint64_t)commandStructure.commandBuffer;
    dispatchableRestorePointObject.dispatchableHandle = (uint64_t)commandStructure.commandBuffer;
    detail::enumerate_structure_handles(
        commandStructure,
        [&](VkObjectType objectType, const uint64_t& handle)
        {
            if (handle) {
                GvkRestorePointObject restorePointObject{ };
                restorePointObject.type = objectType;
                restorePointObject.handle = handle;
                restorePointObject.dispatchableHandle = dispatchableRestorePointObject.handle;
                auto itr = restorePointObjects.find(restorePointObject);
                if (objectType == VK_OBJECT_TYPE_FRAMEBUFFER) {
                    const_cast<uint64_t&>(handle) = itr != restorePointObjects.end() ? itr->second.handle : 0;
                } else {
                    assert(itr != restorePointObjects.end());
                    const_cast<uint64_t&>(handle) = itr->second.handle;
                }
            }
        }
    );
}

void update_command_structure_handles(const std::map<GvkRestorePointObject, GvkRestorePointObject>& restorePointObjects, uint64_t parentHandle, const GvkCommandStructureBeginCommandBuffer& commandStructure)
{
    detail::enumerate_structure_handles(
        commandStructure,
        [&](VkObjectType objectType, const uint64_t& handle)
        {
            if (handle) {
                GvkRestorePointObject restorePointObject{ };
                restorePointObject.type = objectType;
                restorePointObject.handle = handle;
                restorePointObject.dispatchableHandle = parentHandle;
                auto itr = restorePointObjects.find(restorePointObject);
                if (objectType == VK_OBJECT_TYPE_FRAMEBUFFER) {
                    const_cast<uint64_t&>(handle) = itr != restorePointObjects.end() ? itr->second.handle : 0;
                } else {
                    assert(itr != restorePointObjects.end());
                    const_cast<uint64_t&>(handle) = itr->second.handle;
                }
            }
        }
    );
}

} // namespace restore_point
} // namespace gvk
