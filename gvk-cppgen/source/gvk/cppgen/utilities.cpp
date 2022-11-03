
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

#include "gvk/cppgen/compile-guard-generator.hpp"
#include "gvk/cppgen/utilities.hpp"

#include "gvk/string.hpp"

namespace gvk {
namespace cppgen {

bool is_static_const_value(const std::string& apiElementName)
{
    static const std::set<std::string> sStaticConstValues {
        "VkAccessFlagBits2",
        "VkFormatFeatureFlagBits2",
        "VkPipelineStageFlagBits2",
    };
    return sStaticConstValues.count(apiElementName);
}

bool is_strongly_typed_bitmask(const xml::Manifest& manifest, const std::string& apiElementName)
{
    auto flagBitsTypeName = string::replace(apiElementName, "Flags", "FlagBits");
    const auto& enumerationItr = manifest.enumerations.find(flagBitsTypeName);
    if (enumerationItr != manifest.enumerations.end()) {
        const auto& enumeration = enumerationItr->second;
        return enumeration.isBitmask && !enumeration.enumerators.empty() && !is_static_const_value(enumeration.name);
    }
    return false;
}

bool structure_requires_custom_implementation(const std::string& apiElementName)
{
    static const std::set<std::string> sStructuresRequiringCustomImplentation {
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

        // Video encode/decode
        "VkVideoDecodeH264DpbSlotInfoEXT",
        "VkVideoDecodeH264MvcEXT",
        "VkVideoDecodeH264PictureInfoEXT",
        "VkVideoDecodeH264SessionParametersAddInfoEXT",
        "VkVideoDecodeH265SessionParametersAddInfoEXT",
        "VkVideoDecodeH265DpbSlotInfoEXT",
        "VkVideoDecodeH265PictureInfoEXT",
        "VkVideoEncodeH264DpbSlotInfoEXT",
        "VkVideoEncodeH264NaluSliceEXT",
        "VkVideoEncodeH264ReferenceListsEXT",
        "VkVideoEncodeH264SessionParametersAddInfoEXT",
        "VkVideoEncodeH264VclFrameInfoEXT",
        "VkVideoEncodeH265DpbSlotInfoEXT",
        "VkVideoEncodeH265NaluSliceSegmentEXT",
        "VkVideoEncodeH265ReferenceListsEXT",
        "VkVideoEncodeH265SessionParametersAddInfoEXT",
        "VkVideoEncodeH265VclFrameInfoEXT",

        // Special case members
        "VkAccelerationStructureBuildGeometryInfoKHR",
        "VkAccelerationStructureVersionInfoKHR",
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
    return sStructuresRequiringCustomImplentation.count(apiElementName);
}

bool structure_requires_custom_serialization(const std::string& apiElementName)
{
    static const std::set<std::string> sStructuresRequiringCustomImplementation{
        "VkAccelerationStructureInstanceKHR",
        "VkAccelerationStructureMatrixMotionInstanceNV",
        "VkAccelerationStructureSRTMotionInstanceNV",
        "VkSurfaceFullScreenExclusiveWin32InfoEXT",
        "VkWin32SurfaceCreateInfoKHR",
    };
    return structure_requires_custom_implementation(apiElementName) || sStructuresRequiringCustomImplementation.count(apiElementName);
}

std::string get_command_args(const xml::Command& command, bool types, bool names)
{
    std::stringstream strStrm;
    int count = 0;
    for (const auto& parameter : command.parameters) {
        if (count++) {
            strStrm << ", ";
        }
        if (types) {
            strStrm << parameter.type;
        }
        if (types && names) {
            strStrm << " ";
        }
        if (names) {
            strStrm << parameter.name;
        }
    }
    return strStrm.str();
}

std::set<std::string> get_inner_scope_compile_guards(
    const std::set<std::string>& outerScopeCompileGuards,
    std::set<std::string> innerScopeCompileGuards
)
{
    for (const auto& compileGuard : outerScopeCompileGuards) {
        innerScopeCompileGuards.erase(compileGuard);
    }
    return innerScopeCompileGuards;
}

std::vector<string::Replacement> get_inner_scope_replacements(
    const std::vector<string::Replacement>& outerScopeReplacements,
    std::vector<string::Replacement> innerScopeReplacements
)
{
    std::set<std::string> innerScopeKeys;
    for (const auto& innerScopeReplacement : innerScopeReplacements) {
        innerScopeKeys.insert(innerScopeReplacement.first);
    }
    for (const auto& outerScopeReplacement : outerScopeReplacements) {
        if (!innerScopeKeys.count(outerScopeReplacement.first)) {
            innerScopeReplacements.push_back(outerScopeReplacement);
        }
    }
    return innerScopeReplacements;
}

void generate_pnext_switch(
    FileGenerator& file,
    const xml::Manifest& manifest,
    const std::string& indentation,
    const std::string& evaluation,
    const std::string& caseProcessor,
    const std::string& defaultProcessor
)
{
    file << indentation << "switch (" << evaluation << ") {\n";
    for (const auto& structureItr : manifest.structures) {
        const auto& structure = structureItr.second;
        if (structure.alias.empty() && !structure.vkStructureType.empty()) {
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << indentation << "case " << structure.vkStructureType << ": {\n";
            std::vector<string::Replacement> replacements{
                { "{structureType}", structure.name },
                { "{sType}", structure.vkStructureType },
            };
            file << indentation << "    " << string::replace(caseProcessor, replacements) << '\n';
            file << indentation << "} break;\n";
        }
    }
    file << indentation << "default: {\n";
    file << indentation << "    " << defaultProcessor << '\n';
    file << indentation << "}\n";
    file << indentation << "}\n";
}

} // namespace cppgen
} // namespace gvk
