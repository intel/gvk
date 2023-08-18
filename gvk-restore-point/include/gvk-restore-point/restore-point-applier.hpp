
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
#include "gvk-structures/auto.hpp"
#include "gvk-restore-point/generated/basic-restore-point-applier.hpp"

#include <vector>

namespace gvk {

class RestorePointApplier final
    : public gvk::detail::BasicRestorePointApplier
{
public:
    ~RestorePointApplier();
    VkResult apply_restore_point(const restore_point::ApplyInfo& restorePointInfo, const DispatchTable& dispatchTable, const DispatchTable& dynamicDispatchTable) override final;

protected:
    VkResult process_VkInstance(const GvkStateTrackedObject& stateTrackedObject, const GvkInstanceRestoreInfo& restoreInfo) override final;
    VkResult process_VkPhysicalDevice(const GvkStateTrackedObject& stateTrackedObject, const GvkPhysicalDeviceRestoreInfo& restoreInfo) override final;
    VkResult process_VkDevice(const GvkStateTrackedObject& stateTrackedObject, const GvkDeviceRestoreInfo& restoreInfo) override final;
    VkResult process_VkPipeline(const GvkStateTrackedObject& stateTrackedObject, const GvkPipelineRestoreInfo& restoreInfo) override final;
    VkResult process_VkCommandBuffer(const GvkStateTrackedObject& stateTrackedObject, const GvkCommandBufferRestoreInfo& restoreInfo) override final;
    VkResult process_VkDescriptorSet(const GvkStateTrackedObject& stateTrackedObject, const GvkDescriptorSetRestoreInfo& restoreInfo) override final;
    VkResult process_VkBuffer(const GvkStateTrackedObject& stateTrackedObject, const GvkBufferRestoreInfo& restoreInfo) override final;
    VkResult process_VkImage(const GvkStateTrackedObject& stateTrackedObject, const GvkImageRestoreInfo& restoreInfo) override final;
    VkResult process_VkDeviceMemory(const GvkStateTrackedObject& stateTrackedObject, const GvkDeviceMemoryRestoreInfo& restoreInfo) override final;
    VkResult process_VkEvent(const GvkStateTrackedObject& stateTrackedObject, const GvkEventRestoreInfo& restoreInfo) override final;
    VkResult process_VkFence(const GvkStateTrackedObject& stateTrackedObject, const GvkFenceRestoreInfo& restoreInfo) override final;
    VkResult process_VkSemaphore(const GvkStateTrackedObject& stateTrackedObject, const GvkSemaphoreRestoreInfo& restoreInfo) override final;
    VkResult process_VkDisplayModeKHR(const GvkStateTrackedObject& stateTrackedObject, const GvkDisplayModeRestoreInfoKHR& restoreInfo) override final;
    VkResult process_VkShaderEXT(const GvkStateTrackedObject& stateTrackedObject, const GvkShaderRestoreInfoEXT& restoreInfo) override final;
    VkResult process_VkSurfaceKHR(const GvkStateTrackedObject& stateTrackedObject, const GvkSurfaceRestoreInfoKHR& restoreInfo) override final;
    VkResult process_VkSwapchainKHR(const GvkStateTrackedObject& stateTrackedObject, const GvkSwapchainRestoreInfoKHR& restoreInfo) override final;
};

} // namespace gvk
