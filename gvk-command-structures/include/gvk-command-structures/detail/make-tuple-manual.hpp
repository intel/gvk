
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
#include "gvk-command-structures/generated/command.h"
#include "gvk-structures/detail/make-tuple-manual.hpp"

#include <tuple>

namespace gvk {

GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureAllocateCommandBuffers)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureAllocateDescriptorSets)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureBuildAccelerationStructuresKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdBuildAccelerationStructuresKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdSetBlendConstants)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdSetSampleMaskEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdSetFragmentShadingRateEnumNV)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdSetFragmentShadingRateKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureGetAccelerationStructureBuildSizesKHR)
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCreateXlibSurfaceKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureGetPhysicalDeviceXlibPresentationSupportKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

inline auto make_tuple(const GvkCommandStructureCmdPushConstants& obj)
{
    return std::make_tuple(
        obj.sType,
        obj.commandBuffer,
        obj.layout,
        obj.stageFlags,
        obj.offset,
        obj.size,
        detail::ArrayTupleElementWrapper<uint8_t> { obj.size, (const uint8_t*)obj.pValues }
    );
}

} // namespace gvk
