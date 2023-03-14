
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

class CreateCommandBufferRestorePointGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_RESTORE_POINT_GENERATED_SOURCE_PATH "/create-command-buffer-restore-point.cpp");
        file << std::endl;
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/restore-point-creator.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info.h\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-enumerations-to-string.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-comparison-operators.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-create-copy.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-deserialization.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-destroy-copy.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-get-stype.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-serialization.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-to-string.hpp\"" << std::endl;
        file << "#include \"gvk-structures/defaults.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <fstream>" << std::endl;
        file << "#include <utility>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "VkResult RestorePointCreator::process_VkCommandBuffer(const GvkStateTrackedObject& stateTrackedObject, GvkCommandBufferRestoreInfo& restoreInfo)" << std::endl;
        file << "{" << std::endl;
        file << "    auto path = mRestorePointInfo.path / \"VkCommandBuffer\";" << std::endl;
        file << "    std::filesystem::create_directories(path);" << std::endl;
        file << "    auto jsonPath = (path / to_hex_string(stateTrackedObject.handle)).replace_extension(\"cmds.json\");" << std::endl;
        file << "    std::ofstream jsonFile(jsonPath);" << std::endl;
        file << "    assert(jsonFile.is_open());" << std::endl;
        file << "    auto cmdsPath = (path / to_hex_string(stateTrackedObject.handle)).replace_extension(\"cmds\");" << std::endl;
        file << "    std::ofstream cmdsFile(cmdsPath, std::ios::binary);" << std::endl;
        file << "    assert(cmdsFile.is_open());" << std::endl;
        file << "    auto enumerateCmds = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)" << std::endl;
        file << "    {" << std::endl;
        file << "        assert(pInfo);" << std::endl;
        file << "        assert(pUserData);" << std::endl;
        file << "        auto& jsonFile = *((std::pair<std::ofstream*, std::ofstream*>*)pUserData)->first;" << std::endl;
        file << "        auto& cmdsFile = *((std::pair<std::ofstream*, std::ofstream*>*)pUserData)->second;" << std::endl;
        file << "        cmdsFile.write((char*)&((const GvkCommandBaseStructure*)pInfo)->sType, sizeof(GvkCommandStructureType));" << std::endl;
        file << "        switch (((const GvkCommandBaseStructure*)pInfo)->sType) {" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.type == xml::Command::Type::Cmd || command.name == "vkBeginCommandBuffer" || command.name == "vkEndCommandBuffer") {
                std::string structureType = "GVK_COMMAND_STRUCTURE_TYPE";
                for (const auto& token : gvk::string::split_camel_case(gvk::string::strip_vk(command.name))) {
                    structureType += "_" + gvk::string::to_upper(token);
                }
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                file << "        case " << structureType << ": {" << std::endl;
                file << "            jsonFile << to_string(*(const GvkCommandStructure" << gvk::string::strip_vk(command.name) << "*)pInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;" << std::endl;
                file << "            serialize(cmdsFile, *(const GvkCommandStructure" << gvk::string::strip_vk(command.name) << "*)pInfo);" << std::endl;
                file << "        } break;" << std::endl;
            }
        }
        file << "        default: {" << std::endl;
        file << "            assert(false && \"TODO : Documentation\");" << std::endl;
        file << "        } break;" << std::endl;
        file << "        }" << std::endl;
        file << "    };" << std::endl;
        file << "    std::pair<std::ofstream*, std::ofstream*> files { &jsonFile, &cmdsFile };" << std::endl;
        file << "    GvkStateTrackedObjectEnumerateInfo enumerateInfo { };" << std::endl;
        file << "    enumerateInfo.pfnCallback = enumerateCmds;" << std::endl;
        file << "    enumerateInfo.pUserData = &files;" << std::endl;
        file << "    pfnGvkEnumerateStateTrackedCommandBufferCmds(&stateTrackedObject, &enumerateInfo);" << std::endl;
        file << "    return BasicRestorePointCreator::process_VkCommandBuffer(stateTrackedObject, restoreInfo);" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
