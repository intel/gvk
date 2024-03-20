
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
        "VkBufferUsageFlagBits2KHR",
        "VkFormatFeatureFlagBits2",
        "VkMemoryDecompressionMethodFlagBitsNV",
        "VkPhysicalDeviceSchedulingControlsFlagBitsARM",
        "VkPipelineCreateFlagBits2KHR",
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

std::string get_extension_vendor(const std::string& str)
{
    auto tokens = gvk::string::split_camel_case(str);
    return gvk::string::is_upper(tokens.back()) ? tokens.back() : std::string { };
}

xml::Parameter create_parameter(const std::string& type, const std::string& name)
{
    xml::Parameter parameter;
    parameter.type = type;
    parameter.unqualifiedType = type;
    parameter.name = name;
    return parameter;
}

xml::Parameter create_const_pointer_parameter(const std::string& type, const std::string& name)
{
    xml::Parameter parameter;
    parameter.type = "const " + type + "*";
    parameter.unqualifiedType = type;
    parameter.name = name;
    parameter.flags = gvk::xml::Const | gvk::xml::Pointer;
    return parameter;
}

std::pair<xml::Parameter, xml::Parameter> get_array_parameters(const std::string& countName, const std::string& arrayName, const std::string& unqualifiedType)
{
    std::pair<xml::Parameter, xml::Parameter> parameters;
    parameters.first.type = "uint32_t";
    parameters.first.unqualifiedType = "uint32_t";
    parameters.first.name = countName;
    parameters.second.type = "const " + unqualifiedType + "*";
    parameters.second.unqualifiedType = unqualifiedType;
    parameters.second.name = arrayName;
    parameters.second.length = countName;
    parameters.second.flags = gvk::xml::Dynamic | gvk::xml::Const | gvk::xml::Pointer | gvk::xml::Array;
    return parameters;
}

void add_array_members_to_structure(const std::string& unqualifiedType, const std::string& countName, const std::string& arrayName, xml::Structure& structure)
{
    auto arrayMembers = get_array_parameters(countName, arrayName, unqualifiedType);
    structure.members.push_back(arrayMembers.first);
    structure.members.push_back(arrayMembers.second);
}

std::set<std::string> get_inner_scope_compile_guards(const std::set<std::string>& outerScopeCompileGuards, std::set<std::string> innerScopeCompileGuards)
{
    for (const auto& compileGuard : outerScopeCompileGuards) {
        innerScopeCompileGuards.erase(compileGuard);
    }
    return innerScopeCompileGuards;
}

std::vector<string::Replacement> get_inner_scope_replacements(const std::vector<string::Replacement>& outerScopeReplacements, std::vector<string::Replacement> innerScopeReplacements)
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

void generate_pnext_switch(
    FileGenerator& file,
    const xml::Manifest& manifest,
    const std::string& indentation,
    const std::string& evaluation,
    const std::vector<std::string>& caseProcessor,
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
            for (const auto& line : caseProcessor) {
                file << indentation << "    " << string::replace(line, replacements) << '\n';
            }
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
