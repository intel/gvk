
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

class BasicPipelineExplorerGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PATH,
            GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PREFIX,
            GVK_PIPELINE_EXPLORER_GENERATED_SOURCE_PATH,
            "basic-pipeline-explorer"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-command-structures/generated/basic-command-recorder.hpp\"" << std::endl;
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << "#include \"gvk-layer.hpp\"" << std::endl;
        file << "#include <fstream>" << std::endl;
        file << "#include <mutex>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::pipeline_explorer");
        file << std::endl;
        file << "class BasicPipelineExplorer" << std::endl;
        file << "    : public layer::BasicApiCallHandler" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        file << "    BasicPipelineExplorer() = default;" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            if (!gvk::string::contains(command.name, "INTEL")) {
                file << "    virtual " << command.returnType << " execute_" << command.name << "(" << get_parameter_list(command.parameters) << ") override;" << std::endl;
            }
        }
        file << "protected:" << std::endl;
        file << "    BasicCommandRecorder mCommandRecorder;" << std::endl;
        file << "    bool TODO_shouldBeControlledByRequestInfo_getApiCalls { };" << std::endl;
        file << "private:" << std::endl;
        file << "#if 0" << std::endl;
        file << "    std::ofstream mOutputFile{ \"basic-pipeline-explorer-calls.txt\" };" << std::endl;
        file << "    std::mutex mOutputFileMutex;" << std::endl;
        file << "#endif" << std::endl;
        file << "    BasicPipelineExplorer(const BasicPipelineExplorer&) = delete;" << std::endl;
        file << "    BasicPipelineExplorer& operator=(const BasicPipelineExplorer&) = delete;" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-pipeline-explorer/backend/handle-info.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::pipeline_explorer");
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
            if (!gvk::string::contains(command.name, "INTEL")) {
                file << command.returnType << " BasicPipelineExplorer::execute_" << command.name << "(" << get_parameter_list(command.parameters) << ")" << std::endl;
                file << "{" << std::endl;

                file << "    #if 0" << std::endl;
                file << "    {" << std::endl;
                file << "        std::lock_guard<std::mutex> lock(mOutputFileMutex);" << std::endl;
                file << "        mOutputFile << std::endl << \"Thread , Frame : \" << std::endl << \"" << command.name << "\" << std::endl;" << std::endl;
                file << "    }" << std::endl;
                file << "    #endif" << std::endl;

                if (command.type == xml::Command::Type::Cmd) {
                    file << "    CommandBufferInfo commandBufferInfo(commandBuffer);" << std::endl;
                    file << "    if (commandBufferInfo) {" << std::endl;
                    file << "        commandBufferInfo->cmdTracker.record_" << command.name << "(" << get_parameter_list(command.parameters, false) << ");" << std::endl;
                    file << "    } else {" << std::endl;
                    file << "        // TODO : Error message to GUI" << std::endl;
                    file << "    }" << std::endl;
                }
                file << "    if (TODO_shouldBeControlledByRequestInfo_getApiCalls) {" << std::endl;
                file << "        mCommandRecorder.record_" << command.name << "(" << get_parameter_list(command.parameters, false) << ");" << std::endl;
                file << "    }" << std::endl;
                file << "    return BasicApiCallHandler::execute_" << command.name << "(" << get_parameter_list(command.parameters, false) << ");" << std::endl;
                file << "}" << std::endl;
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
