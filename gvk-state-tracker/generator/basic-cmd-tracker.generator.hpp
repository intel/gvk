
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

class BasicCmdTrackerGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_STATE_TRACKER_GENERATED_INCLUDE_PATH,
            GVK_STATE_TRACKER_GENERATED_INCLUDE_PREFIX,
            GVK_STATE_TRACKER_GENERATED_SOURCE_PATH,
            "basic-cmd-tracker"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-state-tracker/image-layout-tracker.hpp\"" << std::endl;
        file << "#include \"gvk-structures/auto.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <unordered_map>" << std::endl;
        file << "#include <vector>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::state_tracker");
        file << std::endl;
        file << "class BasicCmdTracker" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        file << "    BasicCmdTracker() = default;" << std::endl;
        file << "    virtual ~BasicCmdTracker() = 0;" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.type == xml::Command::Type::Cmd) {
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                file << "    virtual void record_" << command.name << "(" << get_parameter_list(command.parameters) << ");" << std::endl;
            }
        }
        file << "    virtual void reset();" << std::endl;
        file << std::endl;
        file << "protected:" << std::endl;
        file << "    std::unordered_map<VkImage, ImageLayoutTracker> mImageLayoutTrackers;" << std::endl;
        file << "    Auto<GvkCommandStructureCmdBeginRenderPass> mBeginRenderPass;" << std::endl;
        file << "    Auto<GvkCommandStructureCmdBeginRenderPass2> mBeginRenderPass2;" << std::endl;
        file << "    std::vector<const GvkCommandBaseStructure*> mCmds;" << std::endl;
        file << "    BasicCmdTracker(const BasicCmdTracker&) = delete;" << std::endl;
        file << "    BasicCmdTracker& operator=(const BasicCmdTracker&) = delete;" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-structures/copy.hpp\"" << std::endl;
        file << "#include \"gvk-structures/defaults.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::state_tracker");
        file << std::endl;
        file << "BasicCmdTracker::~BasicCmdTracker()" << std::endl;
        file << "{" << std::endl;
        file << "    reset();" << std::endl;
        file << "}" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.type == xml::Command::Type::Cmd) {
                file << std::endl;
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                file << "void BasicCmdTracker::record_" << command.name << "(" << get_parameter_list(command.parameters) << ")" << std::endl;
                file << "{" << std::endl;
                file << "    auto cmd = get_default<GvkCommandStructure" << string::strip_vk(command.name) << ">();" << std::endl;
                for (const auto& parameter : command.parameters) {
                    if (parameter.flags & xml::Static && parameter.flags & xml::Array) {
                        file << "    for (uint32_t i = 0; i < " << string::remove(string::remove(parameter.length, "["), "]") << "; ++i) {" << std::endl;
                        file << "        cmd." << parameter.name << "[i] = " << parameter.name << "[i];" << std::endl;
                        file << "    }" << std::endl;
                    } else {
                        file << "    cmd." << parameter.name << " = " << parameter.name << ";" << std::endl;
                    }
                }
                file << "    mCmds.push_back((const GvkCommandBaseStructure*)detail::create_dynamic_array_copy(1, &cmd, nullptr));" << std::endl;
                file << "}" << std::endl;
            }
        }
        file << "void BasicCmdTracker::reset()" << std::endl;
        file << "{" << std::endl;
        file << "    mImageLayoutTrackers.clear();" << std::endl;
        file << "    mBeginRenderPass.reset();" << std::endl;
        file << "    mBeginRenderPass2.reset();" << std::endl;
        file << "    for (auto pCmd : mCmds) {" << std::endl;
        file << "        assert(pCmd);" << std::endl;
        file << "        switch (pCmd->sType) {" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            const auto& command = commandItr.second;
            if (command.type == xml::Command::Type::Cmd) {
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                std::string sType = "GVK_COMMAND_STRUCTURE_TYPE";
                for (const auto& token : string::split_camel_case(string::strip_vk(command.name))) {
                    sType += "_" + string::to_upper(token);
                }
                file << "        case " << sType << ": {" << std::endl;
                file << "            detail::destroy_dynamic_array_copy(1, (const GvkCommandStructure" << string::strip_vk(command.name) << "*)pCmd, nullptr);" << std::endl;
                file << "        } break;" << std::endl;
            }
        }
        file << "        default:" << std::endl;
        file << "        {" << std::endl;
        file << "            assert(false && \"Unsupported GvkCommandStructureType\");" << std::endl;
        file << "        } break;" << std::endl;
        file << "        }" << std::endl;
        file << "        mCmds.clear();" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
