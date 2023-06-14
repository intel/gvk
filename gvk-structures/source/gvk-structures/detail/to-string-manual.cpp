
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
#include "gvk-structures/generated/core-structure-to-string.hpp"
#include "gvk-structures/generated/core-enumerations-to-string.hpp"
#include "gvk-structures/generated/handle-to-string.hpp"
#include "gvk-structures/detail/to-string-utilities.hpp"

namespace gvk {

////////////////////////////////////////////////////////////////////////////////
// Linux
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_TO_STRING_DEFINITION(VkXlibSurfaceCreateInfoKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// Win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
template <> void print<HINSTANCE>(Printer& printer, const HINSTANCE& hInstance)
{
    printer.mOstrm << "\"" << (hInstance ? to_hex_string(hInstance) : "NULL") << "\"";
}

template <> void print<HWND>(Printer& printer, const HWND& hWnd)
{
    printer.mOstrm << "\"" << (hWnd ? to_hex_string(hWnd) : "NULL") << "\"";
}

template <> void print<LPCWSTR>(Printer& printer, const LPCWSTR& pwStr)
{
    printer.mOstrm << "\"";
    if (pwStr) {
        auto strLen = WideCharToMultiByte(CP_UTF8, 0, pwStr, -1, NULL, 0, NULL, NULL);
        std::string str(strLen, 0);
        WideCharToMultiByte(CP_UTF8, 0, pwStr, -1, str.data(), 0, NULL, NULL);
        printer.mOstrm << str;
    } else {
        printer.mOstrm << "NULL";
    }
    printer.mOstrm << "\"";
}

template <> void print<SECURITY_ATTRIBUTES>(Printer& printer, const SECURITY_ATTRIBUTES& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("nLength", obj.nLength);
            printer.print_field("lpSecurityDescriptor", obj.lpSecurityDescriptor);
            printer.print_field("bInheritHandle", obj.bInheritHandle);
        }
    );
}

template <> void print<VkExportFenceWin32HandleInfoKHR>(Printer& printer, const VkExportFenceWin32HandleInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_pointer("pAttributes", obj.pAttributes);
            printer.print_field("dwAccess", obj.dwAccess);
            printer.print_field("name", obj.name);
        }
    );
}

template <> void print<VkExportMemoryWin32HandleInfoKHR>(Printer& printer, const VkExportMemoryWin32HandleInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_pointer("pAttributes", obj.pAttributes);
            printer.print_field("dwAccess", obj.dwAccess);
            printer.print_field("name", obj.name);
        }
    );
}

template <> void print<VkExportMemoryWin32HandleInfoNV>(Printer& printer, const VkExportMemoryWin32HandleInfoNV& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_pointer("pAttributes", obj.pAttributes);
            printer.print_field("dwAccess", obj.dwAccess);
        }
    );
}

template <> void print<VkExportSemaphoreWin32HandleInfoKHR>(Printer& printer, const VkExportSemaphoreWin32HandleInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_pointer("pAttributes", obj.pAttributes);
            printer.print_field("dwAccess", obj.dwAccess);
            printer.print_field("name", obj.name);
        }
    );
}

template <> void print<VkImportFenceWin32HandleInfoKHR>(Printer& printer, const VkImportFenceWin32HandleInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("fence", obj.fence);
            printer.print_field("flags", obj.flags);
            printer.print_field("handleType", obj.handleType);
            printer.print_field("handle", obj.handle);
            printer.print_field("name", obj.name);
        }
    );
}

template <> void print<VkImportMemoryWin32HandleInfoKHR>(Printer& printer, const VkImportMemoryWin32HandleInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("handleType", obj.handleType);
            printer.print_field("handle", obj.handle);
            printer.print_field("name", obj.name);
        }
    );
}

template <> void print<VkImportMemoryWin32HandleInfoNV>(Printer& printer, const VkImportMemoryWin32HandleInfoNV& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("handleType", obj.handleType);
            printer.print_field("handle", obj.handle);
        }
    );
}

template <> void print<VkImportSemaphoreWin32HandleInfoKHR>(Printer& printer, const VkImportSemaphoreWin32HandleInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("semaphore", obj.semaphore);
            printer.print_field("flags", obj.flags);
            printer.print_field("handleType", obj.handleType);
            printer.print_field("handle", obj.handle);
            printer.print_field("name", obj.name);
        }
    );
}
#endif // VK_USE_PLATFORM_WIN32_KHR

////////////////////////////////////////////////////////////////////////////////
// Video encode/decode
GVK_STUB_TO_STRING_DEFINITION(VkVideoBeginCodingInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoCapabilitiesKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoCodingControlInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeCapabilitiesKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264CapabilitiesKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264DpbSlotInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264PictureInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264ProfileInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264SessionParametersAddInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH264SessionParametersCreateInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265CapabilitiesKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265DpbSlotInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265PictureInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265ProfileInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265SessionParametersAddInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeH265SessionParametersCreateInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoDecodeUsageInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeCapabilitiesKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264CapabilitiesEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264FrameSizeEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264NaluSliceInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264ProfileInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264QpEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264RateControlInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264RateControlLayerInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264SessionParametersCreateInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265CapabilitiesEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265FrameSizeEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265NaluSliceSegmentInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265ProfileInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265QpEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265RateControlInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265RateControlLayerInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265SessionParametersCreateInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeH265VclFrameInfoEXT)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeRateControlInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeRateControlLayerInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEncodeUsageInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoEndCodingInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoFormatPropertiesKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoPictureResourceInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoProfileInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoProfileListInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoReferenceSlotInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoSessionCreateInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoSessionMemoryRequirementsKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoSessionParametersCreateInfoKHR)
GVK_STUB_TO_STRING_DEFINITION(VkVideoSessionParametersUpdateInfoKHR)

////////////////////////////////////////////////////////////////////////////////
// Special case members
template <>
void print<VkAccelerationStructureBuildGeometryInfoKHR>(Printer& printer, const VkAccelerationStructureBuildGeometryInfoKHR& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
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
void print<VkAccelerationStructureTrianglesDisplacementMicromapNV>(Printer& printer, const VkAccelerationStructureTrianglesDisplacementMicromapNV& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("displacementBiasAndScaleFormat", obj.displacementBiasAndScaleFormat);
            printer.print_field("displacementVectorFormat", obj.displacementVectorFormat);
            printer.print_field("displacementBiasAndScaleBuffer", obj.displacementBiasAndScaleBuffer);
            printer.print_field("displacementBiasAndScaleStride", obj.displacementBiasAndScaleStride);
            printer.print_field("displacementVectorBuffer", obj.displacementVectorBuffer);
            printer.print_field("displacementVectorStride", obj.displacementVectorStride);
            printer.print_field("displacedMicromapPrimitiveFlags", obj.displacedMicromapPrimitiveFlags);
            printer.print_field("displacedMicromapPrimitiveFlagsStride", obj.displacedMicromapPrimitiveFlagsStride);
            printer.print_field("indexType", obj.indexType);
            printer.print_field("indexBuffer", obj.indexBuffer);
            printer.print_field("indexStride", obj.indexStride);
            printer.print_field("baseTriangle", obj.baseTriangle);
            printer.print_field("usageCountsCount", obj.usageCountsCount);
            printer.print_array("pGeometries", obj.usageCountsCount, obj.pUsageCounts);
            printer.print_array("ppGeometries", obj.usageCountsCount, obj.ppUsageCounts,
                [&](auto, auto pUsageCount)
                {
                    if (pUsageCount) {
                        print(printer, *pUsageCount);
                    } else {
                        printer.mOstrm << "null";
                    }
                }
            );
            printer.print_field("micromap", obj.micromap);
        }
    );
}

template <>
void print<VkAccelerationStructureTrianglesOpacityMicromapEXT>(Printer& printer, const VkAccelerationStructureTrianglesOpacityMicromapEXT& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("indexType", obj.indexType);
            printer.print_field("indexBuffer", obj.indexBuffer);
            printer.print_field("indexStride", obj.indexStride);
            printer.print_field("baseTriangle", obj.baseTriangle);
            printer.print_field("usageCountsCount", obj.usageCountsCount);
            printer.print_array("pGeometries", obj.usageCountsCount, obj.pUsageCounts);
            printer.print_array("ppGeometries", obj.usageCountsCount, obj.ppUsageCounts,
                [&](auto, auto pUsageCount)
                {
                    if (pUsageCount) {
                        print(printer, *pUsageCount);
                    } else {
                        printer.mOstrm << "null";
                    }
                }
            );
            printer.print_field("micromap", obj.micromap);
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
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("pVersionData", obj.pVersionData);
        }
    );
}

template <>
void print<VkMicromapBuildInfoEXT>(Printer& printer, const VkMicromapBuildInfoEXT& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("type", obj.type);
            printer.print_field("flags", obj.flags);
            printer.print_field("mode", obj.mode);
            printer.print_field("dstMicromap", obj.dstMicromap);
            printer.print_field("usageCountsCount", obj.usageCountsCount);
            printer.print_array("pGeometries", obj.usageCountsCount, obj.pUsageCounts);
            printer.print_array("ppGeometries", obj.usageCountsCount, obj.ppUsageCounts,
                [&](auto, auto pUsageCount)
                {
                    if (pUsageCount) {
                        print(printer, *pUsageCount);
                    } else {
                        printer.mOstrm << "null";
                    }
                }
            );
            printer.print_field("data", obj.data);
            printer.print_field("scratchData", obj.scratchData);
            printer.print_field("triangleArray", obj.triangleArray);
            printer.print_field("triangleArrayStride", obj.triangleArrayStride);
        }
    );
}

template <>
void print<VkMicromapVersionInfoEXT>(Printer& printer, const VkMicromapVersionInfoEXT& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("pVersionData", obj.pVersionData);
        }
    );
}

template <>
void print<VkPipelineCacheCreateInfo>(Printer& printer, const VkPipelineCacheCreateInfo& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_flags<VkPipelineCacheCreateFlagBits>("flags", obj.flags);
            printer.print_field("initialDataSize", obj.initialDataSize);
            printer.print_array("pInitialData", obj.initialDataSize, (const uint8_t*)obj.pInitialData);
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
            detail::print_pnext(printer, obj.pNext);
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
void print<VkShaderCreateInfoEXT>(Printer& printer, const VkShaderCreateInfoEXT& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("flags", obj.flags);
            printer.print_field("stage", obj.stage);
            printer.print_field("nextStage", obj.nextStage);
            printer.print_field("codeType", obj.codeType);
            printer.print_field("codeSize", obj.codeSize);
            printer.print_array("pCode", obj.codeSize, (const uint8_t*)obj.pCode);
            printer.print_field("pName", obj.pName);
            printer.print_field("setLayoutCount", obj.setLayoutCount);
            printer.print_array("pSetLayouts", obj.setLayoutCount, obj.pSetLayouts);
            printer.print_field("pushConstantRangeCount", obj.pushConstantRangeCount);
            printer.print_array("pPushConstantRanges", obj.pushConstantRangeCount, obj.pPushConstantRanges);
            printer.print_pointer("pSpecializationInfo", obj.pSpecializationInfo);
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
            detail::print_pnext(printer, obj.pNext);
            printer.print_field("flags", obj.flags);
            printer.print_field("codeSize", obj.codeSize);
            printer.print_array("pCode", obj.codeSize / sizeof(uint32_t), obj.pCode);
        }
    );
}

template <>
void print<VkSpecializationInfo>(Printer& printer, const VkSpecializationInfo& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("mapEntryCount", obj.mapEntryCount);
            printer.print_array("pMapEntries", obj.mapEntryCount, obj.pMapEntries);
            printer.print_field("dataSize", obj.dataSize);
            printer.print_array("pData", obj.dataSize, (const uint8_t*)obj.pData);
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
