
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

#include <unordered_set>

namespace gvk {
namespace cppgen {

class UniqueHandlesLayerHooksGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_LAYER_GENERATED_INCLUDE_PATH,
            GVK_LAYER_GENERATED_INCLUDE_PREFIX,
            GVK_LAYER_GENERATED_SOURCE_PATH,
            "unique-handles-layer-hooks"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static const std::unordered_set<std::string>& get_manually_implemented_entry_points()
    {
        static const std::unordered_set<std::string> scManuallyImplementedEntryPoints{
        };
        return scManuallyImplementedEntryPoints;
    }

    static const std::unordered_set<std::string>& get_manually_implemented_object_creation_handlers()
    {
        static const std::unordered_set<std::string> scManuallyImplementedEntryPoints{
            // "vkAllocateCommandBuffers", // NOTE : Handle wrapper creation omitted for dispatchable type VkCommandBuffer
            "vkAllocateDescriptorSets",
            "vkCreatePipelineBinariesKHR",
            "vkCreateSwapchainKHR",
        };
        return scManuallyImplementedEntryPoints;
    }

    static const std::unordered_set<std::string>& get_manually_implemented_object_destruction_handlers()
    {
        static const std::unordered_set<std::string> scManuallyImplementedEntryPoints{
            // "vkDestroyCommandPool", // NOTE : Handle wrapper destruction omitted for dispatchable type VkCommandBuffer
            // "vkFreeCommandBuffers", // NOTE : Handle wrapper destruction omitted for dispatchable type VkCommandBuffer
            "vkResetDescriptorPool",
            "vkDestroyDescriptorPool",
            "vkFreeDescriptorSets",
            "vkDestroySwapchainKHR",
        };
        return scManuallyImplementedEntryPoints;
    }

    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-layer/generated/basic-api-call-handler.hpp\"" << std::endl;
        file << "#include \"gvk-dispatch-table.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <utility>" << std::endl;
        file << "#include <vector>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::layer::hooks::unique_handles");
        file << std::endl;

        // Entry point declarations
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            file << command.returnType << " g" << command.name << "(" << get_parameter_list(command.parameters) << ");" << std::endl;
        }

        // Function pointer and dispatch table accessor declarations
        file << "PFN_vkVoidFunction get(const char* pName);" << std::endl;
        file << "gvk::DispatchTable get_dispatch_table();" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-layer/registry.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <unordered_map>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::layer::hooks::unique_handles");
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            assert(!command.parameters.empty());

            // Generate command compile guards and signature
            file << std::endl;
            auto compileGuards = command.compileGuards;
            if (get_manually_implemented_entry_points().count(command.name)) {
                compileGuards.insert("GVK_MANUALLY_IMPLEMENTED");
            }
            CompileGuardGenerator compileGuardGenerator(file, compileGuards);
            file << command.returnType << " g" << command.name << "(" << get_parameter_list(command.parameters) << ")" << std::endl;
            file << "{" << std::endl;

            // Generate command structure initialization
            std::string sType = "GVK_COMMAND_STRUCTURE_TYPE";
            for (const auto& token : gvk::string::split_camel_case(gvk::string::strip_vk(command.name))) {
                sType += "_" + gvk::string::to_upper(token);
            }
            std::string commandStructureName = "GvkCommandStructure" + gvk::string::strip_vk(command.name);
            file << "    // Prepare command structure with command arguments" << std::endl;
            file << "    " << commandStructureName << " command{ };" << std::endl;
            file << "    command.sType = " << sType << ";" << std::endl;
            for (const auto& parameter : command.parameters) {
                if (parameter.flags & gvk::xml::Static && parameter.flags & gvk::xml::Array) {
                    assert(parameter.flags & gvk::xml::Const && "Encountered unexpected mutable static array; gvk maintenance required");
                    assert((parameter.flags & ~gvk::xml::Const) == (gvk::xml::Static | gvk::xml::Array) && "Encountered static array with unexpected attributes; gvk maintenance required");
                    file << "    memcpy(&command." << parameter.name << ", &" << parameter.name << ", sizeof(command." << parameter.name << "));" << std::endl;
                } else {
                    file << "    command." << parameter.name << " = " << parameter.name << ";" << std::endl;
                }
            }

            // Prepare replacements
            std::vector<string::Replacement> replacements{
                { "{commandName}", command.name },
                { "{resultAssignment}", command.returnType == "void" ? std::string() : "command.result = " },
                { "{dispatchableHandleType}", (command.parameters[0].type == "VkInstance" || command.parameters[0].type == "VkPhysicalDevice") ? "VkInstance" : "VkDevice" },
                { "{dispatchableHandle}", command.parameters[0].name },
            };

            // Generate code common to all API call handlers
            file << string::replace(
R"(
    gvk_result_scope_begin(VK_SUCCESS) {

        // Get UniqueHandlesManager
        auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
        gvk_result_assert(uniqueHandlesManager.enabled);

        // Set current command
        uniqueHandlesManager.set_current_command(command);

        // Get DispatchTable
        const auto& dispatchTableItr = uniqueHandlesManager.{dispatchableHandleType}DispatchTables.find(get_dispatch_key({dispatchableHandle}));
        gvk_result_assert(dispatchTableItr != uniqueHandlesManager.{dispatchableHandleType}DispatchTables.end());
        gvk_result_assert(dispatchTableItr->second.g{commandName});

        // Unwrap command structure handles
        gvk_result(uniqueHandlesManager.unwrap_handles(&command));

        // Execute command structure with unwrapped handles
        {resultAssignment}gvk::detail::execute_command_structure(dispatchTableItr->second, command);
)", replacements);

            // Get target parameter info
            const auto& targetParameter = command.get_target_parameter();
            const auto& targetParameterHandleItr = manifest.handles.find(targetParameter.unqualifiedType);
            bool targetParameterIsDispatchable = targetParameterHandleItr != manifest.handles.end() && targetParameterHandleItr->second.isDispatchable;

            // Generate object creation and handle out logic
            if (get_manually_implemented_object_creation_handlers().count(command.name)) {
                file << std::endl;
                file << "        // Create new handle wrapper (manually implemented; see \"gvk/gvk-layer/source/gvk-layer/unique-handles-manager.hpp\")" << std::endl;
                file << "        gvk_result(on_create_object(command));" << std::endl;
            } else if (command.type == gvk::xml::Command::Type::Create) {
                file << std::endl;
                if (targetParameterIsDispatchable) {
                    file << "        // NOTE : Handle wrapper creation omitted for dispatchable type " << targetParameter.unqualifiedType << std::endl;
                } else if (!targetParameter.length.empty()) {
                    file << "        // Create new handle wrappers" << std::endl;
                    file << "        for (uint32_t i = 0; i < " + targetParameter.length + "; ++i) {" << std::endl;
                    file << "            gvk_result(uniqueHandlesManager.on_create_object(" + targetParameter.name + " + i));" << std::endl;
                    file << "        }" << std::endl;
                } else {
                    file << "        // Create new handle wrapper" << std::endl;
                    file << "        gvk_result(uniqueHandlesManager.on_create_object(" + targetParameter.name + "));" << std::endl;
                }
            } else if (command.contains_handle_out_parameter(manifest)) {
                file << std::endl;
                file << "        // Manage handle out parameter (manually implemented; see \"gvk/gvk-layer/source/gvk-layer/unique-handles-manager.hpp\")" << std::endl;
                file << "        gvk_result(on_handle_out(command));" << std::endl;
            }

            // Generate call to rewrap_handles()
            file << std::endl;
            file << "        // Rewrap command structure handles" << std::endl;
            file << "        gvk_result(uniqueHandlesManager.rewrap_handles());" << std::endl;

            // Generate object destruction logic
            if (get_manually_implemented_object_destruction_handlers().count(command.name)) {
                file << std::endl;
                file << "        // Destroy handle wrapper (manually implemented; see \"gvk/gvk-layer/source/gvk-layer/unique-handles-manager.hpp\")" << std::endl;
                file << "        gvk_result(on_destroy_object(command));" << std::endl;
            } else if (command.type == gvk::xml::Command::Type::Destroy) {
                file << std::endl;
                if (targetParameterIsDispatchable) {
                    file << "        // NOTE : Handle wrapper destruction omitted for dispatchable type " << targetParameter.unqualifiedType << std::endl;
                } else if (!targetParameter.length.empty()) {
                    file << "        // Destroy handle wrappers" << std::endl;
                    file << "        for (uint32_t i = 0; i < " + targetParameter.length + "; ++i) {" << std::endl;
                    file << "            gvk_result(uniqueHandlesManager.on_destroy_object(" + targetParameter.name + "[i]));" << std::endl;
                    file << "        }" << std::endl;
                } else {
                    file << "        // Destroy handle wrapper" << std::endl;
                    file << "        gvk_result(uniqueHandlesManager.on_destroy_object(" + targetParameter.name + "));" << std::endl;
                }
            }

            // Clear current command
            file << std::endl;
            file << "        // Clear current command" << std::endl;
            file << "        uniqueHandlesManager.clear_current_command();" << std::endl;

            // Generate layer hook return
            file << "    } gvk_result_scope_end;" << std::endl;
            if (command.returnType != "void") {
                file << std::endl;
                file << "    // NOTE : UniqueHandleManager failures will set gvkResult to an error code, if all" << std::endl;
                file << "    //  UniqueHandleManager operations were succesful return the result of the command" << std::endl;
                file << "    //  (which itself may be an error code), otherwise return the error code recieved" << std::endl;
                file << "    //  from the UniqueHandleManager" << std::endl;
                if (command.returnType == "VkResult") {
                    file << "    return gvkResult == VK_SUCCESS ? command.result : gvkResult;" << std::endl;
                } else {
                    file << "    return gvkResult == VK_SUCCESS ? command.result : " << command.returnType << "{ };" << std::endl;
                }
            }
            file << "}" << std::endl;
        }
        file << std::endl;

        // Generate accessor for unique handles layer hooks function pointers
        file << "PFN_vkVoidFunction get(const char* pName)" << std::endl;
        file << "{" << std::endl;
        file << "    static const std::unordered_map<std::string, PFN_vkVoidFunction> scLayerHooks {" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            file << string::replace("        { \"{commandName}\", (PFN_vkVoidFunction)g{commandName} },", "{commandName}", command.name ) << std::endl;
        }
        file << "    };" << std::endl;
        file << "    auto itr = scLayerHooks.find(pName ? pName : std::string());" << std::endl;
        file << "    return itr != scLayerHooks.end() ? itr->second : nullptr;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;

        // Generate dispatch table pointing to unique handles layer hooks
        file << "gvk::DispatchTable get_dispatch_table()" << std::endl;
        file << "{" << std::endl;
        file << "    gvk::DispatchTable dispatchTable{ };" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            file << "    dispatchTable.g" << command.name << " = g" << command.name << ";" << std::endl;
        }
        file << "    return dispatchTable;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }

    static std::unordered_map<std::string, std::vector<std::string>>& get_out_functions()
    {
        static std::unordered_map<std::string, std::vector<std::string>> sOutFunctions;
        return sOutFunctions;
    }
};

} // namespace cppgen
} // namespace gvk
