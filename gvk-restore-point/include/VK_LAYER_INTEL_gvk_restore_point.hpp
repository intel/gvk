
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

#ifndef VK_LAYER_INTEL_gvk_restore_point_hpp_OMIT_ENTRY_POINT_DECLARATIONS
#define VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS 1
#endif // VK_LAYER_INTEL_gvk_restore_point_hpp_OMIT_ENTRY_POINT_DECLARATIONS

#ifndef VK_LAYER_INTEL_gvk_restore_point_hpp
#define VK_LAYER_INTEL_gvk_restore_point_hpp 1

#include "VK_LAYER_INTEL_gvk_restore_point.h"
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#ifdef VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS
extern PFN_gvkCreateRestorePoint gvkCreateRestorePoint;
extern PFN_gvkGetRestorePointObjects gvkGetRestorePointObjects;
extern PFN_gvkApplyRestorePoint gvkApplyRestorePoint;
extern PFN_gvkDestroyRestorePoint gvkDestroyRestorePoint;
#endif // VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS

namespace gvk {
namespace restore_point {

#ifdef VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS
VkResult load_layer_entry_points();
#endif // VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS

} // namespace restore_point
} // namespace gvk

#endif // VK_LAYER_INTEL_gvk_restore_point_hpp

#ifdef VK_LAYER_INTEL_gvk_restore_point_hpp_IMPLEMENTATION

#include "gvk-defines.hpp"

#ifdef VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS
PFN_gvkCreateRestorePoint gvkCreateRestorePoint;
PFN_gvkGetRestorePointObjects gvkGetRestorePointObjects;
PFN_gvkApplyRestorePoint gvkApplyRestorePoint;
PFN_gvkDestroyRestorePoint gvkDestroyRestorePoint;
#define VK_LAYER_INTEL_LOAD_GVK_RESTORE_POINT_LAYER_ENTRY_POINT(GVK_RESTORE_POINT_LAYER_ENTRY_POINT_NAME)                                                 \
GVK_RESTORE_POINT_LAYER_ENTRY_POINT_NAME = (PFN_##GVK_RESTORE_POINT_LAYER_ENTRY_POINT_NAME)gvk_dlsym(dlLayer, #GVK_RESTORE_POINT_LAYER_ENTRY_POINT_NAME); \
gvk_result(GVK_RESTORE_POINT_LAYER_ENTRY_POINT_NAME ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
#endif // VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS

namespace gvk {
namespace restore_point {

#ifdef VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS
VkResult load_layer_entry_points()
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto dlLayer = gvk_dlopen(VK_LAYER_INTEL_GVK_RESTORE_POINT_NAME);
        gvk_result(dlLayer ? VK_SUCCESS : VK_ERROR_LAYER_NOT_PRESENT);
        VK_LAYER_INTEL_LOAD_GVK_RESTORE_POINT_LAYER_ENTRY_POINT(gvkCreateRestorePoint);
        VK_LAYER_INTEL_LOAD_GVK_RESTORE_POINT_LAYER_ENTRY_POINT(gvkGetRestorePointObjects);
        VK_LAYER_INTEL_LOAD_GVK_RESTORE_POINT_LAYER_ENTRY_POINT(gvkApplyRestorePoint);
        VK_LAYER_INTEL_LOAD_GVK_RESTORE_POINT_LAYER_ENTRY_POINT(gvkDestroyRestorePoint);
    } gvk_result_scope_end;
    return gvkResult;
}
#endif // VK_LAYER_INTEL_gvk_restore_point_hpp_DECLARE_ENTRY_POINTS

} // namespace restore_point
} // namespace gvk

#endif // VK_LAYER_INTEL_gvk_restore_point_hpp_IMPLEMENTATION
