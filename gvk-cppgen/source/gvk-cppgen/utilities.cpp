
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

#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/utilities.hpp"

#include <cassert>

namespace gvk {
namespace cppgen {

bool is_static_const_value(const std::string& apiElementName)
{
    static const std::set<std::string> sStaticConstValues {
        "VkAccessFlagBits2",
        "VkFormatFeatureFlagBits2",
        "VkMemoryDecompressionMethodFlagBitsNV",
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

        // GvkCommandStructures
        "GvkCommandStructureAllocateCommandBuffers",
        "GvkCommandStructureAllocateDescriptorSets",
        "GvkCommandStructureBuildAccelerationStructuresKHR",
        "GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR",
        "GvkCommandStructureCmdBuildAccelerationStructuresKHR",
        "GvkCommandStructureCmdSetBlendConstants",
        "GvkCommandStructureCmdSetSampleMaskEXT",
        "GvkCommandStructureCmdSetFragmentShadingRateEnumNV",
        "GvkCommandStructureCmdSetFragmentShadingRateKHR",
        "GvkCommandStructureCreateXlibSurfaceKHR",
        "GvkCommandStructureGetAccelerationStructureBuildSizesKHR",
        "GvkCommandStructureGetPhysicalDeviceXlibPresentationSupportKHR",
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

        // GvkCommandStructures
        "GvkCommandStructureGetDeviceProcAddr",
        "GvkCommandStructureGetInstanceProcAddr",
        "GvkCommandStructureGetMemoryWin32HandlePropertiesKHR",
        "GvkCommandStructureGetFenceWin32HandleKHR",
        "GvkCommandStructureGetMemoryRemoteAddressNV",
        "GvkCommandStructureGetMemoryWin32HandleKHR",
        "GvkCommandStructureGetMemoryWin32HandleNV",
        "GvkCommandStructureGetSemaphoreWin32HandleKHR",
    };
    return structure_requires_custom_implementation(apiElementName) || sStructuresRequiringCustomImplementation.count(apiElementName);
}

xml::Command append_return_result_parameter(xml::Command command)
{
    assert(!command.returnType.empty());
    if (command.returnType != "void") {
        xml::Parameter result;
        result.type = command.returnType;
        result.name = "gvkResult";
        command.parameters.push_back(result);
    }
    return command;
}

std::string get_parameter_list(const std::vector<xml::Parameter>& parameters, bool types, bool names)
{
    std::stringstream strStrm;
    int count = 0;
    for (const auto& parameter : parameters) {
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
        if (types) {
            const auto& length = parameter.length;
            if (!length.empty() && length.front() == '[' && length.back() == ']') {
                strStrm << length;
            }
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

void generate_noop_command_body(FileGenerator& file, const xml::Command& command)
{
    file << "{" << std::endl;
    for (const auto& parameter : command.parameters) {
        file << "    (void)" << parameter.name << ";" << std::endl;
    }
    if (command.returnType != "void") {
        file << "    return { };" << std::endl;
    }
    file << "}" << std::endl;
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
    if (!defaultProcessor.empty()) {
        file << indentation << "    " << defaultProcessor << '\n';
    }
    file << indentation << "}\n";
    file << indentation << "}\n";
}

void generate_object_type_switch(
    FileGenerator& file,
    const xml::Manifest& manifest,
    const std::string& indentation,
    const std::string& evaluation,
    const std::string& caseProcessor,
    const std::string& defaultProcessor
)
{
    file << indentation << "switch (" << evaluation << ") {\n";
    for (const auto& handleItr : manifest.handles) {
        const auto& handle = handleItr.second;
        if (handle.alias.empty() && !handle.vkObjectType.empty()) {
            CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
            file << indentation << "case " << handle.vkObjectType << ": {\n";
            std::vector<string::Replacement> replacements {
                { "{vkHandleType}", handle.name },
                { "{vkObjectType}", handle.vkObjectType },
                { "{gvkHandleType}", string::strip_vk(handle.name) },
            };
            file << indentation << "    " << string::replace(caseProcessor, replacements) << '\n';
            file << indentation << "} break;\n";
        }
    }
    file << indentation << "default: {\n";
    if (!defaultProcessor.empty()) {
        file << indentation << "    " << defaultProcessor << '\n';
    }
    file << indentation << "}\n";
    file << indentation << "}\n";
}

} // namespace cppgen
} // namespace gvk
