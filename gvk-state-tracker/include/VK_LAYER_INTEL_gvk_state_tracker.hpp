
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

#ifndef VK_LAYER_INTEL_gvk_state_tracker_hpp_OMIT_ENTRY_POINT_DECLARATIONS
#define VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS 1
#endif // VK_LAYER_INTEL_gvk_state_tracker_hpp_OMIT_ENTRY_POINT_DECLARATIONS

#ifndef VK_LAYER_INTEL_gvk_state_tracker_hpp
#define VK_LAYER_INTEL_gvk_state_tracker_hpp 1

#include "VK_LAYER_INTEL_gvk_state_tracker.h"

#include <tuple>

#ifdef VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS
extern PFN_gvkGetStateTrackerPhysicalDevice gvkGetStateTrackerPhysicalDevice;
extern PFN_gvkEnumerateStateTrackedObjects gvkEnumerateStateTrackedObjects;
extern PFN_gvkEnumerateStateTrackedObjectDependencies gvkEnumerateStateTrackedObjectDependencies;
extern PFN_gvkEnumerateStateTrackedObjectBindings gvkEnumerateStateTrackedObjectBindings;
extern PFN_gvkEnumerateStateTrackedCommandBufferCmds gvkEnumerateStateTrackedCommandBufferCmds;
extern PFN_gvkGetStateTrackedObjectInfo gvkGetStateTrackedObjectInfo;
extern PFN_gvkGetStateTrackedObjectCreateInfo gvkGetStateTrackedObjectCreateInfo;
extern PFN_gvkGetStateTrackedObjectAllocateInfo gvkGetStateTrackedObjectAllocateInfo;
extern PFN_gvkGetStateTrackedImageLayouts gvkGetStateTrackedImageLayouts;
extern PFN_gvkGetStateTrackedMappedMemory gvkGetStateTrackedMappedMemory;
extern PFN_gvkGetStateTrackedAcclerationStructureBuildInfo gvkGetStateTrackedAcclerationStructureBuildInfo;
extern PFN_gvkDisableStateTracker gvkDisableStateTracker;
extern PFN_gvkEnableStateTracker gvkEnableStateTracker;
#endif // VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS

namespace gvk {
namespace state_tracker {

#ifdef VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS
VkResult load_layer_entry_points();
#endif // VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS

} // namespace state_tracker

template <typename GvkHandleType>
inline GvkStateTrackedObject get_state_tracked_object(const GvkHandleType& gvkHandle)
{
    GvkStateTrackedObject stateTrackedObject { };
    stateTrackedObject.type = gvkHandle.template get<VkObjectType>();
    stateTrackedObject.handle = (uint64_t)gvkHandle.template get<typename GvkHandleType::VkHandleType>();
    stateTrackedObject.dispatchableHandle = (uint64_t)gvkHandle.template get<typename GvkHandleType::DispatchableVkHandleType>();
    return stateTrackedObject;
}

inline auto make_tuple(const GvkStateTrackedObjectInfo& stateTrackedObjectInfo)
{
    return std::make_tuple(
        stateTrackedObjectInfo.flags
    );
}

inline auto make_tuple(const GvkStateTrackedObject& stateTrackedObject)
{
    return std::make_tuple(
        stateTrackedObject.type,
        stateTrackedObject.handle,
        stateTrackedObject.dispatchableHandle
    );
}

} // namespace gvk

inline bool operator==(const GvkStateTrackedObjectInfo& lhs, const GvkStateTrackedObjectInfo& rhs) { return gvk::make_tuple(lhs) == gvk::make_tuple(rhs); }
inline bool operator!=(const GvkStateTrackedObjectInfo& lhs, const GvkStateTrackedObjectInfo& rhs) { return gvk::make_tuple(lhs) != gvk::make_tuple(rhs); }
inline bool operator<(const GvkStateTrackedObjectInfo& lhs, const GvkStateTrackedObjectInfo& rhs) { return gvk::make_tuple(lhs) < gvk::make_tuple(rhs); }
inline bool operator>(const GvkStateTrackedObjectInfo& lhs, const GvkStateTrackedObjectInfo& rhs) { return gvk::make_tuple(lhs) > gvk::make_tuple(rhs); }
inline bool operator<=(const GvkStateTrackedObjectInfo& lhs, const GvkStateTrackedObjectInfo& rhs) { return gvk::make_tuple(lhs) <= gvk::make_tuple(rhs); }
inline bool operator>=(const GvkStateTrackedObjectInfo& lhs, const GvkStateTrackedObjectInfo& rhs) { return gvk::make_tuple(lhs) >= gvk::make_tuple(rhs); }
inline bool operator==(const GvkStateTrackedObject& lhs, const GvkStateTrackedObject& rhs) { return gvk::make_tuple(lhs) == gvk::make_tuple(rhs); }
inline bool operator!=(const GvkStateTrackedObject& lhs, const GvkStateTrackedObject& rhs) { return gvk::make_tuple(lhs) != gvk::make_tuple(rhs); }
inline bool operator<(const GvkStateTrackedObject& lhs, const GvkStateTrackedObject& rhs) { return gvk::make_tuple(lhs) < gvk::make_tuple(rhs); }
inline bool operator>(const GvkStateTrackedObject& lhs, const GvkStateTrackedObject& rhs) { return gvk::make_tuple(lhs) > gvk::make_tuple(rhs); }
inline bool operator<=(const GvkStateTrackedObject& lhs, const GvkStateTrackedObject& rhs) { return gvk::make_tuple(lhs) <= gvk::make_tuple(rhs); }
inline bool operator>=(const GvkStateTrackedObject& lhs, const GvkStateTrackedObject& rhs) { return gvk::make_tuple(lhs) >= gvk::make_tuple(rhs); }

#endif // VK_LAYER_INTEL_gvk_state_tracker_hpp

#ifdef VK_LAYER_INTEL_gvk_state_tracker_hpp_IMPLEMENTATION

#include "gvk-defines.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-runtime.hpp"

#ifdef VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS
PFN_gvkGetStateTrackerPhysicalDevice gvkGetStateTrackerPhysicalDevice;
PFN_gvkEnumerateStateTrackedObjects gvkEnumerateStateTrackedObjects;
PFN_gvkEnumerateStateTrackedObjectDependencies gvkEnumerateStateTrackedObjectDependencies;
PFN_gvkEnumerateStateTrackedObjectBindings gvkEnumerateStateTrackedObjectBindings;
PFN_gvkEnumerateStateTrackedCommandBufferCmds gvkEnumerateStateTrackedCommandBufferCmds;
PFN_gvkGetStateTrackedObjectInfo gvkGetStateTrackedObjectInfo;
PFN_gvkGetStateTrackedObjectCreateInfo gvkGetStateTrackedObjectCreateInfo;
PFN_gvkGetStateTrackedObjectAllocateInfo gvkGetStateTrackedObjectAllocateInfo;
PFN_gvkGetStateTrackedImageLayouts gvkGetStateTrackedImageLayouts;
PFN_gvkGetStateTrackedMappedMemory gvkGetStateTrackedMappedMemory;
PFN_gvkGetStateTrackedAcclerationStructureBuildInfo gvkGetStateTrackedAcclerationStructureBuildInfo;
PFN_gvkDisableStateTracker gvkDisableStateTracker;
PFN_gvkEnableStateTracker gvkEnableStateTracker;
#define VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(GVK_STATE_TRACKER_LAYER_ENTRY_POINT_NAME)                                                 \
GVK_STATE_TRACKER_LAYER_ENTRY_POINT_NAME = (PFN_##GVK_STATE_TRACKER_LAYER_ENTRY_POINT_NAME)gvk_dlsym(dlLayer, #GVK_STATE_TRACKER_LAYER_ENTRY_POINT_NAME); \
gvk_result(GVK_STATE_TRACKER_LAYER_ENTRY_POINT_NAME ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
#endif // VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS

namespace gvk {
namespace state_tracker {

#ifdef VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS
VkResult load_layer_entry_points()
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto dlLayer = gvk_dlopen(VK_LAYER_INTEL_GVK_STATE_TRACKER_NAME);
        gvk_result(dlLayer ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkGetStateTrackerPhysicalDevice);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkEnumerateStateTrackedObjects);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkEnumerateStateTrackedObjectDependencies);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkEnumerateStateTrackedObjectBindings);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkEnumerateStateTrackedCommandBufferCmds);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkGetStateTrackedObjectInfo);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkGetStateTrackedObjectCreateInfo);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkGetStateTrackedObjectAllocateInfo);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkGetStateTrackedImageLayouts);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkGetStateTrackedMappedMemory);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkGetStateTrackedAcclerationStructureBuildInfo);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkDisableStateTracker);
        VK_LAYER_INTEL_LOAD_GVK_STATE_TRACKER_LAYER_ENTRY_POINT(gvkEnableStateTracker);
    } gvk_result_scope_end;
    return gvkResult;
}
#endif // VK_LAYER_INTEL_gvk_state_tracker_hpp_DECLARE_ENTRY_POINTS

} // namespace state_tracker
} // namespace gvk

#endif // VK_LAYER_INTEL_gvk_state_tracker_hpp_IMPLEMENTATION
