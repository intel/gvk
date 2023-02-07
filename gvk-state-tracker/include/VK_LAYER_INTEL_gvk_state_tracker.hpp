
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

#ifndef VK_LAYER_INTEL_gvk_state_tracker_hpp
#define VK_LAYER_INTEL_gvk_state_tracker_hpp 1

#include "VK_LAYER_INTEL_gvk_state_tracker.h"

#include <tuple>

namespace gvk {

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
