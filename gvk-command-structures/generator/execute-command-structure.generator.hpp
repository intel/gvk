
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

#include "gvk-cppgen.hpp"

namespace gvk {
namespace cppgen {

class ExecuteCommandStructureGenerator final
{
public:
    static void generate(const xml::Manifest& manifest, const ApiElementCollectionInfo& apiElements)
    {
        ModuleGenerator module(
            GVK_COMMAND_STRUCTURES_GENERATED_INCLUDE_PATH,
            GVK_COMMAND_STRUCTURES_GENERATED_INCLUDE_PREFIX,
            GVK_COMMAND_STRUCTURES_GENERATED_SOURCE_PATH,
            "execute-command-structure"
        );
        generate_header(module.header, apiElements);
        generate_source(module.source, manifest, apiElements);
    }

private:
    static void generate_header(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
    {
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        for (const auto& include : apiElements.headerIncludes) {
            file << "#include \"" << include << "\"" << std::endl;
        }
        file << "#include \"gvk-dispatch-table.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;
        for (const auto& structure : apiElements.structures) {
            if (structure.name != "GvkCommandCollection") {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace("{returnType} execute_command_structure(const DispatchTable& dispatchTable, const {structureType}& obj);", {
                    { "{returnType}", structure.members.back().name == "result" ? structure.members.back().type : "void" },
                    { "{structureType}", structure.name },
                    }) << std::endl;
            }
        }
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest, const ApiElementCollectionInfo& apiElements)
    {
        (void)manifest;
        for (const auto& include : apiElements.sourceIncludes) {
            file << "#include \"" << include << "\"" << std::endl;
        }
        file << "#include \"gvk-command-structures/generated/command-structure-get-stype.hpp\"" << std::endl;
        file << "#include \"gvk-structures/detail/get-count.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        for (const auto& structure : apiElements.structures) {
            if (!structure.vkStructureType.empty()) {
                generate_command_structure_handler(file, structure);
            }
        }
        generate_base_structure_handler(file, apiElements.structures);
        file << std::endl;
    }

    static void generate_command_structure_handler(FileGenerator& file, const xml::Structure& structure)
    {
        file << std::endl;
        auto returnType = structure.members.back().name == "result" ? structure.members.back().type : "void";
        CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
        file << string::replace("{returnType} execute_command_structure(const DispatchTable& dispatchTable, const {structureType}& obj)", {
            { "{returnType}", returnType },
            { "{structureType}", structure.name },
        }) << std::endl;
        file << "{" << std::endl;
        std::stringstream strStrm;
        for (size_t member_i = 1; member_i < structure.members.size(); ++member_i) {
            if (structure.members[member_i].name != "result") {
                strStrm << "obj." << structure.members[member_i].name << ", ";
            }
        }
        auto commandArgumentsStr = strStrm.str();
        if (!commandArgumentsStr.empty()) {
            commandArgumentsStr.pop_back();
            commandArgumentsStr.pop_back();
        }
        file << "    ";
        if (returnType != "void") {
            file << "return ";
        }
        file << "dispatchTable." << string::replace(structure.name, "GvkCommandStructure", "gvk") << "(" << commandArgumentsStr << ");" << std::endl;
        file << "}" << std::endl;
    }

    static void generate_base_structure_handler(FileGenerator& file, const std::vector<xml::Structure>& structures)
    {
        // TODO : Handle structures with `result` parameter.
        file << std::endl;
        file << "void execute_command_structure(const DispatchTable& dispatchTable, const GvkCommandBaseStructure& obj)" << std::endl;
        file << "{" << std::endl;
        file << "    // TODO : Handle `result` parameter" << std::endl;
        file << "    switch (obj.sType) {" << std::endl;
        for (const auto& structure : structures) {
            if (!structure.vkStructureType.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << "    case get_stype<" << structure.name << ">(): {" << std::endl;
                file << "        execute_command_structure(dispatchTable, (const " << structure.name << "&)obj);" << std::endl;
                file << "    } break;" << std::endl;
            }
        }
        file << "    default: {" << std::endl;
        file << "        assert(false && \"TODO : Error handling\");" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "void execute_command_structure(const DispatchTable& dispatchTable, const GvkCommandCmdBaseStructure& obj)" << std::endl;
        file << "{" << std::endl;
        file << "    execute_command_structure(dispatchTable, (const GvkCommandBaseStructure&)obj);" << std::endl;
        file << "}" << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
