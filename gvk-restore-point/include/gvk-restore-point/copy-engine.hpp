
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
#include "gvk-handles.hpp"
#include "gvk-restore-info.hpp"
#include "gvk-runtime.hpp"
#include "gvk-structures.hpp"

#include "asio.hpp"

#include <filesystem>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

typedef struct VkCommandBufferProxy_T
{
    VkCommandBuffer vkCommandBuffer{ };
} VkCommandBufferProxy_T;

VK_DEFINE_HANDLE(VkCommandBufferProxy)

namespace gvk {
namespace restore_point {

class CopyEngine final
{
public:
    struct CreateInfo
    {
        uint32_t threadCount{ };
        void(*pfnInitializeThreadCallback)(){ };
    };

    struct DownloadDeviceMemoryInfo
    {
        VkDevice device{ };
        VkDeviceMemory memory{ };
        VkMemoryAllocateInfo memoryAllocateInfo{ };
        uint32_t regionCount{ };
        const VkBufferCopy* pRegions{ };
        void* pUserData{ };
        void(*pfnCallback)(const DownloadDeviceMemoryInfo&, const VkBindBufferMemoryInfo&, const uint8_t*){ };
    };

    struct DownloadBufferInfo
    {
        VkDevice device{ };
        VkBuffer buffer{ };
        VkBufferCreateInfo bufferCreateInfo{ };
        VkDeviceSize offset{ };
        VkDeviceSize size{ };
        void* pUserData{ };
        void(*pfnCallback)(const DownloadBufferInfo&, const VkBindBufferMemoryInfo&, const uint8_t*){ };
    };

    struct DownloadImageInfo
    {
        VkDevice device{ };
        VkImage image{ };
        VkImageCreateInfo imageCreateInfo{ };
        VkImageSubresourceRange imageSubresourceRange{ };
        const VkImageLayout* pImageLayouts{ };
        void* pUserData{ };
        void(*pfnCallback)(const DownloadImageInfo&, const VkBindBufferMemoryInfo&, const uint8_t*){ };
    };

    struct DownloadAccelerationStructureInfo
    {
        VkAccelerationStructureKHR accelerationStructure{ };
        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{ };
        VkDeviceSize accelerationStructureSerializedSize{ };
        VkBuffer buffer{ };
        VkBufferCreateInfo bufferCreateInfo{ };
        VkDeviceAddress bufferDeviceAddress{ };
        VkDeviceMemory memory{ };
        VkMemoryAllocateInfo memoryAllocateInfo{ };
        VkDevice device{ };
        void* pUserData{ };
        void(*pfnCallback)(const DownloadAccelerationStructureInfo&, const VkBindBufferMemoryInfo&, const uint8_t*) { };
    };

    struct UploadDeviceMemoryInfo
    {
        std::filesystem::path path;
        VkDevice device{ };
        VkDeviceMemory memory{ };
        VkMemoryAllocateInfo memoryAllocateInfo{ };
        uint32_t regionCount{ };
        const VkBufferCopy* pRegions{ };
        void* pUserData{ };
        void(*pfnCallback)(const UploadDeviceMemoryInfo&, const VkBindBufferMemoryInfo&, uint8_t*) { };
    };

    struct UploadBufferInfo
    {
        std::filesystem::path path;
        VkDevice device{ };
        VkBuffer buffer{ };
        VkBufferCreateInfo bufferCreateInfo{ };
        VkDeviceSize offset{ };
        VkDeviceSize size{ };
        void* pUserData{ };
        void(*pfnCallback)(const UploadBufferInfo&, const VkBindBufferMemoryInfo&, uint8_t*){ };
    };

    struct UploadImageInfo
    {
        std::filesystem::path path;
        VkDevice device{ };
        VkImage image{ };
        VkImageCreateInfo imageCreateInfo{ };
        VkImageSubresourceRange imageSubresourceRange{ };
        const VkImageLayout* pOldImageLayouts{ };
        const VkImageLayout* pNewImageLayouts{ };
        void* pUserData{ };
        void(*pfnCallback)(const UploadImageInfo&, const VkBindBufferMemoryInfo&, uint8_t*){ };
    };

    struct UploadAccelerationStructureInfo
    {
        std::filesystem::path path;
        VkAccelerationStructureKHR accelerationStructure{ };
        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{ };
        GvkAccelerationStructureSerilizationInfoKHR accelerationStructureSerializationInfo{ };
        VkDeviceSize accelerationStructureSerializedSize{ };
        VkBuffer buffer{ };
        VkBufferCreateInfo bufferCreateInfo{ };
        VkDeviceAddress bufferDeviceAddress{ };
        VkDeviceMemory memory{ };
        VkMemoryAllocateInfo memoryAllocateInfo{ };
        VkDevice device{ };
        void* pUserData{ };
        void(*pfnCallback)(const UploadAccelerationStructureInfo&, const VkBindBufferMemoryInfo&, uint8_t*) { };
    };

    struct TransitionImageLayoutInfo
    {
        VkDevice device{ };
        VkImage image{ };
        VkImageCreateInfo imageCreateInfo{ };
        VkImageSubresourceRange imageSubresourceRange{ };
        const VkImageLayout* pOldImageLayouts{ };
        const VkImageLayout* pNewImageLayouts{ };
    };

    struct BuildAcclerationStructureInfo
    {
        VkDevice device{ };
        VkAccelerationStructureKHR accelerationStructure{ };
        VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{ };
        const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos{ };
    };

    CopyEngine() = default;
    static VkResult create(const gvk::Device& device, const CreateInfo* pCreateInfo, CopyEngine* pCopyEngine);
    CopyEngine(CopyEngine&& other);
    CopyEngine& operator=(CopyEngine&& other);
    ~CopyEngine();
    void reset();
    void wait();
    operator bool() const;

    void download(DownloadDeviceMemoryInfo downloadInfo);
    void download(DownloadBufferInfo downloadInfo);
    void download(DownloadImageInfo downloadInfo);
    void download(DownloadAccelerationStructureInfo downloadInfo);
    void upload(UploadDeviceMemoryInfo uploadInfo);
    void upload(UploadBufferInfo uploadInfo);
    void upload(UploadImageInfo uploadInfo);
    void upload(UploadAccelerationStructureInfo uploadInfo);
    void upload_ex(UploadAccelerationStructureInfo uploadInfo);
    void transition_image_layouts(TransitionImageLayoutInfo transitionInfo);
    void build_acceleration_structure(BuildAcclerationStructureInfo buildInfo);
    VkResult get_acceleration_structure_serialization_size(VkAccelerationStructureKHR accelerationStructure, VkDeviceSize* pSize);

private:
    class TaskResources final
    {
    public:
        Buffer buffer;
        DeviceMemory memory;
        Fence fence;
        CommandPool commandPool;
        VkCommandBuffer vkCommandBuffer{ };
    };

    class AccelerationStructureTaskResources final
    {
    public:
        Buffer buffer;
        DeviceMemory memory;
        Fence fence;
        QueryPool queryPool;
        CommandPool commandPool;
        VkCommandBuffer vkCommandBuffer{ };
    };

    void initialize_thread();
    VkResult get_task_resources(VkDeviceSize taskSize, TaskResources* pTaskResources);
    VkResult get_acceleration_structure_task_resources(VkDeviceSize taskSize, AccelerationStructureTaskResources* pTaskResources);
    VkResult get_acceleration_structure_task_resources(const GvkAccelerationStructureSerilizationInfoKHR& accelerationStructureSerializationInfo, AccelerationStructureTaskResources* pTaskResources);

    Device mDevice;
    std::mutex mQueueMutex;
    Queue mQueue;
    void(*mpfnInitializeThreadCallback)() { };
    std::unique_ptr<asio::thread_pool> mupThreadPool;
    std::mutex mTaskResourcesMutex;
    std::unordered_map<std::thread::id, TaskResources> mTaskResources;
    std::unordered_map<std::thread::id, AccelerationStructureTaskResources> mAccelerationStructureTaskResources;
    bool mAccelerationStrcutureSerializationInfoRetrieved{ };

    CopyEngine(const CopyEngine&) = delete;
    CopyEngine& operator=(const CopyEngine&) = delete;
};

uint32_t get_image_data_size(const VkImageCreateInfo& imageCreateInfo, const VkImageSubresourceRange& imageSubresourceRange);
uint32_t get_image_data_size(VkFormat format, const VkExtent3D& extent, const VkImageSubresourceRange& imageSubresourceRange);

} // namespace restore_point
} // namespace gvk
