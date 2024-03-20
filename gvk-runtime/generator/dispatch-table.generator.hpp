
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

class DispatchTableGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_RUNTIME_GENERATED_INCLUDE_PATH,
            GVK_RUNTIME_GENERATED_INCLUDE_PREFIX,
            GVK_RUNTIME_GENERATED_SOURCE_PATH,
            "dispatch-table"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "struct DispatchTable" << std::endl;
        file << "{" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            CompileGuardGenerator compileGuards(file, command.compileGuards);
            file << string::replace("    PFN_{commandName} g{commandName} { nullptr };", "{commandName}", command.name) << std::endl;
        }
        file << std::endl;
        file << "    static void load_global_entry_points(DispatchTable* pDispatchTable);" << std::endl;
        file << "    static void load_instance_entry_points(VkInstance vkInstance, DispatchTable* pDispatchTable);" << std::endl;
        file << "    static void load_device_entry_points(VkDevice vkDevice, DispatchTable* pDispatchTable);" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-runtime.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        generate_load_entry_points_function(
            file, manifest,
            [](const xml::Command& command)
            {
                return
                    command.name == "vkCreateInstance" ||
                    command.name == "vkEnumerateInstanceVersion" ||
                    command.name == "vkEnumerateInstanceExtensionProperties" ||
                    command.name == "vkEnumerateInstanceLayerProperties";
            },
            "load_global_entry_points(DispatchTable* pDispatchTable)",
            "VK_NULL_HANDLE",
            "vkGetInstanceProcAddr",
            "load_vkGetInstanceProcAddr()"
        );
        file << std::endl;
        generate_load_entry_points_function(
            file, manifest,
            "load_instance_entry_points(VkInstance vkInstance, DispatchTable* pDispatchTable)",
            "vkInstance",
            "vkGetInstanceProcAddr"
        );
        file << std::endl;
        generate_load_entry_points_function(
            file, manifest,
            [](const xml::Command& command)
            {
                return
                    !command.parameters.empty() && (
                        command.parameters[0].type == "VkDevice" ||
                        command.parameters[0].type == "VkQueue" ||
                        command.parameters[0].type == "VkCommandBuffer"
                    );
            },
            "load_device_entry_points(VkDevice vkDevice, DispatchTable* pDispatchTable)",
            "vkDevice",
            "vkGetDeviceProcAddr"
        );
        file << std::endl;
    }

    template <typename PredicateType>
    static void generate_load_entry_points_function(
        FileGenerator& file,
        const xml::Manifest& manifest,
        PredicateType predicate,
        const std::string& signature,
        const std::string& vkHandleArgument,
        const std::string& getProcAddr,
        const std::string& setGetProcAddr = { }
    )
    {
        file << "void DispatchTable::" << signature << std::endl;
        file << "{" << std::endl;
        file << "    assert(pDispatchTable);" << std::endl;
        if (!setGetProcAddr.empty()) {
            file << "    pDispatchTable->g" << getProcAddr << " = " << setGetProcAddr << ";" << std::endl;
        } else {
            file << "    assert(pDispatchTable->g" << getProcAddr << ");" << std::endl;
        }
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.name != getProcAddr && predicate(command)) {
                CompileGuardGenerator compileGuards(file, command.compileGuards);
                file << string::replace("    pDispatchTable->g{commandName} = (PFN_{commandName})pDispatchTable->g{getProcAddr}({vkHandleArgument}, \"{commandName}\");", {
                    { "{vkHandleArgument}", vkHandleArgument },
                    { "{getProcAddr}", getProcAddr },
                    { "{commandName}", command.name },
                }) << std::endl;
            }
        }
        file << "}" << std::endl;
    }

    static void generate_load_entry_points_function(
        FileGenerator& file,
        const xml::Manifest& manifest,
        const std::string& signature,
        const std::string& vkHandleArgument,
        const std::string& getProcAddr
    )
    {
        generate_load_entry_points_function(file, manifest, [](const xml::Command&) { return true; }, signature, vkHandleArgument, getProcAddr);
    }
};

} // namespace cppgen
} // namespace gvk
