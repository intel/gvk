
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
#include "gvk-command-structures/generated/command.h"
#include "gvk-command-structures/generated/command-enumerations-to-string.hpp"
#include "gvk-command-structures/generated/command-structure-to-string.hpp"
#include "gvk-structures.hpp"

#include <algorithm>

namespace gvk {

#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_TO_STRING_DEFINITION(GvkCommandStructureCreateXlibSurfaceKHR)
GVK_STUB_TO_STRING_DEFINITION(GvkCommandStructureGetPhysicalDeviceXlibPresentationSupportKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

template <>
void print<GvkCommandStructureAllocateCommandBuffers>(Printer& printer, const GvkCommandStructureAllocateCommandBuffers& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("device", obj.device);
            printer.print_field("pAllocateInfo", obj.pAllocateInfo);
            printer.print_array("pCommandBuffers", obj.pAllocateInfo ? obj.pAllocateInfo->commandBufferCount : 0, obj.pCommandBuffers);
            printer.print_field("result", obj.result);
        }
    );
}

template <>
void print<GvkCommandStructureAllocateDescriptorSets>(Printer& printer, const GvkCommandStructureAllocateDescriptorSets& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("device", obj.device);
            printer.print_field("pAllocateInfo", obj.pAllocateInfo);
            printer.print_array("pDescriptorSets", obj.pAllocateInfo ? obj.pAllocateInfo->descriptorSetCount : 0, obj.pDescriptorSets);
            printer.print_field("result", obj.result);
        }
    );
}

template <>
void print<GvkCommandStructureBuildAccelerationStructuresKHR>(Printer& printer, const GvkCommandStructureBuildAccelerationStructuresKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("device", obj.device);
            printer.print_field("deferredOperation", obj.deferredOperation);
            printer.print_field("infoCount", obj.infoCount);
            printer.print_array("pInfos", obj.infoCount, obj.pInfos);
            for (uint32_t i = 0; i < obj.infoCount; ++i) {
                printer.print_array("ppBuildRangeInfos", obj.pInfos[i].geometryCount, obj.ppBuildRangeInfos[i]);
            }
            printer.print_field("result", obj.result);
        }
    );
}

template <>
void print<GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR>(Printer& printer, const GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("commandBuffer", obj.commandBuffer);
            printer.print_field("infoCount", obj.infoCount);
            printer.print_array("pInfos", obj.infoCount, obj.pInfos);
            printer.print_array("pInfos", obj.infoCount, obj.pIndirectDeviceAddresses);
            printer.print_array("pInfos", obj.infoCount, obj.pIndirectStrides);
            for (uint32_t i = 0; i < obj.infoCount; ++i) {
                printer.print_array("ppMaxPrimitiveCounts", obj.pInfos[i].geometryCount, obj.ppMaxPrimitiveCounts[i]);
            }
        }
    );
}

template <>
void print<GvkCommandStructureCmdBuildAccelerationStructuresKHR>(Printer& printer, const GvkCommandStructureCmdBuildAccelerationStructuresKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("commandBuffer", obj.commandBuffer);
            printer.print_field("infoCount", obj.infoCount);
            printer.print_array("pInfos", obj.infoCount, obj.pInfos);
            for (uint32_t i = 0; i < obj.infoCount; ++i) {
                printer.print_array("ppBuildRangeInfos", obj.pInfos[i].geometryCount, obj.ppBuildRangeInfos[i]);
            }
        }
    );
}

template <>
void print<GvkCommandStructureCmdPushConstants>(Printer& printer, const GvkCommandStructureCmdPushConstants& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("commandBuffer", obj.commandBuffer);
            printer.print_field("layout", obj.layout);
            printer.print_flags<VkShaderStageFlagBits>("stageFlags", obj.stageFlags);
            printer.print_field("offset", obj.offset);
            printer.print_field("size", obj.size);
            printer.print_array("pValues", obj.size, (const uint8_t*)obj.pValues);
        }
    );
}

template <>
void print<GvkCommandStructureCmdSetBlendConstants>(Printer& printer, const GvkCommandStructureCmdSetBlendConstants& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("commandBuffer", obj.commandBuffer);
            printer.print_array("blendConstants", 4, obj.blendConstants);
        }
    );
}

template <>
void print<GvkCommandStructureCmdSetSampleMaskEXT>(Printer& printer, const GvkCommandStructureCmdSetSampleMaskEXT& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("commandBuffer", obj.commandBuffer);
            printer.print_field("samples", obj.samples);
            printer.print_array("sampleMask", std::max(1, obj.samples / 32), obj.pSampleMask);
        }
    );
}

template <>
void print<GvkCommandStructureCmdSetFragmentShadingRateEnumNV>(Printer& printer, const GvkCommandStructureCmdSetFragmentShadingRateEnumNV& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("commandBuffer", obj.commandBuffer);
            printer.print_field("shadingRate", obj.shadingRate);
            printer.print_array("combinerOps", 2, obj.combinerOps);
        }
    );
}

template <>
void print<GvkCommandStructureCmdSetFragmentShadingRateKHR>(Printer& printer, const GvkCommandStructureCmdSetFragmentShadingRateKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("commandBuffer", obj.commandBuffer);
            printer.print_pointer("pFragmentSize", obj.pFragmentSize);
            printer.print_array("combinerOps", 2, obj.combinerOps);
        }
    );
}

template <>
void print<GvkCommandStructureGetAccelerationStructureBuildSizesKHR>(Printer& printer, const GvkCommandStructureGetAccelerationStructureBuildSizesKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("device", obj.device);
            printer.print_field("buildType", obj.buildType);
            printer.print_pointer("pBuildInfo", obj.pBuildInfo);
            printer.print_pointer("pMaxPrimitiveCounts", obj.pMaxPrimitiveCounts);
            printer.print_pointer("pSizeInfo", obj.pSizeInfo);
        }
    );
}

} // namespace gvk
