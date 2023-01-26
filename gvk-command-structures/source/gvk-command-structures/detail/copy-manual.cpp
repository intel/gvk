
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

#include "gvk-command-structures/generated/command-structure-create-copy.hpp"
#include "gvk-command-structures/generated/command-structure-destroy-copy.hpp"
#include "gvk-structures/detail/copy-utilities.hpp"

namespace gvk {
namespace detail {

GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureAllocateCommandBuffers)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureAllocateDescriptorSets)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureBuildAccelerationStructuresKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureCmdBuildAccelerationStructuresKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureCmdSetBlendConstants)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureCmdSetSampleMaskEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureCmdSetFragmentShadingRateEnumNV)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureCmdSetFragmentShadingRateKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureGetAccelerationStructureBuildSizesKHR)
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureCreateXlibSurfaceKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureGetPhysicalDeviceXlibPresentationSupportKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

} // namespace detail
} // namespace gvk
