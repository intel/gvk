
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

#include <cassert>

namespace gvk {
namespace cppgen {

class LayerHooksGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_LAYER_GENERATED_INCLUDE_PATH,
            GVK_LAYER_GENERATED_INCLUDE_PREFIX,
            GVK_LAYER_GENERATED_SOURCE_PATH,
            "layer-hooks"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::layer::hooks");
        file << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            file << command.returnType << " g" << command.name << "(" << get_parameter_list(command.parameters) << ");" << std::endl;
        }
        file << "PFN_vkVoidFunction get(const char* pName);" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-layer/generated/basic-layer.hpp\"" << std::endl;
        file << "#include \"gvk-layer/registry.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <unordered_map>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::layer::hooks");
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            assert(!command.parameters.empty());
            std::vector<string::Replacement> replacements{
                { "{commandName}", command.name },
                { "{vkCommandArgs}", get_parameter_list(command.parameters, false, true) },
                { "{gvkCommandArgs}", get_parameter_list(append_return_result_parameter(command).parameters, false, true) },
                { "{resultAssignment}", command.returnType == "void" ? std::string() : "gvkResult = " },
                { "{dispatchableHandleType}", (command.parameters[0].type == "VkInstance" || command.parameters[0].type == "VkPhysicalDevice") ? "VkInstance" : "VkDevice" },
                { "{dispatchableHandle}", command.parameters[0].name },
            };
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            file << command.returnType << " g" << command.name << "(" << get_parameter_list(command.parameters) << ")" << std::endl;
            file << "{" << std::endl;
            file << (command.returnType == "void" ? std::string() : "    " + command.returnType + " gvkResult { };\n");
            file << string::replace(
R"(    auto& layers = Registry::get().layers;
    for (auto layerItr = layers.begin(); layerItr != layers.end(); ++layerItr) {
        assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
        {resultAssignment}(*layerItr)->pre_{commandName}({gvkCommandArgs});
    }
    const auto& dispatchTableItr = Registry::get().{dispatchableHandleType}DispatchTables.find(get_dispatch_key({dispatchableHandle}));
    assert(dispatchTableItr != Registry::get().{dispatchableHandleType}DispatchTables.end());
    if (dispatchTableItr->second.g{commandName}) {
        {resultAssignment}dispatchTableItr->second.g{commandName}({vkCommandArgs});
    }
    for (auto layerItr = layers.rbegin(); layerItr != layers.rend(); ++layerItr) {
        assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
        {resultAssignment}(*layerItr)->post_{commandName}({gvkCommandArgs});
    }
)", replacements);
            file << (command.returnType == "void" ? std::string() : "    return gvkResult;\n");
            file << "}" << std::endl;
        }
        file << std::endl;
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
    }
};

} // namespace cppgen
} // namespace gvk
