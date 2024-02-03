
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

class CreatorProcessCommandBufferGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_RESTORE_POINT_GENERATED_SOURCE_PATH "/creator-process-command-buffer.cpp");
        file << std::endl;
        file << "#include \"gvk-restore-point/creator.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <fstream>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "static void enumerate_cmds(const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)" << std::endl;
        file << "{" << std::endl;
        file << "    assert(pInfo);" << std::endl;
        file << "    assert(pUserData);" << std::endl;
        file << "    auto& userData = *(CmdEnumerationUserData*)pUserData;" << std::endl;
        file << "    assert(userData.pCreateInfo);" << std::endl;
        file << "    assert(userData.commandBuffer);" << std::endl;
        file << "    ++((CmdEnumerationUserData*)pUserData)->cmdCount;" << std::endl;
        file << "    if (!userData.cmdsFile.is_open()) {" << std::endl;
        file << "        auto path = userData.pCreateInfo->path / \"VkCommandBuffer\";" << std::endl;
        file << "        if (userData.pCreateInfo->flags & (GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT | GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT)) {" << std::endl;
        file << "            std::filesystem::create_directories(path);" << std::endl;
        file << "        }" << std::endl;
        file << "        path /= to_hex_string(userData.commandBuffer);" << std::endl;
        file << "        if (userData.pCreateInfo->flags & GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT) {" << std::endl;
        file << "            userData.cmdsFile.open(path.replace_extension(\"cmds\"), std::ios::binary);" << std::endl;
        file << "        }" << std::endl;
        file << "        if (userData.pCreateInfo->flags & GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT) {" << std::endl;
        file << "            userData.jsonFile.open(path.replace_extension(\"cmds.json\"));" << std::endl;
        file << "        }" << std::endl;
        file << "    }" << std::endl;
        file << "    userData.cmdsFile.write((char*)&((const GvkCommandBaseStructure*)pInfo)->sType, sizeof(GvkCommandStructureType));" << std::endl;
        file << "    switch (((const GvkCommandBaseStructure*)pInfo)->sType) {" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.type == xml::Command::Type::Cmd || command.name == "vkBeginCommandBuffer" || command.name == "vkEndCommandBuffer") {
                std::string structureType = "GVK_COMMAND_STRUCTURE_TYPE";
                for (const auto& token : gvk::string::split_camel_case(gvk::string::strip_vk(command.name))) {
                    structureType += "_" + gvk::string::to_upper(token);
                }
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                file << "    case " << structureType << ": {" << std::endl;
                file << "        if (userData.pCreateInfo->flags & GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT) {" << std::endl;
                file << "            serialize(userData.cmdsFile, *(const GvkCommandStructure" << gvk::string::strip_vk(command.name) << "*)pInfo);" << std::endl;
                file << "        }" << std::endl;
                file << "        if (userData.pCreateInfo->flags & GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT) {" << std::endl;
                file << "            userData.jsonFile << to_string(*(const GvkCommandStructure" << gvk::string::strip_vk(command.name) << "*)pInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;" << std::endl;
                file << "        }" << std::endl;
                file << "    } break;" << std::endl;
            }
        }
        file << "    default: {" << std::endl;
        file << "        assert(false && \"Unserviced vkCmd; gvk maintenance required\");" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult Creator::process_VkCommandBuffer(GvkCommandBufferRestoreInfo& restoreInfo)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {" << std::endl;
        file << "        auto stateTrackedObject = get_default<GvkStateTrackedObject>();" << std::endl;
        file << "        stateTrackedObject.type = VK_OBJECT_TYPE_COMMAND_BUFFER;" << std::endl;
        file << "        stateTrackedObject.handle = (uint64_t)restoreInfo.handle;" << std::endl;
        file << "        stateTrackedObject.dispatchableHandle = (uint64_t)get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);" << std::endl;
        file << "        CmdEnumerationUserData userData { };" << std::endl;
        file << "        userData.pCreateInfo = &mCreateInfo;" << std::endl;
        file << "        userData.commandBuffer = restoreInfo.handle;" << std::endl;
        file << "        auto enumerateInfo = get_default<GvkStateTrackedObjectEnumerateInfo>();" << std::endl;
        file << "        enumerateInfo.pfnCallback = enumerate_cmds;" << std::endl;
        file << "        enumerateInfo.pUserData = &userData;" << std::endl;
        file << "        gvkEnumerateStateTrackedCommandBufferCmds(&stateTrackedObject, &enumerateInfo);" << std::endl;
        file << "        if (userData.cmdCount) {" << std::endl;
        file << "            if (userData.pCreateInfo->flags & GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT) {" << std::endl;
        file << "                gvk_result(userData.cmdsFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "            }" << std::endl;
        file << "            if (userData.pCreateInfo->flags & GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT) {" << std::endl;
        file << "                gvk_result(userData.jsonFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "            }" << std::endl;
        file << "        }" << std::endl;
        file << "        gvk_result(BasicCreator::process_VkCommandBuffer(restoreInfo));" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
