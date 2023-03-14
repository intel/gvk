
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

class ApplyCommandBufferRestorePointGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_RESTORE_POINT_GENERATED_SOURCE_PATH "/apply-command-buffer-restore-point.cpp");
        file << std::endl;
        file << "#include \"gvk-restore-point/generated/basic-restore-point-applier.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures/generated/command-structure-enumerate-handles.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures/generated/execute-command-structure.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <fstream>" << std::endl;
        file << "#include <utility>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;
        file << "VkResult BasicRestorePointApplier::restore_command_buffer_cmds(const GvkStateTrackedObject& stateTrackedObject)" << std::endl;
        file << "{" << std::endl;
        file << "    auto path = (mApplyInfo.path / \"VkCommandBuffer\" / to_hex_string(stateTrackedObject.handle)).replace_extension(\".cmds\");" << std::endl;
        file << "    std::ifstream file(path, std::ios::binary);" << std::endl;
        file << "    assert(file.is_open());" << std::endl;
        file << "    while (!file.eof()) {" << std::endl;
        file << "        GvkCommandStructureType commandStructureType = GVK_COMMAND_STRUCTURE_TYPE_UNDEFINED;" << std::endl;
        file << "        file.read((char*)&commandStructureType, sizeof(GvkCommandStructureType));" << std::endl;
        file << "        switch (commandStructureType) {" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.type == xml::Command::Type::Cmd || command.name == "vkBeginCommandBuffer" || command.name == "vkEndCommandBuffer") {
                std::string structureType = "GVK_COMMAND_STRUCTURE_TYPE";
                for (const auto& token : gvk::string::split_camel_case(gvk::string::strip_vk(command.name))) {
                    structureType += "_" + gvk::string::to_upper(token);
                }
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                file << "        case " << structureType << ": {" << std::endl;
                file << "            Auto<GvkCommandStructure" << string::strip_vk(command.name) << "> commandStructure;" << std::endl;
                file << "            deserialize(file, nullptr, commandStructure);" << std::endl;
                file << "            update_cmd_handles(&(const GvkCommandStructure" << string::strip_vk(command.name) << "&)commandStructure);" << std::endl;
                file << "            execute_command_structure(mDispatchTable, commandStructure);" << std::endl;
                file << "        } break;" << std::endl;
            }
        }
        file << "        default: {" << std::endl;
        file << "            assert(file.eof() && \"TODO : Documentation\");" << std::endl;
        file << "        } break;" << std::endl;
        file << "        }" << std::endl;
        file << "    }" << std::endl;
        file << "    return { };" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
