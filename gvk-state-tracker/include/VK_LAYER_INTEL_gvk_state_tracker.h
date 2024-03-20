
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

#ifndef VK_LAYER_INTEL_gvk_state_tracker_h
#define VK_LAYER_INTEL_gvk_state_tracker_h 1
#ifdef __cplusplus
extern "C" {
#endif

#include "vulkan/vulkan.h"

#define VK_LAYER_INTEL_GVK_STATE_TRACKER_NAME "VK_LAYER_INTEL_gvk_state_tracker"

typedef enum GvkStateTrackedObjectStatusFlagBits {
    GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT = 0x00000001,
    GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT = 0x00000002,
    GVK_STATE_TRACKER_OBJECT_STATUS_RECORDING_BIT = 0x00000004,
    GVK_STATE_TRACKER_OBJECT_STATUS_EXECUTABLE_BIT = 0x00000008,
    GVK_STATE_TRACKER_OBJECT_STATUS_PENDING_BIT = 0x00000010,
    GVK_STATE_TRACKER_OBJECT_STATUS_INVALID_BIT = 0x00000020,
    GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT = 0x00000040,
    GVK_STATE_TRACKER_OBJECT_STATUS_ACQUIRED_BIT = 0x00000080,
    GVK_STATE_TRACKER_OBJECT_STATUS_ALL_COMMAND_BUFFER_BIT = GVK_STATE_TRACKER_OBJECT_STATUS_RECORDING_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_EXECUTABLE_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_PENDING_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_INVALID_BIT,
    GVK_STATE_TRACKED_OBJECT_STATUS_BITS_MAX_ENUM = 0x7FFFFFFF
} GvkStateTrackedObjectStatusBits;
typedef VkFlags GvkStateTrackedObjectStatusFlags;

typedef struct GvkStateTrackedObjectInfo {
    GvkStateTrackedObjectStatusFlags flags;
    const char* pName;
} GvkStateTrackedObjectInfo;

typedef struct GvkStateTrackedObject {
    VkObjectType type;
    uint64_t handle;
    uint64_t dispatchableHandle;
} GvkStateTrackedObject;

typedef void(VKAPI_PTR* PFN_gvkEnumerateStateTrackedObjectsCallback)(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData);

typedef struct GvkStateTrackedObjectEnumerateInfo {
    VkPipelineBindPoint bindPoint;
    PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback;
    void* pUserData;
} GvkStateTrackedObjectEnumerateInfo;

typedef void(VKAPI_PTR* PFN_gvkGetStateTrackerPhysicalDevice)(VkInstance instance, VkPhysicalDevice physicalDevice, VkPhysicalDevice* pStateTrackerPhysicalDevice);
typedef void(VKAPI_PTR* PFN_gvkEnumerateStateTrackedObjects)(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo);
typedef void(VKAPI_PTR* PFN_gvkEnumerateStateTrackedObjectDependencies)(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo);
typedef void(VKAPI_PTR* PFN_gvkEnumerateStateTrackedObjectBindings)(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo);
typedef void(VKAPI_PTR* PFN_gvkEnumerateStateTrackedCommandBufferCmds)(const GvkStateTrackedObject* pStateTrackedCommandBuffer, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo);
typedef void(VKAPI_PTR* PFN_gvkGetStateTrackedObjectInfo)(const GvkStateTrackedObject* pStateTrackedObject, GvkStateTrackedObjectInfo* pStateTrackedObjectInfo);
typedef void(VKAPI_PTR* PFN_gvkGetStateTrackedObjectCreateInfo)(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo);
typedef void(VKAPI_PTR* PFN_gvkGetStateTrackedObjectAllocateInfo)(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pAllocateInfoType, VkBaseOutStructure* pAllocateInfo);
typedef void(VKAPI_PTR* PFN_gvkGetStateTrackedImageLayouts)(const GvkStateTrackedObject* pStateTrackedImage, const VkImageSubresourceRange* pSubresourceRange, VkImageLayout* pImageLayouts);
typedef void(VKAPI_PTR* PFN_gvkGetStateTrackedMappedMemory)(const GvkStateTrackedObject* pStateTrackedDeviceMemory, VkDeviceSize* pOffset, VkDeviceSize* pSize, VkMemoryMapFlags* pFlags, void** ppData);
typedef void(VKAPI_PTR* PFN_gvkGetStateTrackedAcclerationStructureBuildInfo)(const GvkStateTrackedObject* pStateTrackedAcclerationStructure, VkAccelerationStructureBuildGeometryInfoKHR* pBuildGeometryInfo, VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos);
typedef void(VKAPI_PTR* PFN_gvkDisableStateTracker)();
typedef void(VKAPI_PTR* PFN_gvkEnableStateTracker)();

#ifdef __cplusplus
}
#endif
#endif // VK_LAYER_INTEL_gvk_state_tracker_h
