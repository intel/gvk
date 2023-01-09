
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

#include "gvk-cppgen/include.hpp"
#include "gvk-string/include.hpp"

#include <vector>

namespace gvk {
namespace cppgen {

class CommandStructuresGenerator final
{
public:
    static std::vector<xml::Structure> get_command_structures(const xml::Manifest& manifest)
    {
        std::vector<xml::Structure> structures;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            xml::Structure structure;
            structure.name = "GvkCommandStructure" + string::strip_vk(command.name);
            structure.vendor = command.vendor;
            structure.extension = command.extension;
            structure.compileGuards = command.compileGuards;
            structure.vkStructureType = "GVK_COMMAND_STRUCTURE_TYPE";
            for (const auto& token : string::split_camel_case(string::strip_vk(command.name))) {
                structure.vkStructureType += "_" + string::to_upper(token);
            }
            xml::Parameter sTypeMember;
            sTypeMember.type = "GvkCommandStructureType";
            sTypeMember.name = "sType";
            structure.members.push_back(sTypeMember);
            structure.members.insert(structure.members.end(), command.parameters.begin(), command.parameters.end());
            if (command.returnType != "void") {
                xml::Parameter resultMember;
                resultMember.type = command.returnType;
                resultMember.name = "result";
                structure.members.push_back(resultMember);
            }
            structures.push_back(structure);
        }
        return structures;
    }

    static xml::Enumeration get_structure_type_enumeration(const xml::Manifest& manifest)
    {
        std::set<std::string> sTypeValues;
        xml::Enumeration enumeration;
        enumeration.name = "GvkCommandStructureType";

        xml::Enumerator enumerator;
        enumerator.name = "GVK_COMMAND_STRUCTURE_TYPE_UNDEFINED";
        enumerator.value = "0";
        sTypeValues.insert(enumerator.value);
        enumeration.enumerators.insert(enumerator);

        for (const auto& commandStructure : get_command_structures(manifest)) {
            enumerator.name = commandStructure.vkStructureType;
            enumerator.value = gvk::to_hex_string(string::hash(enumerator.name));
            enumerator.compileGuards = commandStructure.compileGuards;
            if (!sTypeValues.insert(enumerator.value).second) {
                enumerator.value += " GVK_COMMAND_STRUCTURE_TYPE collision!";
            }
            enumeration.enumerators.insert(enumerator);
        }
        return enumeration;
    }

    static void generate(const xml::Manifest& manifest)
    {
        auto commandStructures = get_command_structures(manifest);
        auto commandStructureTypeEnumeration = get_structure_type_enumeration(manifest);
        FileGenerator file(GVK_STRUCTURES_GENERATED_INCLUDE_PATH "/command-structures.h");
        file << std::endl;
        HeaderGuardGenerator headerGuardGenerator(file, "gvk_structures_command_structures_h");
        file << std::endl;
        file << "#include \"vulkan/vulkan.h\"" << std::endl;
        file << std::endl;
        file << "typedef enum " << commandStructureTypeEnumeration.name << " {" << std::endl;
        for (const auto& enumerator : commandStructureTypeEnumeration.enumerators) {
            CompileGuardGenerator compileGuardGenerator(file, enumerator.compileGuards);
            file << "    " << enumerator.name << " = " << enumerator.value << "," << std::endl;
        }
        file << "    GVK_COMMAND_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF," << std::endl;
        file << "} GvkCommandStructureType;" << std::endl;
        file << std::endl;
        file << "typedef struct GvkCommandBaseStructure {" << std::endl;
        file << "    GvkCommandStructureType sType;" << std::endl;
        file << "} GvkCommandBaseStructure;" << std::endl;
        for (const auto& commandStructure : commandStructures) {
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, commandStructure.compileGuards);
            file << "typedef struct " << commandStructure.name << " {" << std::endl;
            for (const auto& member : commandStructure.members) {
                if (member.flags & xml::Static && member.flags & xml::Array) {
                    file << "    " << member.unqualifiedType << " " << member.name << member.length << ";" << std::endl;
                } else {
                    file << "    " << member.type << " " << member.name << ";" << std::endl;
                }
            }
            file << "} " << commandStructure.name << ";" << std::endl;
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
