
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

class BasicLayerGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_LAYER_GENERATED_INCLUDE_PATH,
            GVK_LAYER_GENERATED_INCLUDE_PREFIX,
            GVK_LAYER_GENERATED_SOURCE_PATH,
            "basic-layer"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_noop_command_body(FileGenerator& file, const xml::Command& command)
    {
        file << "{" << std::endl;
        for (const auto& parameter : command.parameters) {
            if (parameter.name == "gvkResult") {
                file << "    return gvkResult;" << std::endl;
            } else {
                file << "    (void)" << parameter.name << ";" << std::endl;
            }
        }
        file << "}" << std::endl;
    }

    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::layer");
        file << std::endl;
        file << "class BasicLayer" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        file << "    virtual ~BasicLayer() = 0;" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = append_return_result_parameter(commandItr.second);
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            file << "    virtual " << command.returnType << " pre_" << command.name << "(" << get_parameter_list(command.parameters) << ");" << std::endl;
            file << "    virtual " << command.returnType << " post_" << command.name << "(" << get_parameter_list(command.parameters) << ");" << std::endl;
        }
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::layer");
        file << std::endl;
        file << "BasicLayer::~BasicLayer()" << std::endl;
        file << "{" << std::endl;
        file << "}" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = append_return_result_parameter(commandItr.second);
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            file << command.returnType << " BasicLayer::pre_" << command.name << "(" << get_parameter_list(command.parameters) << ")" << std::endl;
            generate_noop_command_body(file, command);
            file << std::endl;
            file << command.returnType << " BasicLayer::post_" << command.name << "(" << get_parameter_list(command.parameters) << ")" << std::endl;
            generate_noop_command_body(file, command);
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
