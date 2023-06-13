
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
#include "enumerate-pnext-handles.generator.hpp"
#include "get-object-type.generator.hpp"
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
        gvk::cppgen::EnumeratePNextHandlesGenerator::generate(manifest);
        gvk::cppgen::GetObjectTypeGenerator::generate(manifest);
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
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                apiElements.structures.push_back(structureItr.second);
                if (gvk::string::contains(structure.name, "VkVideo")) {
                    apiElements.manuallyImplemented.insert(structure.name);
                }
            }
        }

        // Linux
        apiElements.manuallyImplemented.insert("VkXlibSurfaceCreateInfoKHR");
        // Win32
        apiElements.manuallyImplemented.insert("VkExportFenceWin32HandleInfoKHR");
        apiElements.manuallyImplemented.insert("VkExportMemoryWin32HandleInfoKHR");
        apiElements.manuallyImplemented.insert("VkExportMemoryWin32HandleInfoNV");
        apiElements.manuallyImplemented.insert("VkExportSemaphoreWin32HandleInfoKHR");
        apiElements.manuallyImplemented.insert("VkImportFenceWin32HandleInfoKHR");
        apiElements.manuallyImplemented.insert("VkImportMemoryWin32HandleInfoKHR");
        apiElements.manuallyImplemented.insert("VkImportMemoryWin32HandleInfoNV");
        apiElements.manuallyImplemented.insert("VkImportSemaphoreWin32HandleInfoKHR");
        // Special case members
        apiElements.manuallyImplemented.insert("VkAccelerationStructureBuildGeometryInfoKHR");
        apiElements.manuallyImplemented.insert("VkAccelerationStructureTrianglesDisplacementMicromapNV");
        apiElements.manuallyImplemented.insert("VkAccelerationStructureTrianglesOpacityMicromapEXT");
        apiElements.manuallyImplemented.insert("VkAccelerationStructureVersionInfoKHR");
        apiElements.manuallyImplemented.insert("VkMicromapBuildInfoEXT");
        apiElements.manuallyImplemented.insert("VkMicromapVersionInfoEXT");
        apiElements.manuallyImplemented.insert("VkPipelineCacheCreateInfo");
        apiElements.manuallyImplemented.insert("VkPipelineMultisampleStateCreateInfo");
        apiElements.manuallyImplemented.insert("VkShaderCreateInfoEXT");
        apiElements.manuallyImplemented.insert("VkShaderModuleCreateInfo");
        apiElements.manuallyImplemented.insert("VkSpecializationInfo");
        apiElements.manuallyImplemented.insert("VkTransformMatrixKHR");
        // Unions
        apiElements.manuallyImplemented.insert("VkAccelerationStructureGeometryDataKHR");
        apiElements.manuallyImplemented.insert("VkAccelerationStructureMotionInstanceDataNV");
        apiElements.manuallyImplemented.insert("VkClearColorValue");
        apiElements.manuallyImplemented.insert("VkClearValue");
        apiElements.manuallyImplemented.insert("VkDeviceOrHostAddressConstKHR");
        apiElements.manuallyImplemented.insert("VkDeviceOrHostAddressKHR");
        apiElements.manuallyImplemented.insert("VkPerformanceCounterResultKHR");
        apiElements.manuallyImplemented.insert("VkPerformanceValueDataINTEL");
        apiElements.manuallyImplemented.insert("VkPipelineExecutableStatisticValueKHR");

        gvk::cppgen::EnumerationToStringGenerator::generate(apiElements);
        gvk::cppgen::StructureComparisonOperatorsGenerator::generate(apiElements);
        gvk::cppgen::StructureCreateCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDestroyCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureEnumerateHandlesGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureGetSTypeGenerator::generate(apiElements);
        gvk::cppgen::StructureMakeTupleGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureToStringGenerator::generate(manifest, apiElements);

        // Manually implemented serialization
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
