
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
#include "gvk-restore-point/generated/basic-creator.hpp"
#include "gvk-restore-point/copy-engine.hpp"
#include "gvk-restore-point/logger.hpp"

#include <set>
#include <unordered_map>

namespace gvk {
namespace restore_point {

class Creator final
    : public BasicCreator
{
public:
    VkResult create_restore_point(const CreateInfo& createInfo) override final;
    void create_VkAccelerationStructure_restore_point();
    const std::vector<GvkRestorePointObject>& get_restore_point_objects() const;

protected:
    VkResult process_VkInstance(GvkInstanceRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkPhysicalDevice(GvkPhysicalDeviceRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkDevice(GvkDeviceRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkQueue(GvkQueueRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkCommandBuffer(GvkCommandBufferRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkDescriptorSet(GvkDescriptorSetRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkDeviceMemory(GvkDeviceMemoryRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkAccelerationStructureKHR(GvkAccelerationStructureRestoreInfoKHR& objectRestoreInfo) override final;
    VkResult process_VkBuffer(GvkBufferRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkImage(GvkImageRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkEvent(GvkEventRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkFence(GvkFenceRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkSemaphore(GvkSemaphoreRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkQueryPool(GvkQueryPoolRestoreInfo& objectRestoreInfo) override final;
    VkResult process_VkSurfaceKHR(GvkSurfaceRestoreInfoKHR& objectRestoreInfo) override final;
    VkResult process_VkSwapchainKHR(GvkSwapchainRestoreInfoKHR& objectRestoreInfo) override final;
    VkResult process_VkDebugReportCallbackEXT(GvkDebugReportCallbackRestoreInfoEXT& restoreInfo) override final;
    VkResult process_VkDebugUtilsMessengerEXT(GvkDebugUtilsMessengerRestoreInfoEXT& restoreInfo) override final;
    static void process_downloaded_VkDeviceMemory(const CopyEngine::DownloadDeviceMemoryInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData);
    static void process_downloaded_VkAccelerationStructureKHR(const CopyEngine::DownloadAccelerationStructureInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData);
    static void process_downloaded_VkBuffer(const CopyEngine::DownloadBufferInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData);
    static void process_downloaded_VkImage(const CopyEngine::DownloadImageInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData);

private:
    Instance mInstance;
    std::set<Device> mDevices;
    std::unordered_map<VkQueue, Auto<VkDeviceQueueCreateInfo>> mDeviceQueueCreateInfos;
    std::unordered_map<VkDevice, CopyEngine> mCopyEngines;
    Log mLog;
};

} // namespace state_tracker
} // namespace gvk
