
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

#include "gvk-defines.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-restore-point/restore-point-creator.hpp"
#include "gvk-restore-point/generated/restore-info.h"
#include "gvk-restore-point/generated/restore-info-structure-get-stype.hpp"
#include "gvk-restore-point/generated/restore-info-structure-serialization.hpp"
#include "gvk-restore-point/generated/restore-info-structure-to-string.hpp"
#include "gvk-structures/defaults.hpp"

#include <fstream>
#include <utility>
#include <vector>

#include <iostream>

PFN_gvkEnumerateStateTrackedObjects pfnGvkEnumerateStateTrackedObjects;
PFN_gvkEnumerateStateTrackedObjectDependencies pfnGvkEnumerateStateTrackedObjectDependencies;
PFN_gvkEnumerateStateTrackedObjectBindings pfnGvkEnumerateStateTrackedObjectBindings;
PFN_gvkEnumerateStateTrackedCommandBufferCmds pfnGvkEnumerateStateTrackedCommandBufferCmds;
PFN_gvkGetStateTrackedObjectInfo pfnGvkGetStateTrackedObjectInfo;
PFN_gvkGetStateTrackedObjectCreateInfo pfnGvkGetStateTrackedObjectCreateInfo;
PFN_gvkGetStateTrackedObjectAllocateInfo pfnGvkGetStateTrackedObjectAllocateInfo;
PFN_gvkGetStateTrackedImageLayouts pfnGvkGetStateTrackedImageLayouts;
PFN_gvkGetStateTrackedMappedMemory pfnGvkGetStateTrackedMappedMemory;
PFN_gvkDisableStateTracker pfnGvkDisableStateTracker;
PFN_gvkEnableStateTracker pfnGvkEnableStateTracker;

namespace gvk {
namespace restore_point {

VkResult RestorePointCreator::create_restore_point(VkInstance instance, const CreateInfo& restorePointInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto dlStateTracker = gvk_dlopen(VK_LAYER_INTEL_GVK_STATE_TRACKER_NAME);
        gvk_result(dlStateTracker ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkEnumerateStateTrackedObjects = (PFN_gvkEnumerateStateTrackedObjects)gvk_dlsym(dlStateTracker, "gvkEnumerateStateTrackedObjects");
        gvk_result(pfnGvkEnumerateStateTrackedObjects ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkEnumerateStateTrackedObjectDependencies = (PFN_gvkEnumerateStateTrackedObjectDependencies)gvk_dlsym(dlStateTracker, "gvkEnumerateStateTrackedObjectDependencies");
        gvk_result(pfnGvkEnumerateStateTrackedObjectDependencies ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkEnumerateStateTrackedObjectBindings = (PFN_gvkEnumerateStateTrackedObjectBindings)gvk_dlsym(dlStateTracker, "gvkEnumerateStateTrackedObjectBindings");
        gvk_result(pfnGvkEnumerateStateTrackedObjectBindings ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkEnumerateStateTrackedCommandBufferCmds = (PFN_gvkEnumerateStateTrackedCommandBufferCmds)gvk_dlsym(dlStateTracker, "gvkEnumerateStateTrackedCommandBufferCmds");
        gvk_result(pfnGvkEnumerateStateTrackedCommandBufferCmds ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkGetStateTrackedObjectInfo = (PFN_gvkGetStateTrackedObjectInfo)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedObjectInfo");
        gvk_result(pfnGvkGetStateTrackedObjectInfo ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkGetStateTrackedObjectCreateInfo = (PFN_gvkGetStateTrackedObjectCreateInfo)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedObjectCreateInfo");
        gvk_result(pfnGvkGetStateTrackedObjectCreateInfo ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkGetStateTrackedObjectAllocateInfo = (PFN_gvkGetStateTrackedObjectAllocateInfo)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedObjectAllocateInfo");
        gvk_result(pfnGvkGetStateTrackedObjectAllocateInfo ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkGetStateTrackedImageLayouts = (PFN_gvkGetStateTrackedImageLayouts)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedImageLayouts");
        gvk_result(pfnGvkGetStateTrackedImageLayouts ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkGetStateTrackedMappedMemory = (PFN_gvkGetStateTrackedMappedMemory)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedMappedMemory");
        gvk_result(pfnGvkGetStateTrackedMappedMemory ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkDisableStateTracker = (PFN_gvkDisableStateTracker)gvk_dlsym(dlStateTracker, "gvkDisableStateTracker");
        gvk_result(pfnGvkDisableStateTracker ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        pfnGvkEnableStateTracker = (PFN_gvkEnableStateTracker)gvk_dlsym(dlStateTracker, "gvkEnableStateTracker");
        gvk_result(pfnGvkGetStateTrackedImageLayouts ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        gvk_dlclose(dlStateTracker);
    } gvk_result_scope_end;

    mRestorePointInfo = restorePointInfo;
    std::cout << "===============================================================================" << std::endl;
    std::cout << "Beginning restore point creation" << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    pfnGvkDisableStateTracker();
    mCopyEngine.set_thread_initialization_callback(mRestorePointInfo.threadInitializationCallback);
    std::filesystem::create_directories(restorePointInfo.path);
    GvkStateTrackedObjectEnumerateInfo enumerateInfo { };
    enumerateInfo.pfnCallback = process_object;
    enumerateInfo.pUserData = this;
    GvkStateTrackedObject stateTrackedObject { };
    stateTrackedObject.type = VK_OBJECT_TYPE_INSTANCE;
    stateTrackedObject.handle = (uint64_t)instance;
    stateTrackedObject.dispatchableHandle = (uint64_t)instance;
    auto restoreInfo = get_default<GvkInstanceRestoreInfo>();
    auto vkResult = process_VkInstance(stateTrackedObject, restoreInfo);
    if (vkResult == VK_SUCCESS) {
        pfnGvkEnumerateStateTrackedObjects(&stateTrackedObject, &enumerateInfo);
    }
    GvkRestorePointManifest manifest { };
    manifest.objectCount = (uint32_t)mRestorePointObjects.size();
    manifest.pObjects = !mRestorePointObjects.empty() ? mRestorePointObjects.data() : nullptr;
    {
        auto jsonPath = (restorePointInfo.path / "GvkRestorePointManifest").replace_extension("json");
        std::ofstream jsonFile(jsonPath);
        assert(jsonFile.is_open());
        jsonFile << to_string(manifest, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
    }
    {
        auto infoPath = (restorePointInfo.path / "GvkRestorePointManifest").replace_extension("info");
        std::ofstream infoFile(infoPath, std::ios::binary);
        assert(infoFile.is_open());
        serialize(infoFile, manifest);
    }
    mCopyEngine.reset();
    pfnGvkEnableStateTracker();
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "Finished restore point creation" << std::endl;
    std::cout << "===============================================================================" << std::endl;
    return vkResult;
}

VkResult RestorePointCreator::process_VkInstance(const GvkStateTrackedObject& stateTrackedObject, GvkInstanceRestoreInfo& restoreInfo)
{
    Instance gvkInstance((VkInstance)stateTrackedObject.handle);
    assert(gvkInstance);
    std::vector<VkPhysicalDevice> vkPhysicalDevices;
    for (const auto& gvkPhysicalDevice : gvkInstance.get<PhysicalDevices>()) {
        vkPhysicalDevices.push_back(gvkPhysicalDevice);
    }
    restoreInfo.physicalDeviceCount = (uint32_t)vkPhysicalDevices.size();
    restoreInfo.pPhysicalDevices = !vkPhysicalDevices.empty() ? vkPhysicalDevices.data() : nullptr;
    return BasicRestorePointCreator::process_VkInstance(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkPhysicalDevice(const GvkStateTrackedObject& stateTrackedObject, GvkPhysicalDeviceRestoreInfo& restoreInfo)
{
    PhysicalDevice gvkPhysicalDevice((VkPhysicalDevice)stateTrackedObject.handle);
    assert(gvkPhysicalDevice);
    gvkPhysicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceFeatures(gvkPhysicalDevice, &restoreInfo.physicalDeviceFeatures);
    gvkPhysicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceProperties(gvkPhysicalDevice, &restoreInfo.physicalDeviceProperties);
    gvkPhysicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceMemoryProperties(gvkPhysicalDevice, &restoreInfo.physicalDeviceMemoryProperties);
    uint32_t queueFamilyPropertyCount = 0;
    gvkPhysicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceQueueFamilyProperties(gvkPhysicalDevice, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    gvkPhysicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceQueueFamilyProperties(gvkPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
    restoreInfo.queueFamilyPropertyCount = (uint32_t)queueFamilyProperties.size();
    restoreInfo.pQueueFamilyProperties = !queueFamilyProperties.empty() ? queueFamilyProperties.data() : nullptr;
    return BasicRestorePointCreator::process_VkPhysicalDevice(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkDevice(const GvkStateTrackedObject& stateTrackedObject, GvkDeviceRestoreInfo& restoreInfo)
{
    Device gvkDevice((VkDevice)stateTrackedObject.handle);
    assert(gvkDevice);
    std::vector<VkQueue> vkQueues;
    for (const auto& queueFamily : gvkDevice.get<QueueFamilies>()) {
        for (const auto& vkQueue : queueFamily.queues) {
            vkQueues.push_back(vkQueue);
        }
    }
    restoreInfo.queueCount = (uint32_t)vkQueues.size();
    restoreInfo.pQueues = !vkQueues.empty() ? vkQueues.data() : nullptr;
    return BasicRestorePointCreator::process_VkDevice(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkQueue(const GvkStateTrackedObject& stateTrackedObject, GvkQueueRestoreInfo& restoreInfo)
{
    Queue gvkQueue((VkQueue)stateTrackedObject.handle);
    assert(gvkQueue);
    restoreInfo.deviceQueueCreateInfo = gvkQueue.get<VkDeviceQueueCreateInfo>();
    return BasicRestorePointCreator::process_VkQueue(stateTrackedObject, restoreInfo);
}

#if 0
// NOTE : Implemented in "gvk-restore-point/source/generated/create-command-bufer-restore-point.cpp"
VkResult RestorePointCreator::process_VkCommandBuffer(const GvkStateTrackedObject& stateTrackedObject, GvkCommandBufferRestoreInfo& restoreInfo);
#endif

VkResult RestorePointCreator::process_VkDeviceMemory(const GvkStateTrackedObject& stateTrackedObject, GvkDeviceMemoryRestoreInfo& restoreInfo)
{
    class DeviceMemoryBindings
    {
    public:
        std::vector<VkBindBufferMemoryInfo> bufferBindInfos;
        std::vector<VkBindImageMemoryInfo> imageBindInfos;
    };
    auto enumerateDeviceMemoryBindings = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
    {
        assert(pInfo);
        assert(pUserData);
        switch (pInfo->sType) {
        case get_stype<VkBindBufferMemoryInfo>(): {
            ((DeviceMemoryBindings*)pUserData)->bufferBindInfos.push_back(*(VkBindBufferMemoryInfo*)pInfo);
        } break;
        case get_stype<VkBindImageMemoryInfo>(): {
            ((DeviceMemoryBindings*)pUserData)->imageBindInfos.push_back(*(VkBindImageMemoryInfo*)pInfo);
        } break;
        default: {
            assert(false && "TODO : Documentation");
        } break;
        }
    };
    DeviceMemoryBindings deviceMemoryBindings;
    GvkStateTrackedObjectEnumerateInfo enumerateInfo { };
    enumerateInfo.pfnCallback = enumerateDeviceMemoryBindings;
    enumerateInfo.pUserData = &deviceMemoryBindings;
    pfnGvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);
    restoreInfo.bufferBindInfoCount = (uint32_t)deviceMemoryBindings.bufferBindInfos.size();
    restoreInfo.pBufferBindInfos = !deviceMemoryBindings.bufferBindInfos.empty() ? deviceMemoryBindings.bufferBindInfos.data() : nullptr;
    restoreInfo.imageBindInfoCount = (uint32_t)deviceMemoryBindings.imageBindInfos.size();
    restoreInfo.pImageBindInfos = !deviceMemoryBindings.imageBindInfos.empty() ? deviceMemoryBindings.imageBindInfos.data() : nullptr;

    void* pData = &restoreInfo.mappedMemoryInfo.dataHandle;
    pfnGvkGetStateTrackedMappedMemory(&stateTrackedObject, &restoreInfo.mappedMemoryInfo.offset, &restoreInfo.mappedMemoryInfo.size, &restoreInfo.mappedMemoryInfo.flags, &pData);

    auto allocateInfoType = get_stype<VkMemoryAllocateInfo>();
    VkMemoryAllocateInfo memoryAllocateInfo { };
    pfnGvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &allocateInfoType, (VkBaseOutStructure*)&memoryAllocateInfo);
    assert(memoryAllocateInfo.sType == get_stype<VkMemoryAllocateInfo>());

    gvk::CopyEngine::DeviceMemoryCopyInfo deviceMemoryCopyInfo { };
    deviceMemoryCopyInfo.vkDeviceMemory = (VkDeviceMemory)stateTrackedObject.handle;
    deviceMemoryCopyInfo.allocateInfo = memoryAllocateInfo;
    deviceMemoryCopyInfo.path = mRestorePointInfo.path;
    deviceMemoryCopyInfo.onProcessData = mRestorePointInfo.processDeviceMemoryDataCallback;
    mCopyEngine.download((VkDevice)stateTrackedObject.dispatchableHandle, deviceMemoryCopyInfo);
    return BasicRestorePointCreator::process_VkDeviceMemory(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkDescriptorSet(const GvkStateTrackedObject& stateTrackedObject, GvkDescriptorSetRestoreInfo& restoreInfo)
{
    auto enumerateDescriptorBindings = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
    {
        assert(pInfo);
        assert(pInfo->sType == get_stype<VkWriteDescriptorSet>());
        assert(pUserData);
        ((std::vector<VkWriteDescriptorSet>*)pUserData)->push_back(*(VkWriteDescriptorSet*)pInfo);
    };
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    GvkStateTrackedObjectEnumerateInfo enumerateInfo { };
    enumerateInfo.pfnCallback = enumerateDescriptorBindings;
    enumerateInfo.pUserData = &writeDescriptorSets;
    pfnGvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);
    restoreInfo.descriptorWriteCount = (uint32_t)writeDescriptorSets.size();
    restoreInfo.pDescriptorWrites = !writeDescriptorSets.empty() ? writeDescriptorSets.data() : nullptr;
    return BasicRestorePointCreator::process_VkDescriptorSet(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkBuffer(const GvkStateTrackedObject& stateTrackedObject, GvkBufferRestoreInfo& restoreInfo)
{
    auto enumerateDeviceMemoryBindings = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
    {
        assert(pInfo);
        assert(pInfo->sType == get_stype<VkBindBufferMemoryInfo>());
        assert(pUserData);
        ((std::vector<VkBindBufferMemoryInfo>*)pUserData)->push_back(*(VkBindBufferMemoryInfo*)pInfo);
    };
    std::vector<VkBindBufferMemoryInfo> bindBufferMemoryInfos;
    GvkStateTrackedObjectEnumerateInfo enumerateInfo { };
    enumerateInfo.pfnCallback = enumerateDeviceMemoryBindings;
    enumerateInfo.pUserData = &bindBufferMemoryInfos;
    pfnGvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);
    restoreInfo.memoryBindInfoCount = (uint32_t)bindBufferMemoryInfos.size();
    restoreInfo.pMemoryBindInfos = !bindBufferMemoryInfos.empty() ? bindBufferMemoryInfos.data() : nullptr;
    return BasicRestorePointCreator::process_VkBuffer(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkImage(const GvkStateTrackedObject& stateTrackedObject, GvkImageRestoreInfo& restoreInfo)
{
    auto enumerateDeviceMemoryBindings = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
    {
        assert(pInfo);
        assert(pInfo->sType == get_stype<VkBindImageMemoryInfo>());
        assert(pUserData);
        ((std::vector<VkBindImageMemoryInfo>*)pUserData)->push_back(*(VkBindImageMemoryInfo*)pInfo);
    };
    std::vector<VkBindImageMemoryInfo> bindImageMemoryInfos;
    GvkStateTrackedObjectEnumerateInfo enumerateInfo { };
    enumerateInfo.pfnCallback = enumerateDeviceMemoryBindings;
    enumerateInfo.pUserData = &bindImageMemoryInfos;
    pfnGvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);
    restoreInfo.memoryBindInfoCount = (uint32_t)bindImageMemoryInfos.size();
    restoreInfo.pMemoryBindInfos = !bindImageMemoryInfos.empty() ? bindImageMemoryInfos.data() : nullptr;

    auto createInfoType = get_stype<VkImageCreateInfo>();
    VkImageCreateInfo imageCreateInfo { };
    pfnGvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&imageCreateInfo);
    assert(imageCreateInfo.sType == get_stype<VkImageCreateInfo>());
    auto imageSubresourceRange = get_default<VkImageSubresourceRange>();
    imageSubresourceRange.levelCount = imageCreateInfo.mipLevels;
    imageSubresourceRange.layerCount = imageCreateInfo.arrayLayers;
    auto imageSubresourceCount = imageSubresourceRange.levelCount * imageSubresourceRange.layerCount;
    std::vector<VkImageLayout> imageLayouts(imageSubresourceCount);
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedObject, &imageSubresourceRange, imageLayouts.data());
    restoreInfo.imageSubresourceCount = imageSubresourceCount;
    restoreInfo.pImageLayouts = !imageLayouts.empty() ? imageLayouts.data() : nullptr;
    return BasicRestorePointCreator::process_VkImage(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkEvent(const GvkStateTrackedObject& stateTrackedObject, GvkEventRestoreInfo& restoreInfo)
{
    Device gvkDevice((VkDevice)stateTrackedObject.dispatchableHandle);
    assert(gvkDevice);
    restoreInfo.status = gvkDevice.get<DispatchTable>().gvkGetEventStatus(gvkDevice, (VkEvent)stateTrackedObject.handle);
    return BasicRestorePointCreator::process_VkEvent(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkFence(const GvkStateTrackedObject& stateTrackedObject, GvkFenceRestoreInfo& restoreInfo)
{
    Device gvkDevice((VkDevice)stateTrackedObject.dispatchableHandle);
    assert(gvkDevice);
    auto vkResult = gvkDevice.get<DispatchTable>().gvkGetFenceStatus(gvkDevice, (VkFence)stateTrackedObject.handle);
    restoreInfo.signaled = vkResult == VK_SUCCESS;
    return BasicRestorePointCreator::process_VkFence(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkSemaphore(const GvkStateTrackedObject& stateTrackedObject, GvkSemaphoreRestoreInfo& restoreInfo)
{
    return BasicRestorePointCreator::process_VkSemaphore(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkQueryPool(const GvkStateTrackedObject& stateTrackedObject, GvkQueryPoolRestoreInfo& restoreInfo)
{
    (void)stateTrackedObject;
    (void)restoreInfo;
    return BasicRestorePointCreator::process_VkQueryPool(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkSurfaceKHR(const GvkStateTrackedObject& stateTrackedObject, GvkSurfaceRestoreInfoKHR& restoreInfo)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
    auto createInfoType = get_stype<VkWin32SurfaceCreateInfoKHR>();
    VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo { };
    pfnGvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&win32SurfaceCreateInfo);
    if (win32SurfaceCreateInfo.sType == get_stype<VkWin32SurfaceCreateInfoKHR>()) {
        RECT rect { };
        auto success = GetClientRect(win32SurfaceCreateInfo.hwnd, &rect);
        if (success) {
            restoreInfo.width = rect.right - rect.left;
            restoreInfo.height = rect.bottom - rect.top;
        }
    }
#endif
    return BasicRestorePointCreator::process_VkSurfaceKHR(stateTrackedObject, restoreInfo);
}

VkResult RestorePointCreator::process_VkSwapchainKHR(const GvkStateTrackedObject& stateTrackedObject, GvkSwapchainRestoreInfoKHR& restoreInfo)
{
    Device gvkDevice((VkDevice)stateTrackedObject.dispatchableHandle);
    assert(gvkDevice);
    uint32_t imageCount = 0;
    auto vkResult = gvkDevice.get<DispatchTable>().gvkGetSwapchainImagesKHR(gvkDevice, (VkSwapchainKHR)stateTrackedObject.handle, &imageCount, nullptr);
    std::vector<VkImage> vkImages(imageCount);
    vkResult = gvkDevice.get<DispatchTable>().gvkGetSwapchainImagesKHR(gvkDevice, (VkSwapchainKHR)stateTrackedObject.handle, &imageCount, vkImages.data());
    if (vkResult == VK_SUCCESS) {
        std::vector<GvkSwapchainImageRestoreInfo> swapchainImageRestoreInfos(vkImages.size());
        for (size_t i = 0; i < swapchainImageRestoreInfos.size(); ++i) {
            swapchainImageRestoreInfos[i].image = vkImages[i];
        }
        restoreInfo.imageCount = (uint32_t)swapchainImageRestoreInfos.size();
        restoreInfo.pImages = !swapchainImageRestoreInfos.empty() ? swapchainImageRestoreInfos.data() : nullptr;
        vkResult = BasicRestorePointCreator::process_VkSwapchainKHR(stateTrackedObject, restoreInfo);
    }
    return vkResult;
}

} // namespace restore_point
} // namespace gvk
