
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
#include "gvk-string.hpp"
#include "gvk-xml.hpp"

#include <set>
#include <string>
#include <vector>

std::vector<gvk::xml::Structure> get_command_structures(const gvk::xml::Manifest& manifest)
{
    std::vector<gvk::xml::Structure> structures;

    gvk::xml::Parameter sTypeMember;
    sTypeMember.type = "GvkCommandStructureType";
    sTypeMember.name = "sType";

    gvk::xml::Structure gvkCommandBaseStructure;
    gvkCommandBaseStructure.name = "GvkCommandBaseStructure";
    gvkCommandBaseStructure.members.push_back(sTypeMember);
    structures.push_back(gvkCommandBaseStructure);

    for (const auto& commandItr : manifest.commands) {
        const auto& command = commandItr.second;
        gvk::xml::Structure structure;
        structure.name = "GvkCommandStructure" + gvk::string::strip_vk(command.name);
        structure.vendor = command.vendor;
        structure.extension = command.extension;
        structure.compileGuards = command.compileGuards;
        structure.vkStructureType = "GVK_COMMAND_STRUCTURE_TYPE";
        for (const auto& token : gvk::string::split_camel_case(gvk::string::strip_vk(command.name))) {
            structure.vkStructureType += "_" + gvk::string::to_upper(token);
        }

        structure.members.push_back(sTypeMember);
        structure.members.insert(structure.members.end(), command.parameters.begin(), command.parameters.end());
        if (command.returnType != "void") {
            gvk::xml::Parameter resultMember;
            resultMember.type = command.returnType;
            resultMember.name = "result";
            structure.members.push_back(resultMember);
        }
        structures.push_back(structure);
    }
    return structures;
}

gvk::xml::Enumeration get_command_structure_type_enumeration(const gvk::xml::Manifest& manifest)
{
    std::set<std::string> sTypeValues;
    gvk::xml::Enumeration enumeration;
    enumeration.name = "GvkCommandStructureType";

    gvk::xml::Enumerator enumerator;
    enumerator.name = "GVK_COMMAND_STRUCTURE_TYPE_UNDEFINED";
    enumerator.value = "0";
    sTypeValues.insert(enumerator.value);
    enumeration.enumerators.insert(enumerator);

    for (const auto& commandStructure : get_command_structures(manifest)) {
        if (!commandStructure.vkStructureType.empty()) {
            enumerator.name = commandStructure.vkStructureType;
            enumerator.value = gvk::to_hex_string(gvk::string::hash(enumerator.name));
            enumerator.compileGuards = commandStructure.compileGuards;
            if (!sTypeValues.insert(enumerator.value).second) {
                enumerator.value += " GVK_COMMAND_STRUCTURE_TYPE collision!";
            }
            enumeration.enumerators.insert(enumerator);
        }
    }
    return enumeration;
}

int main(int, const char*[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        gvk::cppgen::ApiElementCollectionInfo apiElements { };
        apiElements.name = "command";
        apiElements.headerGuard = "gvk_command_structures_h";
        apiElements.includePath = GVK_COMMAND_STRUCTURES_GENERATED_INCLUDE_PATH;
        apiElements.includePrefix = GVK_COMMAND_STRUCTURES_GENERATED_INCLUDE_PREFIX;
        apiElements.sourcePath = GVK_COMMAND_STRUCTURES_GENERATED_SOURCE_PATH;
        apiElements.enumerations.push_back(get_command_structure_type_enumeration(manifest));
        apiElements.structures = get_command_structures(manifest);
        apiElements.headerIncludes = {
            GVK_COMMAND_STRUCTURES_GENERATED_INCLUDE_PREFIX "command.h",
        };
        apiElements.sourceIncludes = {
            "gvk-structures.hpp",
        };
        apiElements.manuallyImplemented = {
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
        gvk::cppgen::ApiElementCollectionDeclarationGenerator::generate(apiElements);
        gvk::cppgen::EnumerationToStringGenerator::generate(apiElements);
        gvk::cppgen::StructureComparisonOperatorsGenerator::generate(apiElements);
        gvk::cppgen::StructureCreateCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDestroyCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureGetSTypeGenerator::generate(apiElements);
        gvk::cppgen::StructureMakeTupleGenerator::generate(manifest, apiElements, "gvk-command-structures/detail/make-tuple-manual.hpp");
        gvk::cppgen::StructureToStringGeneratorEx::generate(manifest, apiElements);

        apiElements.manuallyImplemented.insert("GvkCommandStructureGetDeviceProcAddr");
        apiElements.manuallyImplemented.insert("GvkCommandStructureGetInstanceProcAddr");
        apiElements.manuallyImplemented.insert("GvkCommandStructureGetMemoryWin32HandlePropertiesKHR");
        apiElements.manuallyImplemented.insert("GvkCommandStructureGetFenceWin32HandleKHR");
        apiElements.manuallyImplemented.insert("GvkCommandStructureGetMemoryRemoteAddressNV");
        apiElements.manuallyImplemented.insert("GvkCommandStructureGetMemoryWin32HandleKHR");
        apiElements.manuallyImplemented.insert("GvkCommandStructureGetMemoryWin32HandleNV");
        apiElements.manuallyImplemented.insert("GvkCommandStructureGetSemaphoreWin32HandleKHR");
        gvk::cppgen::StructureCerealizationGenerator::generate(manifest, apiElements, "gvk-command-structures/detail/cerealization-manual.hpp");
        gvk::cppgen::StructureDecerealizationGenerator::generate(manifest, apiElements, "gvk-command-structures/detail/cerealization-manual.hpp");
        gvk::cppgen::StructureDeserializationGenerator::generate(apiElements);
        gvk::cppgen::StructureSerializationGenerator::generate(apiElements);
    }
    return 0;
}
