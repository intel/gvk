
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

#ifndef VK_LAYER_INTEL_gvk_restore_point_h
#define VK_LAYER_INTEL_gvk_restore_point_h 1
#ifdef __cplusplus
extern "C" {
#endif

#include "VK_LAYER_INTEL_gvk_state_tracker.h"

#include "vulkan/vulkan.h"

VK_DEFINE_NON_DISPATCHABLE_HANDLE(GvkRestorePoint)
#define VK_LAYER_INTEL_GVK_RESTORE_POINT_NAME "VK_LAYER_INTEL_gvk_restore_point"

typedef void(VKAPI_PTR* PFN_gvkInitializeThreadCallback)();
typedef void(VKAPI_PTR* PFN_gvkProcessResourceDataCallback)(const GvkStateTrackedObject* pRestorePointObject, VkDeviceMemory stagingMemory, VkDeviceSize size, const uint8_t* pData);
typedef void(VKAPI_PTR* PFN_gvkProcessRestoredObjectCallback)(const GvkStateTrackedObject* pCapturedObject, const GvkStateTrackedObject* pRestoredObject);
#ifdef VK_USE_PLATFORM_WIN32_KHR
typedef void(VKAPI_PTR* PFN_gvkProcessWin32SurfaceCreateInfoCallback)(uint32_t width, uint32_t height, VkWin32SurfaceCreateInfoKHR* pWin32SurfaceCreateInfo);
#endif

typedef enum GvkRestorePointCreateFlagBits {
    GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT = 0x00000001,
    GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT = 0x00000002,
    GVK_RESTORE_POINT_CREATE_DEVICE_MEMORY_DATA_BIT = 0x00000004,
    GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT = 0x00000008,
    GVK_RESTORE_POINT_CREATE_BUFFER_DATA_BIT = 0x00000010,
    GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT = 0x00000020,
    GVK_RESTORE_POINT_CREATE_IMAGE_PNG_BIT = 0x00000040,
    GVK_RESTORE_POINT_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} GvkRestorePointCreateFlagBits;
typedef VkFlags GvkRestorePointCreateFlags;

typedef enum GvkRestorePointApplyFlagBits {
    GVK_RESTORE_POINT_APPLY_FORCE_FULL_RESTORATION = 0x00000001,
    GVK_RESTORE_POINT_APPLY_FLATTEN_COMMAND_BUFFERS_BIT = 0x00000002,
    GVK_RESTORE_POINT_APPLY_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} GvkRestorePointApplyFlagBits;
typedef VkFlags GvkRestorePointApplyFlags;

typedef struct GvkRestorePointCreateInfo {
    GvkRestorePointCreateFlags flags;
    const char* pPath;
    const wchar_t* pwPath;
    uint32_t threadCount;
    PFN_gvkInitializeThreadCallback pfnInitializeThreadCallback;
    PFN_gvkProcessResourceDataCallback pfnProcessResourceDataCallback;
    VkBool32 repeating_HACK;
} GvkRestorePointCreateInfo;

typedef struct GvkRestorePointApplyInfo {
    GvkRestorePointApplyFlags flags;
    const char* pPath;
    const wchar_t* pwPath;
    uint32_t threadCount;
    uint32_t excludedObjectCount;
    const GvkStateTrackedObject* pExcludedObjects;
    PFN_gvkInitializeThreadCallback pfnInitializeThreadCallback;
    PFN_gvkProcessRestoredObjectCallback pfnProcessRestoredObjectCallback;
    PFN_gvkProcessResourceDataCallback pfnProcessResourceDataCallback;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    PFN_gvkProcessWin32SurfaceCreateInfoCallback pfnProcessWin32SurfaceCreateInfoCallback;
#endif
    PFN_vkGetInstanceProcAddr pfnGetInstanceProcAddr;
    VkBool32 repeating_HACK;
} GvkRestorePointApplyInfo;

typedef VkResult(VKAPI_PTR* PFN_gvkCreateRestorePoint)(VkInstance instance, const GvkRestorePointCreateInfo* pCreateInfo, GvkRestorePoint* pRestorePoint);
typedef VkResult(VKAPI_PTR* PFN_gvkGetRestorePointObjects)(VkInstance instance, GvkRestorePoint restorePoint, uint32_t* pRestorePointObjectCount, GvkStateTrackedObject* pRestorePointObjects);
typedef VkResult(VKAPI_PTR* PFN_gvkApplyRestorePoint)(VkInstance instance, const GvkRestorePointApplyInfo* pApplyInfo, GvkRestorePoint restorePoint);
typedef void(VKAPI_PTR* PFN_gvkDestroyRestorePoint)(VkInstance instance, GvkRestorePoint restorePoint);

#ifdef __cplusplus
}
#endif
#endif // VK_LAYER_INTEL_gvk_restore_point_h
