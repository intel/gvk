
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

#include "gvk-handles.hpp"

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace asio {
class thread_pool;
} // namespace asio

namespace gvk {

class CopyEngine final
{
public:
    class DeviceMemoryCopyInfo
    {
    public:
        VkDeviceMemory vkDeviceMemory { VK_NULL_HANDLE };
        Auto<VkMemoryAllocateInfo> allocateInfo;
        std::filesystem::path path;
        void* pData { nullptr };
        std::function<void(VkDevice, VkDeviceMemory, VkMemoryAllocateInfo, void*)> onProcessData;
    };

    class ImageCopyInfo
    {
    public:
        VkImage vkImage { VK_NULL_HANDLE };
        Auto<VkImageCreateInfo> createInfo { };
        std::vector<VkImageLayout> oldImageLayouts;
        std::vector<VkImageLayout> newImageLayouts;
        std::filesystem::path path;
        std::function<void(VkImage)> onProcessLayouts;
    };

    CopyEngine();
    ~CopyEngine();
    void reset();

    void disable_multi_threading();
    void set_thread_initialization_callback(std::function<void(std::thread::id)> onThreadInitialized);
    void download(VkDevice vkDevice, DeviceMemoryCopyInfo deviceMemoryCopyInfo);
    void upload(VkDevice vkDevice, DeviceMemoryCopyInfo deviceMemoryCopyInfo);
    void transition_image_layouts(VkDevice vkDevice, ImageCopyInfo imageCopyInfo);
    void wait();

private:
    class TaskResources
    {
    public:
        CommandBuffer gvkCommandBuffer;
        Fence gvkFence;
        Buffer gvkBuffer;
        DeviceMemory gvkDeviceMemory;
    };

    void initialize_thread();
    const TaskResources& get_task_resources(const Device& gvkDevice, VkDeviceSize taskSize);
    void download_impl(VkDevice vkDevice, DeviceMemoryCopyInfo deviceMemoryCopyInfo);
    void upload_impl(VkDevice vkDevice, DeviceMemoryCopyInfo deviceMemoryCopyInfo);
    void transition_image_layouts_impl(VkDevice vkDevice, ImageCopyInfo imageCopyInfo);

    bool mMultiThreaded { true };
    asio::thread_pool* mpThreadPool { nullptr };
    std::mutex mQueueMutex;
    std::mutex mTaskResourceMutex;
    std::unordered_set<std::thread::id> mInitializedThreads;
    std::function<void(std::thread::id)> mOnThreadInitialized;
    std::map<Device, std::unordered_map<std::thread::id, TaskResources>> mTaskResources;
    DispatchTable mDispatchTable;
};

} // namespace gvk
