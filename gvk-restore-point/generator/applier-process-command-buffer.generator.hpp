
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

class ApplierProcessCommandBufferGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_RESTORE_POINT_GENERATED_SOURCE_PATH "/applier-process-command-buffer.cpp");
        file << std::endl;
        file << "#include \"gvk-restore-point/applier.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures/generated/execute-command-structure.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/update-structure-handles.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <fstream>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "VkResult Applier::process_VkCommandBuffer_cmds(const GvkRestorePointObject& restorePointObject)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {" << std::endl;
        file << "        auto cmdsPath = (mApplyInfo.path / \"VkCommandBuffer\" / to_hex_string(restorePointObject.handle)).replace_extension(\".cmds\");" << std::endl;
        file << "        if (std::filesystem::exists(cmdsPath)) {" << std::endl;
        file << "            std::ifstream cmdsFile(cmdsPath, std::ios::binary);" << std::endl;
        file << "            gvk_result(cmdsFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "            Auto<GvkCommandBufferRestoreInfo> restoreInfo;" << std::endl;
        file << "            gvk_result(read_object_restore_info(mApplyInfo.path, \"VkCommandBuffer\", to_hex_string(restorePointObject.handle), restoreInfo));" << std::endl;
        file << "            auto device = get_dependency<VkDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);" << std::endl;
        file << "            while (!cmdsFile.eof()) {" << std::endl;
        file << "                GvkCommandStructureType commandStructureType = GVK_COMMAND_STRUCTURE_TYPE_UNDEFINED;" << std::endl;
        file << "                cmdsFile.read((char*)&commandStructureType, sizeof(GvkCommandStructureType));" << std::endl;
        file << "                switch (commandStructureType) {" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.type == xml::Command::Type::Cmd || command.name == "vkBeginCommandBuffer" || command.name == "vkEndCommandBuffer") {
                std::string structureType = "GVK_COMMAND_STRUCTURE_TYPE";
                for (const auto& token : string::split_camel_case(string::strip_vk(command.name))) {
                    structureType += "_" + string::to_upper(token);
                }
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                file << "                case " << structureType << ": {" << std::endl;
                file << "                    Auto<GvkCommandStructure" << string::strip_vk(command.name) << "> commandStructure;" << std::endl;
                file << "                    deserialize(cmdsFile, nullptr, commandStructure);" << std::endl;
                file << "                    auto commandBuffer = (VkCommandBuffer)get_restored_object(restorePointObject).handle;" << std::endl;
                file << "                    const_cast<GvkCommandStructure" << string::strip_vk(command.name) << "&>(*commandStructure).commandBuffer = VK_NULL_HANDLE;" << std::endl;
                file << "                    update_command_structure_handles(mRestorePointObjects, (uint64_t)device, *commandStructure);" << std::endl;
                file << "                    const_cast<GvkCommandStructure" << string::strip_vk(command.name) << "&>(*commandStructure).commandBuffer = commandBuffer;" << std::endl;
                file << "                    detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure);" << std::endl;
                file << "                } break;" << std::endl;
            }
        }
        file << "                default: {" << std::endl;
        file << "                    gvk_result(cmdsFile.eof() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "                } break;" << std::endl;
        file << "                }" << std::endl;
        file << "            }" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
