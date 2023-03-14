
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
#include "gvk-restore-point/generated/basic-restore-point-creator.hpp"
#include "gvk-restore-point/detail/copy-engine.hpp"
#include "gvk-restore-point/restore-point-info.hpp"

#include <thread>
#include <unordered_map>
#include <vector>

namespace gvk {
namespace restore_point {

class RestorePointCreator
    : BasicRestorePointCreator
{
public:
    VkResult create_restore_point(VkInstance instance, const CreateInfo& restorePointInfo) override final;

protected:
    VkResult process_VkInstance(const GvkStateTrackedObject& stateTrackedObject, GvkInstanceRestoreInfo& restoreInfo) override final;
    VkResult process_VkPhysicalDevice(const GvkStateTrackedObject& stateTrackedObject, GvkPhysicalDeviceRestoreInfo& restoreInfo) override final;
    VkResult process_VkDevice(const GvkStateTrackedObject& stateTrackedObject, GvkDeviceRestoreInfo& restoreInfo) override final;
    VkResult process_VkQueue(const GvkStateTrackedObject& stateTrackedObject, GvkQueueRestoreInfo& restoreInfo) override final;
    VkResult process_VkCommandBuffer(const GvkStateTrackedObject& stateTrackedObject, GvkCommandBufferRestoreInfo& restoreInfo) override final;
    VkResult process_VkDescriptorSet(const GvkStateTrackedObject& stateTrackedObject, GvkDescriptorSetRestoreInfo& restoreInfo) override final;
    VkResult process_VkDeviceMemory(const GvkStateTrackedObject& stateTrackedObject, GvkDeviceMemoryRestoreInfo& restoreInfo) override final;
    VkResult process_VkBuffer(const GvkStateTrackedObject& stateTrackedObject, GvkBufferRestoreInfo& restoreInfo) override final;
    VkResult process_VkImage(const GvkStateTrackedObject& stateTrackedObject, GvkImageRestoreInfo& restoreInfo) override final;
    VkResult process_VkEvent(const GvkStateTrackedObject& stateTrackedObject, GvkEventRestoreInfo& restoreInfo) override final;
    VkResult process_VkFence(const GvkStateTrackedObject& stateTrackedObject, GvkFenceRestoreInfo& restoreInfo) override final;
    VkResult process_VkSemaphore(const GvkStateTrackedObject& stateTrackedObject, GvkSemaphoreRestoreInfo& restoreInfo) override final;
    VkResult process_VkQueryPool(const GvkStateTrackedObject& stateTrackedObject, GvkQueryPoolRestoreInfo& restoreInfo) override final;
    VkResult process_VkSurfaceKHR(const GvkStateTrackedObject& stateTrackedObject, GvkSurfaceRestoreInfoKHR& restoreInfo) override final;
    VkResult process_VkSwapchainKHR(const GvkStateTrackedObject& stateTrackedObject, GvkSwapchainRestoreInfoKHR& restoreInfo) override final;

private:
    CopyEngine mCopyEngine;
};

} // namespace restore_point
} // namespace gvk
