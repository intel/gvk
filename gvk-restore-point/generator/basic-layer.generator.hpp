
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

namespace gvk {
namespace cppgen {

class BasicLayerGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PATH,
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX,
            GVK_RESTORE_POINT_GENERATED_SOURCE_PATH,
            "basic-layer"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static const std::set<std::string>& get_pure_virtual_commands()
    {
        static std::set<std::string> sPureVirtualCommands{
            "vkAllocateCommandBuffers",
            "vkAllocateDescriptorSets",
            "vkCreateComputePipelines",
            "vkCreateExecutionGraphPipelinesAMDX",
            "vkCreateGraphicsPipelines",
            "vkCreateRayTracingPipelinesKHR",
            "vkCreateRayTracingPipelinesNV",
            "vkCreateShadersEXT",
            "vkCreateSharedSwapchainsKHR",
            "vkDestroyCommandPool",
            "vkDestroyDescriptorPool",
            "vkFreeCommandBuffers",
            "vkFreeDescriptorSets",
        };
        return sPureVirtualCommands;
    }

    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << "#include \"gvk-layer/generated/basic-layer.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/utilities.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <vector>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "class BasicLayer" << std::endl;
        file << "    : public layer::BasicLayer" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = append_return_result_parameter(commandItr.second);
            if (command.type == xml::Command::Type::Create || command.type == xml::Command::Type::Destroy) {
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                auto pureVirtual = get_pure_virtual_commands().count(command.name);
                file << "    virtual " << command.returnType << " pre_" << command.name << "(" << get_parameter_list(command.parameters) << ") override" << (pureVirtual ? " = 0;" : ";") << std::endl;
                file << "    virtual " << command.returnType << " post_" << command.name << "(" << get_parameter_list(command.parameters) << ") override" << (pureVirtual ? " = 0;" : ";") << std::endl;
            }
        }
        file << "protected:" << std::endl;
        file << "    static std::set<GvkRestorePoint>& get_restore_points();" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        for (const auto& commandItr : manifest.commands) {
            const auto& command = append_return_result_parameter(commandItr.second);
            if (command.type == xml::Command::Type::Create || command.type == xml::Command::Type::Destroy) {
                const auto& targetParameter = command.get_target_parameter();
                const auto& targetItr = manifest.handles.find(targetParameter.unqualifiedType);
                assert(targetItr != manifest.handles.end());
                const auto& target = targetItr->second;
                file << std::endl;
                auto compileGuards = command.compileGuards;
                if (get_pure_virtual_commands().count(command.name)) {
                    compileGuards.insert(command.name + "_PURE_VIRTUAL");
                }
                CompileGuardGenerator compileGuardGenerator(file, compileGuards);
                file << command.returnType << " BasicLayer::pre_" << command.name << "(" << get_parameter_list(command.parameters) << ")" << std::endl;
                file << "{" << std::endl;
                for (const auto& parameter : command.parameters) {
                    file << "    (void)" << parameter.name << ";" << std::endl;
                }
                if (command.returnType != "void") {
                    file << "    return gvkResult;" << std::endl;
                }
                file << "}" << std::endl;
                file << std::endl;
                file << command.returnType << " BasicLayer::post_" << command.name << "(" << get_parameter_list(command.parameters) << ")" << std::endl;
                file << "{" << std::endl;
                for (const auto& parameter : command.parameters) {
                    file << "    (void)" << parameter.name << ";" << std::endl;
                }
                if (command.type == xml::Command::Type::Create) {
                    file << "    for (auto gvkRestorePoint : get_restore_points()) {" << std::endl;
                    file << "        assert(gvkRestorePoint);" << std::endl;
                    file << "        auto restorePointObject = get_default<GvkStateTrackedObject>();" << std::endl;
                    file << "        restorePointObject.type = " << target.vkObjectType << ";" << std::endl;
                    file << "        restorePointObject.handle = (uint64_t)*" << targetParameter.name << ";" << std::endl;
                    if (target.name == "VkInstance" ||
                        target.name == "VkPhysicalDevice" ||
                        target.name == "VkDevice" ||
                        target.name == "VkQueue" ||
                        target.name == "VkCommandBuffer") {
                        file << "        restorePointObject.dispatchableHandle = restorePointObject.handle;" << std::endl;
                    } else {
                        file << "        restorePointObject.dispatchableHandle = (uint64_t)" << command.parameters[0].name << ";" << std::endl;
                    }
                    if (targetParameter.flags & xml::Array) {
                        file << "        static_assert(false, \"Manual implementation required\");" << std::endl;
                    } else {
                        file << "        gvkRestorePoint->createdObjects.insert(restorePointObject);" << std::endl;
                    }
                    file << "    }" << std::endl;
                } else if (command.type == xml::Command::Type::Destroy) {
                    file << "    for (auto gvkRestorePoint : get_restore_points()) {" << std::endl;
                    file << "        assert(gvkRestorePoint);" << std::endl;
                    file << "        auto restorePointObject = get_default<GvkStateTrackedObject>();" << std::endl;
                    file << "        restorePointObject.type = " << target.vkObjectType << ";" << std::endl;
                    file << "        restorePointObject.handle = (uint64_t)" << targetParameter.name << ";" << std::endl;
                    if (target.name == "VkInstance" ||
                        target.name == "VkPhysicalDevice" ||
                        target.name == "VkDevice" ||
                        target.name == "VkQueue" ||
                        target.name == "VkCommandBuffer") {
                        file << "        restorePointObject.dispatchableHandle = restorePointObject.handle;" << std::endl;
                    } else {
                        file << "        restorePointObject.dispatchableHandle = (uint64_t)" << command.parameters[0].name << ";" << std::endl;
                    }
                    if (targetParameter.flags & xml::Array) {
                        file << "        static_assert(false, \"Manual implementation required\");" << std::endl;
                    } else {
                        file << "        // TODO : Fire callback here to notify upper layers" << std::endl;
                        file << "        // TODO : Wrap this fucntionality into GvkRestorePoint" << std::endl;
                        file << "        gvkRestorePoint->objectMap.register_object_destruction(restorePointObject);" << std::endl;
                        file << "        gvkRestorePoint->createdObjects.erase(restorePointObject);" << std::endl;
                    }
                    file << "    }" << std::endl;
                }
                if (command.returnType != "void") {
                    file << "    return gvkResult;" << std::endl;
                }
                file << "}" << std::endl;
            }
        }
        file << std::endl;
        file << "std::set<GvkRestorePoint>& BasicLayer::get_restore_points()" << std::endl;
        file << "{" << std::endl;
        file << "    static std::set<GvkRestorePoint> sRestorePoints;" << std::endl;
        file << "    return sRestorePoints;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
