
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

#include "gvk/detail/to-string-utilities.hpp"
#include "gvk/generated/enum-to-string.hpp"
#include "gvk/generated/handle-to-string.hpp"
#include "gvk/generated/structure-to-string.hpp"

namespace gvk {

#define GVK_STUB_TO_STRING_DEFINITION(VK_STRUCTURE_TYPE) \
template <> void print<VK_STRUCTURE_TYPE>(Printer&, const VK_STRUCTURE_TYPE&) { }

////////////////////////////////////////////////////////////////////////////////
// Linux
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_TO_STRING_DEFINITION(VkXlibSurfaceCreateInfoKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// Win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
GVK_STUB_TO_STRING_DEFINITION(LPCWSTR)
GVK_STUB_TO_STRING_DEFINITION(SECURITY_ATTRIBUTES)
GVK_STUB_TO_STRING_DEFINITION(VkExportFenceWin32HandleInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkExportMemoryWin32HandleInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkExportMemoryWin32HandleInfoNV)
GVK_STUB_TO_STRING_DEFINITION(VkExportSemaphoreWin32HandleInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkImportFenceWin32HandleInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkImportMemoryWin32HandleInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkImportMemoryWin32HandleInfoNV)
GVK_STUB_TO_STRING_DEFINITION(VkImportSemaphoreWin32HandleInfoKHR)
#endif // VK_USE_PLATFORM_WIN32_KHR

////////////////////////////////////////////////////////////////////////////////
// Video encode/decode
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264DpbSlotInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264MvcEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264PictureInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264SessionParametersAddInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265SessionParametersAddInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265DpbSlotInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265PictureInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264NaluSliceEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264ReferenceListsEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265NaluSliceSegmentEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265ReferenceListsEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265VclFrameInfoEXT)

////////////////////////////////////////////////////////////////////////////////
// Special case members
template <>
void print<VkAccelerationStructureBuildGeometryInfoKHR>(Printer& printer, const VkAccelerationStructureBuildGeometryInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            print_pnext(printer, obj.pNext);
            printer.print_field("type", obj.type);
            printer.print_flags<VkBuildAccelerationStructureFlagBitsKHR>("flags", obj.flags);
            printer.print_field("mode", obj.mode);
            printer.print_field("srcAccelerationStructure", obj.srcAccelerationStructure);
            printer.print_field("dstAccelerationStructure", obj.dstAccelerationStructure);
            printer.print_field("geometryCount", obj.geometryCount);
            printer.print_array("pGeometries", obj.geometryCount, obj.pGeometries);
            printer.print_array("ppGeometries", obj.geometryCount, obj.ppGeometries,
                [&](auto, auto pGeometry)
                {
                    if (pGeometry) {
                        print(printer, *pGeometry);
                    } else {
                        printer.mOstrm << "null";
                    }
                }
            );
            printer.print_field("scratchData", obj.scratchData);
        }
    );
}

template <>
void print<VkAccelerationStructureVersionInfoKHR>(Printer& printer, const VkAccelerationStructureVersionInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            print_pnext(printer, obj.pNext);
            printer.print_field("pVersionData", obj.pVersionData);
        }
    );
}

template <>
void print<VkPipelineMultisampleStateCreateInfo>(Printer& printer, const VkPipelineMultisampleStateCreateInfo& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            print_pnext(printer, obj.pNext);
            printer.print_field("flags", obj.flags);
            printer.print_flags<VkSampleCountFlagBits>("rasterizationSamples", obj.rasterizationSamples);
            printer.print_field("sampleShadingEnable", obj.sampleShadingEnable);
            printer.print_field("minSampleShading", obj.minSampleShading);
            printer.print_array("pSampleMask", (obj.rasterizationSamples + 31) / 32, obj.pSampleMask);
            printer.print_field("alphaToCoverageEnable", obj.alphaToCoverageEnable);
            printer.print_field("alphaToOneEnable", obj.alphaToOneEnable);
        }
    );
}

template <>
void print<VkShaderModuleCreateInfo>(Printer& printer, const VkShaderModuleCreateInfo& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            print_pnext(printer, obj.pNext);
            printer.print_field("flags", obj.flags);
            printer.print_field("codeSize", obj.codeSize);
            printer.print_array("pCode", obj.codeSize / sizeof(uint32_t), obj.pCode);
        }
    );
}

template <>
void print<VkTransformMatrixKHR>(Printer& printer, const VkTransformMatrixKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_array("matrix", 3, obj.matrix,
                [&](auto, auto row)
                {
                    printer.mOstrm << "[ ";
                    for (uint32_t i = 0; i < 4; ++i) {
                        if (i) {
                            printer.mOstrm << ", ";
                        }
                        print(printer, row[i]);
                    }
                    printer.mOstrm << " ]";
                }
            );
        }
    );
}

////////////////////////////////////////////////////////////////////////////////
// Unions
template <>
void print<VkAccelerationStructureGeometryDataKHR>(Printer& printer, const VkAccelerationStructureGeometryDataKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("triangles", obj.triangles);
            printer.print_field("aabbs", obj.aabbs);
            printer.print_field("instances", obj.instances);
        }
    );
}

template <>
void print<VkAccelerationStructureMotionInstanceDataNV>(Printer& printer, const VkAccelerationStructureMotionInstanceDataNV& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("staticInstance", obj.staticInstance);
            printer.print_field("matrixMotionInstance", obj.matrixMotionInstance);
            printer.print_field("srtMotionInstance", obj.srtMotionInstance);
        }
    );
}

template <>
void print<VkClearColorValue>(Printer& printer, const VkClearColorValue& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_array("float32", 4, obj.float32);
            printer.print_array("int32", 4, obj.int32);
            printer.print_array("uint32", 4, obj.uint32);
        }
    );
}

template <>
void print<VkClearValue>(Printer& printer, const VkClearValue& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("color", obj.color);
            printer.print_field("depthStencil", obj.depthStencil);
        }
    );
}

template <>
void print<VkDeviceOrHostAddressConstKHR>(Printer& printer, const VkDeviceOrHostAddressConstKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("deviceAddress", obj.deviceAddress);
            printer.print_field("hostAddress", obj.hostAddress);
        }
    );
}

template <>
void print<VkDeviceOrHostAddressKHR>(Printer& printer, const VkDeviceOrHostAddressKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("deviceAddress", obj.deviceAddress);
            printer.print_field("hostAddress", obj.hostAddress);
        }
    );
}

template <>
void print<VkPerformanceCounterResultKHR>(Printer& printer, const VkPerformanceCounterResultKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("int32", obj.int32);
            printer.print_field("int64", obj.int64);
            printer.print_field("uint32", obj.uint32);
            printer.print_field("uint64", obj.uint64);
            printer.print_field("float32", obj.float32);
            printer.print_field("float64", obj.float64);
        }
    );
}

template <>
void print<VkPerformanceValueDataINTEL>(Printer&, const VkPerformanceValueDataINTEL&)
{
    // NOPE :
}

template <>
void print<VkPipelineExecutableStatisticValueKHR>(Printer& printer, const VkPipelineExecutableStatisticValueKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("b32", obj.b32);
            printer.print_field("i64", obj.i64);
            printer.print_field("u64", obj.u64);
            printer.print_field("f64", obj.f64);
        }
    );
}

} // namespace gvk
