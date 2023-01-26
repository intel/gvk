
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

#include "gvk-cppgen.hpp"
#include "gvk-xml.hpp"
#include "cerealize-pnext.generator.hpp"
#include "create-pnext-copy.generator.hpp"
#include "decerealize-pnext.generator.hpp"
#include "destroy-pnext-copy.generator.hpp"
#include "handle-to-string.generator.hpp"
#include "pnext-to-string.generator.hpp"
#include "pnext-tuple-element-wrapper.generator.hpp"

int main(int, const char*[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);

        gvk::cppgen::CerealizePNextGenerator::generate(manifest);
        gvk::cppgen::CreatePNextCopyGenerator::generate(manifest);
        gvk::cppgen::DecerealizePNextGenerator::generate(manifest);
        gvk::cppgen::DestroyPNextCopyGenerator::generate(manifest);
        gvk::cppgen::HandleToStringGenerator::generate(manifest);
        gvk::cppgen::PNextToStringGenerator::generate(manifest);
        gvk::cppgen::PNextTupleElementWrapperGenerator::generate(manifest);

        gvk::cppgen::ApiElementCollectionInfo apiElements { };
        apiElements.name = "core";
        apiElements.includePath = GVK_STRUCTURES_GENERATED_INCLUDE_PATH;
        apiElements.includePrefix = GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX;
        apiElements.sourcePath = GVK_STRUCTURES_GENERATED_SOURCE_PATH;
        for (const auto& enumerationItr : manifest.enumerations) {
            if (enumerationItr.second.alias.empty() && !gvk::cppgen::is_static_const_value(enumerationItr.first)) {
                apiElements.enumerations.push_back(enumerationItr.second);
            }
        }
        for (const auto& structureItr : manifest.structures) {
            if (structureItr.second.alias.empty()) {
                apiElements.structures.push_back(structureItr.second);
            }
        }
        apiElements.manuallyImplemented = {
            // Linux
            "VkXlibSurfaceCreateInfoKHR",
            // Win32
            "VkExportFenceWin32HandleInfoKHR",
            "VkExportMemoryWin32HandleInfoKHR",
            "VkExportMemoryWin32HandleInfoNV",
            "VkExportSemaphoreWin32HandleInfoKHR",
            "VkImportFenceWin32HandleInfoKHR",
            "VkImportMemoryWin32HandleInfoKHR",
            "VkImportMemoryWin32HandleInfoNV",
            "VkImportSemaphoreWin32HandleInfoKHR",
            // Decode H264
            "VkVideoDecodeH264ProfileInfoEXT",
            "VkVideoDecodeH264CapabilitiesEXT",
            "VkVideoDecodeH264SessionParametersAddInfoEXT",
            "VkVideoDecodeH264SessionParametersCreateInfoEXT",
            "VkVideoDecodeH264PictureInfoEXT",
            "VkVideoDecodeH264DpbSlotInfoEXT",
            // Decode H265
            "VkVideoDecodeH265ProfileInfoEXT",
            "VkVideoDecodeH265CapabilitiesEXT",
            "VkVideoDecodeH265SessionParametersAddInfoEXT",
            "VkVideoDecodeH265SessionParametersCreateInfoEXT",
            "VkVideoDecodeH265PictureInfoEXT",
            "VkVideoDecodeH265DpbSlotInfoEXT",
            // Encode H264
            "VkVideoEncodeH264CapabilitiesEXT",
            "VkVideoEncodeH264SessionParametersAddInfoEXT",
            "VkVideoEncodeH264SessionParametersCreateInfoEXT",
            "VkVideoEncodeH264DpbSlotInfoEXT",
            "VkVideoEncodeH264ReferenceListsInfoEXT",
            "VkVideoEncodeH264NaluSliceInfoEXT",
            "VkVideoEncodeH264VclFrameInfoEXT",
            "VkVideoEncodeH264EmitPictureParametersInfoEXT",
            "VkVideoEncodeH264ProfileInfoEXT",
            "VkVideoEncodeH264RateControlInfoEXT",
            "VkVideoEncodeH264QpEXT",
            "VkVideoEncodeH264FrameSizeEXT",
            "VkVideoEncodeH264RateControlLayerInfoEXT",
            // Encode H265
            "VkVideoEncodeH265CapabilitiesEXT",
            "VkVideoEncodeH265SessionParametersAddInfoEXT",
            "VkVideoEncodeH265SessionParametersCreateInfoEXT",
            "VkVideoEncodeH265DpbSlotInfoEXT",
            "VkVideoEncodeH265ReferenceListsInfoEXT",
            "VkVideoEncodeH265NaluSliceSegmentInfoEXT",
            "VkVideoEncodeH265VclFrameInfoEXT",
            "VkVideoEncodeH265EmitPictureParametersInfoEXT",
            "VkVideoEncodeH265ProfileInfoEXT",
            "VkVideoEncodeH265RateControlInfoEXT",
            "VkVideoEncodeH265QpEXT",
            "VkVideoEncodeH265FrameSizeEXT",
            "VkVideoEncodeH265RateControlLayerInfoEXT",
            // Special case members
            "VkAccelerationStructureBuildGeometryInfoKHR",
            "VkAccelerationStructureTrianglesOpacityMicromapEXT",
            "VkAccelerationStructureVersionInfoKHR",
            "VkMicromapBuildInfoEXT",
            "VkMicromapVersionInfoEXT",
            "VkPipelineMultisampleStateCreateInfo",
            "VkShaderModuleCreateInfo",
            "VkTransformMatrixKHR",
            // Unions
            "VkAccelerationStructureGeometryDataKHR",
            "VkAccelerationStructureMotionInstanceDataNV",
            "VkClearColorValue",
            "VkClearValue",
            "VkDeviceOrHostAddressConstKHR",
            "VkDeviceOrHostAddressKHR",
            "VkPerformanceCounterResultKHR",
            "VkPerformanceValueDataINTEL",
            "VkPipelineExecutableStatisticValueKHR",
        };
        gvk::cppgen::EnumerationToStringGenerator::generate(apiElements);
        gvk::cppgen::StructureComparisonOperatorsGenerator::generate(apiElements);
        gvk::cppgen::StructureCreateCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDestroyCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureGetSTypeGenerator::generate(apiElements);
        gvk::cppgen::StructureMakeTupleGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureToStringGeneratorEx::generate(manifest, apiElements);

        apiElements.manuallyImplemented.insert("VkAccelerationStructureInstanceKHR");
        apiElements.manuallyImplemented.insert("VkAccelerationStructureMatrixMotionInstanceNV");
        apiElements.manuallyImplemented.insert("VkAccelerationStructureSRTMotionInstanceNV");
        apiElements.manuallyImplemented.insert("VkSurfaceFullScreenExclusiveWin32InfoEXT");
        apiElements.manuallyImplemented.insert("VkWin32SurfaceCreateInfoKHR");
        gvk::cppgen::StructureCerealizationGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDecerealizationGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDeserializationGenerator::generate(apiElements);
        gvk::cppgen::StructureSerializationGenerator::generate(apiElements);
    }
    return 0;
}
