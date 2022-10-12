
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
        file << "    static DispatchTable& get_global_dispatch_table();" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "#ifndef VK_NO_PROTOTYPES" << std::endl;
        generate_load_entry_points_function(
            file, manifest,
            [](const xml::Command& command) { return command.extension.empty(); },
            "load_static_entry_points(DispatchTable* pDispatchTable)",
R"(        if (!pDispatchTable->g{commandName}) {
            pDispatchTable->g{commandName} = {commandName};
        })"
        );
        file << "#endif // VK_NO_PROTOTYPES" << std::endl;
        file << std::endl;
        generate_load_entry_points_function(
            file, manifest,
            "load_instance_entry_points(VkInstance vkInstance, DispatchTable* pDispatchTable)",
R"(        if (!pDispatchTable->g{commandName}) {
            pDispatchTable->g{commandName} = (PFN_{commandName})pDispatchTable->gvkGetInstanceProcAddr(vkInstance, "{commandName}");
        })"
        );
        file << std::endl;
        generate_load_entry_points_function(
            file, manifest,
            "load_device_entry_points(VkDevice vkDevice, DispatchTable* pDispatchTable)",
R"(        if (!pDispatchTable->g{commandName}) {
            pDispatchTable->g{commandName} = (PFN_{commandName})pDispatchTable->gvkGetDeviceProcAddr(vkDevice, "{commandName}");
        })"
        );
        file << "DispatchTable& DispatchTable::get_global_dispatch_table()" << std::endl;
        file << "{" << std::endl;
        file << "    static DispatchTable sDispatchTable;" << std::endl;
        file << "    return sDispatchTable;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
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
