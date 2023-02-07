
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

#include "state-tracked-handles.generator.hpp"
#include "gvk-cppgen.hpp"

#include <cassert>
#include <unordered_set>

namespace gvk {
namespace cppgen {

class BasicStateTrackerGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_STATE_TRACKER_GENERATED_INCLUDE_PATH,
            GVK_STATE_TRACKER_GENERATED_INCLUDE_PREFIX,
            GVK_STATE_TRACKER_GENERATED_SOURCE_PATH,
            "basic-state-tracker"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static bool command_requires_custom_implementation(const std::string& name)
    {
        static const std::set<std::string> sCommandsRequiringCustomImplementation {
            "vkAllocateCommandBuffers",
            "vkFreeCommandBuffers",
            "vkAllocateDescriptorSets",
            "vkFreeDescriptorSets",
            "vkCreateDisplayModeKHR",
            "vkCreateSharedSwapchainsKHR",
            "vkCreateSwapchainKHR",
            "vkCreateVideoSessionParametersKHR",
            "vkDestroySwapchainKHR",
            "vkDestroyVideoSessionParametersKHR",
            "vkCreateComputePipelines",
            "vkCreateGraphicsPipelines",
            "vkCreateRayTracingPipelinesKHR",
            "vkCreateRayTracingPipelinesNV",
        };
        return sCommandsRequiringCustomImplementation.count(name);
    }

    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-layer/generated/basic-layer.hpp\"" << std::endl;
        file << "#include \"gvk-state-tracker/generated/state-tracked-handles.hpp\"" << std::endl;
        file << "#include \"gvk-state-tracker/object-tracker.hpp\"" << std::endl;
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::state_tracker");
        file << std::endl;
        file << "class BasicStateTracker" << std::endl;
        file << "    : public layer::BasicLayer" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        for (const auto& commandItr : manifest.commands) {
            if (command_requires_custom_implementation(commandItr.second.name)) {
                continue;
            }
            const auto& command = append_return_result_parameter(commandItr.second);
            if (command.type == xml::Command::Type::Create || command.type == xml::Command::Type::Destroy || command.type == xml::Command::Type::Cmd) {
                CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
                file << "    virtual " << command.returnType << " post_" << command.name << "(" << get_parameter_list(command.parameters) << ") override;" << std::endl;
            }
        }
        file << "protected:" << std::endl;
        file << "    ObjectTracker<Instance> mInstanceTracker;" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-state-tracker/generated/state-tracked-handles.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::state_tracker");
        for (const auto& commandItr : manifest.commands) {
            if (command_requires_custom_implementation(commandItr.second.name)) {
                continue;
            }
            const auto& command = commandItr.second;
            auto layerHookCommand = append_return_result_parameter(command);
            if (!command.target.empty()) {
                const auto targetHandleItr = manifest.handles.find(command.target);
                assert(targetHandleItr != manifest.handles.end());
                auto targetHandle = targetHandleItr->second;

                std::vector<string::Replacement> replacements {
                    { "{returnType}", command.returnType },
                    { "{commandName}", command.name },
                    { "{commandParameters}", get_parameter_list(command.parameters) },
                    { "{commandArguments}", get_parameter_list(command.parameters, false) },
                    { "{layerHookParameters}", get_parameter_list(layerHookCommand.parameters) },
                    { "{gvkHandleType}", string::strip_vk(targetHandle.name) },
                    { "{vkHandleType}", targetHandle.name },
                    { "{vkHandleArgument}", command.get_target_parameter().name },
                    { "{parentVkHandleArgument}", command.parameters.front().name },
                    { "{parentGvkHandleType}", string::strip_vk(command.parameters.front().type) },
                };

                std::string objectTrackerExpression;
                if (targetHandle.name == "VkInstance") {
                    objectTrackerExpression = "mInstanceTracker";
                } else if (targetHandle.name == "VkDevice") {
                    objectTrackerExpression = string::replace("controlBlock.mPhysicalDevice.mReference.get_obj().m{gvkHandleType}Tracker", replacements);
                } else {
                    objectTrackerExpression = string::replace("controlBlock.m{parentGvkHandleType}.mReference.get_obj().m{gvkHandleType}Tracker", replacements);
                }
                replacements.push_back({ "{objectTrackerExpression}", objectTrackerExpression });

                auto dispatchableHandleIdExpression = "*{vkHandleArgument}";
                auto nonDispatchableHandleIdExpression = get_handle_id_type(manifest, targetHandle) + "({parentVkHandleArgument}, *{vkHandleArgument})";
                auto handleIdExpression = string::replace(targetHandle.isDispatchable ? dispatchableHandleIdExpression : nonDispatchableHandleIdExpression, replacements);
                replacements.push_back({ "{handleIdExpression}", handleIdExpression });

                auto dispatchableHandleLookupExpression = "{gvkHandleType}({vkHandleArgument})";
                auto nonDispatchableHandleLookupExpression = "{gvkHandleType}({ {parentVkHandleArgument}, {vkHandleArgument} })";
                auto handleLookupExpression = string::replace(targetHandle.isDispatchable ? dispatchableHandleLookupExpression : nonDispatchableHandleLookupExpression, replacements);
                replacements.push_back({ "{handleLookupExpression}", handleLookupExpression });

                switch (command.type) {
                case xml::Command::Type::Create:
                {
                    generate_create_definition(file, manifest, command, replacements);
                } break;
                case xml::Command::Type::Destroy:
                {
                    generate_destroy_definition(file, manifest, command, replacements);
                } break;
                case xml::Command::Type::Cmd:
                {
                    generate_cmd_definition(file, command, replacements);
                } break;
                default:
                {
                } break;
                }
            }
        }
        file << std::endl;
    }

    static void generate_create_definition(FileGenerator& file, const xml::Manifest& manifest, const xml::Command& command, std::vector<string::Replacement> replacements)
    {
        file << std::endl;
        const auto& targetHandleItr = manifest.handles.find(command.target);
        assert(targetHandleItr != manifest.handles.end());
        StateTrackedHandleGenerator handleGenerator(manifest, targetHandleItr->second);
        CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
        std::stringstream strStrm;
        strStrm << "{returnType} BasicStateTracker::post_{commandName}({layerHookParameters})" << std::endl;
        strStrm << "{" << std::endl;
        strStrm << "    if (gvkResult == VK_SUCCESS) {" << std::endl;
        strStrm << "        {gvkHandleType} handle;" << std::endl;
        strStrm << "        handle.mReference.reset(gvk::newref, {handleIdExpression});" << std::endl;
        strStrm << "        auto& controlBlock = handle.mReference.get_obj();" << std::endl;
        strStrm << "        controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;" << std::endl;
        strStrm << "        controlBlock.m{vkHandleType} = *{vkHandleArgument};" << std::endl;
        for (const auto& memberInfo : handleGenerator.get_members()) {
            auto assignmentExpression = handleGenerator.get_member_assignment_expression(manifest, command, memberInfo);
            if (!assignmentExpression.empty()) {
                strStrm << "        controlBlock." << memberInfo.storageName << " = " << assignmentExpression << ";" << std::endl;
            }
        }
        strStrm << "        {objectTrackerExpression}.insert(handle);" << std::endl;
        strStrm << "    }" << std::endl;
        strStrm << "    return gvkResult;" << std::endl;
        strStrm << "}" << std::endl;
        file << string::replace(strStrm.str(), replacements);
    }

    static void generate_destroy_definition(FileGenerator& file, const xml::Manifest& manifest, const xml::Command& command, std::vector<string::Replacement> replacements)
    {
        file << std::endl;
        const auto& targetHandleItr = manifest.handles.find(command.target);
        assert(targetHandleItr != manifest.handles.end());
        StateTrackedHandleGenerator handleGenerator(manifest, targetHandleItr->second);
        CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
        std::stringstream strStrm;
        strStrm << "{returnType} BasicStateTracker::post_{commandName}({layerHookParameters})" << std::endl;
        strStrm << "{" << std::endl;
        strStrm << "    (void)pAllocator;" << std::endl;
        strStrm << "    auto handle = {handleLookupExpression};" << std::endl;
        strStrm << "    assert(handle);" << std::endl;
        strStrm << "    auto& controlBlock = handle.mReference.get_obj();" << std::endl;
        strStrm << "    controlBlock.mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;" << std::endl;
        strStrm << "    controlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;" << std::endl;
        for (const auto& memberInfo : handleGenerator.get_members()) {
            if (string::contains(memberInfo.storageType, "ObjectTracker")) {
                CompileGuardGenerator memberInfoCompileGuardGenerator(strStrm, get_inner_scope_compile_guards(command.compileGuards, memberInfo.compileGuards));
                strStrm << "    controlBlock." << memberInfo.storageName << ".clear();" << std::endl;
            }
        }
        strStrm << "    {objectTrackerExpression}.erase(handle);" << std::endl;
        strStrm << "}" << std::endl;
        file << string::replace(strStrm.str(), replacements);
    }

    static void generate_cmd_definition(FileGenerator& file, const xml::Command& command, std::vector<string::Replacement> replacements)
    {
        file << std::endl;
        CompileGuardGenerator compileGuardGenerator(file, command.compileGuards);
        std::stringstream strStrm;
        strStrm << "{returnType} BasicStateTracker::post_{commandName}({layerHookParameters})" << std::endl;
        strStrm << "{" << std::endl;
        strStrm << "    auto handle = {handleLookupExpression};" << std::endl;
        strStrm << "    assert(handle);" << std::endl;
        strStrm << "    handle.mReference.get_obj().mCmdTracker.record_{commandName}({commandArguments});" << std::endl;
        if (command.returnType != "void") {
            strStrm << "    return gvkResult;" << std::endl;
        }
        strStrm << "}" << std::endl;
        file << string::replace(strStrm.str(), replacements);
    }
};

} // namespace cppgen
} // namespace gvk
