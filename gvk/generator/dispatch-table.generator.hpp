
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

#include "gvk/xml/manifest.hpp"
#include "cppgen-utilities.hpp"

namespace gvk {
namespace cppgen {

class DispatchTableGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        gvk::cppgen::Module module("dispatch-table");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "struct DispatchTable" << std::endl;
        file << "{" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.alias.empty()) {
                CompileGuardGenerator compileGuards(file, command.compileGuards);
                file << string::replace("    PFN_{commandName} g{commandName} { nullptr };", "{commandName}", command.name) << std::endl;
            }
        }
        file << std::endl;
        file << "#ifndef VK_NO_PROTOTYPES" << std::endl;
        file << "    static void load_static_entry_points(DispatchTable* pDispatchTable);" << std::endl;
        file << "#endif // VK_NO_PROTOTYPES" << std::endl;
        file << "    static void load_instance_entry_points(VkInstance vkInstance, DispatchTable* pDispatchTable);" << std::endl;
        file << "    static void load_device_entry_points(VkDevice vkDevkce, DispatchTable* pDispatchTable);" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
        file << "extern DispatchTable gDispatchTable;" << std::endl;
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "DispatchTable gDispatchTable;" << std::endl;
        file << std::endl;
        generate_load_entry_points_function(
            file, manifest,
            [](const xml::Command& command) { return command.extension.empty(); },
            "load_static_entry_points(DispatchTable* pDispatchTable)",
R"(        if (!pDispatchTable->g{commandName}) {
            pDispatchTable->g{commandName} = {commandName};
        })"
        );
        generate_load_entry_points_function(
            file, manifest,
            "load_instance_entry_points(VkInstance vkInstance, DispatchTable* pDispatchTable)",
R"(        if (!pDispatchTable->g{commandName}) {
            pDispatchTable->g{commandName} = (PFN_{commandName})pDispatchTable->gvkGetInstanceProcAddr(vkInstance, "{commandName}");
        })"
        );
        generate_load_entry_points_function(
            file, manifest,
            "load_device_entry_points(VkDevice vkDevice, DispatchTable* pDispatchTable)",
R"(        if (!pDispatchTable->g{commandName}) {
            pDispatchTable->g{commandName} = (PFN_{commandName})pDispatchTable->gvkGetDeviceProcAddr(vkDevice, "{commandName}");
        })"
        );
    }

    template <typename PredicateType>
    static void generate_load_entry_points_function(
        File& file,
        const xml::Manifest& manifest,
        PredicateType predicate,
        const std::string& signature,
        const std::string& source
    )
    {
        file << "void DispatchTable::" << signature << std::endl;
        file << "{" << std::endl;
        file << "    if (pDispatchTable) {" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.alias.empty() && predicate(command)) {
                CompileGuardGenerator compileGuards(file, command.compileGuards);
                file << string::replace(source, "{commandName}", command.name) << std::endl;
            }
        }
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }

    static void generate_load_entry_points_function(
        File& file,
        const xml::Manifest& manifest,
        const std::string& signature,
        const std::string& source
    )
    {
        generate_load_entry_points_function(file, manifest, [](const xml::Command&) { return true; }, signature, source);
    }
};

} // namespace cppgen
} // namespace gvk
