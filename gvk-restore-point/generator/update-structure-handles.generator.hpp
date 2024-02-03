
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
#include "gvk-xml.hpp"

#include <unordered_set>

namespace gvk {
namespace cppgen {

class UpdateStructureHandlesGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PATH,
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX,
            GVK_RESTORE_POINT_GENERATED_SOURCE_PATH,
            "update-structure-handles"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << "#include \"gvk-restore-info.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <map>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            auto commandStructureName = "GvkCommandStructure" + string::strip_vk(command.name);
            file << "void update_command_structure_handles(const std::map<GvkRestorePointObject, GvkRestorePointObject>& restorePointObjects, const " << commandStructureName << "& commandStructure);" << std::endl;
            file << "void update_command_structure_handles(const std::map<GvkRestorePointObject, GvkRestorePointObject>& restorePointObjects, uint64_t parentHandle, const " << commandStructureName << "& commandStructure);" << std::endl;
        }
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        std::unordered_set<std::string> manuallyImplemented = {
            "vkBeginCommandBuffer",
        };
        file << "#include \"gvk-command-structures/generated/command-structure-enumerate-handles.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            file << std::endl;
            if (manuallyImplemented.count(command.name)) {
                file << "#if 0 // NOTE : Manually implemented\n";
            }
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            auto commandStructureName = "GvkCommandStructure" + string::strip_vk(command.name);
            file << "void update_command_structure_handles(const std::map<GvkRestorePointObject, GvkRestorePointObject>& restorePointObjects, const " << commandStructureName << "& commandStructure)" << std::endl;
            file << "{" << std::endl;
            const auto& dispatchableHandleItr = manifest.handles.find(command.parameters[0].unqualifiedType);
            if (dispatchableHandleItr != manifest.handles.end()) {
                const auto& dispatchableHandle = dispatchableHandleItr->second;
                file << "    GvkRestorePointObject dispatchableRestorePointObject{ };" << std::endl;
                file << "    dispatchableRestorePointObject.type = " << dispatchableHandle.vkObjectType << ";" << std::endl;
                file << "    dispatchableRestorePointObject.handle = (uint64_t)commandStructure." << command.parameters[0].name << ";" << std::endl;
                file << "    dispatchableRestorePointObject.dispatchableHandle = (uint64_t)commandStructure." << command.parameters[0].name << ";" << std::endl;
                file << "    detail::enumerate_structure_handles(" << std::endl;
                file << "        commandStructure," << std::endl;
                file << "        [&](VkObjectType objectType, const uint64_t& handle)" << std::endl;
                file << "        {" << std::endl;
                file << "            if (handle) {" << std::endl;
                file << "                GvkRestorePointObject restorePointObject{ };" << std::endl;
                file << "                restorePointObject.type = objectType;" << std::endl;
                file << "                restorePointObject.handle = handle;" << std::endl;
                file << "                restorePointObject.dispatchableHandle = dispatchableRestorePointObject.handle;" << std::endl;
                file << "                auto itr = restorePointObjects.find(restorePointObject);" << std::endl;
                file << "                assert(itr != restorePointObjects.end());" << std::endl;
                file << "                const_cast<uint64_t&>(handle) = itr->second.handle;" << std::endl;
                file << "            }" << std::endl;
                file << "        }" << std::endl;
                file << "    );" << std::endl;
            } else {
                file << "    (void)restorePointObjects;" << std::endl;
                file << "    (void)commandStructure;" << std::endl;
            }
            file << "}" << std::endl;
            file << std::endl;
            file << "void update_command_structure_handles(const std::map<GvkRestorePointObject, GvkRestorePointObject>& restorePointObjects, uint64_t parentHandle, const " << commandStructureName << "& commandStructure)" << std::endl;
            file << "{" << std::endl;
            if (dispatchableHandleItr != manifest.handles.end()) {
                file << "    detail::enumerate_structure_handles(" << std::endl;
                file << "        commandStructure," << std::endl;
                file << "        [&](VkObjectType objectType, const uint64_t& handle)" << std::endl;
                file << "        {" << std::endl;
                file << "            if (handle) {" << std::endl;
                file << "                GvkRestorePointObject restorePointObject{ };" << std::endl;
                file << "                restorePointObject.type = objectType;" << std::endl;
                file << "                restorePointObject.handle = handle;" << std::endl;
                file << "                restorePointObject.dispatchableHandle = parentHandle;" << std::endl;
                file << "                auto itr = restorePointObjects.find(restorePointObject);" << std::endl;
                file << "                assert(itr != restorePointObjects.end());" << std::endl;
                file << "                const_cast<uint64_t&>(handle) = itr->second.handle;" << std::endl;
                file << "            }" << std::endl;
                file << "        }" << std::endl;
                file << "    );" << std::endl;
            } else {
                file << "    (void)restorePointObjects;" << std::endl;
                file << "    (void)parentHandle;" << std::endl;
                file << "    (void)commandStructure;" << std::endl;
            }
            file << "}" << std::endl;
            if (manuallyImplemented.count(command.name)) {
                file << "#endif\n";
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
