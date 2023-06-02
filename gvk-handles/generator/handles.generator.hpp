
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

#include <algorithm>
#include <cassert>
#include <set>
#include <sstream>

namespace gvk {
namespace cppgen {

class HandleGenerator final
    : public BasicHandleGenerator
{
public:
    HandleGenerator(const xml::Manifest& manifest, const xml::Handle& handle)
        : BasicHandleGenerator(manifest, handle)
    {
        if (handle.name == "VkInstance") {
            add_manually_implemented_ctor("VkResult create_unmanaged(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, const DispatchTable* pDispatchTable, VkInstance vkInstance, Instance* pGvkInstance)");
            add_member(MemberInfo("std::vector<PhysicalDevice>", "mPhysicalDevices", "const std::vector<PhysicalDevice>&"));
            add_member(MemberInfo("bool", "mUnmanaged"));
            add_manually_implemented_dtor();
        }
        if (handle.name == "VkPhysicalDevice") {
            add_member(MemberInfo("VkInstance", "mVkInstance", "Instance"));
        }
        if (handle.name == "VkDevice") {
            add_manually_implemented_ctor("VkResult create_unmanaged(const PhysicalDevice& physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, const DispatchTable* pDispatchTable, VkDevice vkDevice, Device *pGvkDevice)");
            add_member(MemberInfo("Instance", "mInstance", "Instance"));
            add_member(MemberInfo("std::vector<QueueFamily>", "mQueueFamilies", "const std::vector<QueueFamily>&"));
            add_member(MemberInfo("VmaAllocator", "mVmaAllocator", "VmaAllocator"));
            add_member(MemberInfo("bool", "mUnmanaged"));
            add_manually_implemented_dtor();
        }
        if (handle.name == "VkQueue") {
            add_member(MemberInfo("VkDevice", "mVkDevice", "Device"));
            add_member(MemberInfo("gvk::Auto<VkDeviceQueueCreateInfo>", "mDeviceQueueCreateInfo", "VkDeviceQueueCreateInfo"));
        }
        if (handle.name == "VkBuffer") {
            add_manually_implemented_ctor("VkResult create(const Device& device, const VkBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, Buffer* pBuffer)");
            add_member(MemberInfo("VmaAllocation", "mVmaAllocation", "VmaAllocation"));
            add_manually_implemented_dtor();
        }
        if (handle.name == "VkImage") {
            add_manually_implemented_ctor("VkResult create(const Device& device, const VkImageCreateInfo* pImageCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, Image* pImage)");
            add_member(MemberInfo("VkSwapchainKHR", "mVkSwapchainKHR", "SwapchainKHR"));
            add_member(MemberInfo("VmaAllocation", "mVmaAllocation", "VmaAllocation"));
            add_manually_implemented_dtor();
        }
        if (handle.name == "VkDeferredOperationKHR") {
            generate_ctors(true, false);
        }
        if (handle.name == "VkSwapchainKHR") {
            add_member(MemberInfo("std::vector<Image>", "mImages", "const std::vector<Image>&"));
        }
        add_private_declaration("template <typename HandleType> friend VkResult gvk::detail::initialize_control_block(HandleType&)");
        if (handle.isDispatchable && handle.name != "VkCommandBuffer") {
            add_member(MemberInfo("DispatchTable", "mDispatchTable", "DispatchTable"));
        }
    }

    xml::Parameter get_ctor_parameter(const xml::Manifest& manifest, xml::Parameter parameter) const override final
    {
        if (manifest.handles.count(parameter.unqualifiedType)) {
            if (parameter.type == parameter.unqualifiedType) {
                parameter.type = "const " + parameter.type + "&";
            }
            parameter.type = string::strip_vk(parameter.type);
        }
        return parameter;
    }

protected:
    void generate_ctor(FileGenerator& file, const xml::Manifest& manifest, const xml::Command& ctor) const override final
    {
        assert(!ctor.parameters.empty());
        auto parameters = ctor.parameters;
        parameters.back().name = "pVk" + get_handle_name();
        if (parameters.back().flags & xml::Array) {
            parameters.back().name += "s";
        }

        std::string createInfoArg;
        for (const auto& parameter : parameters) {
            if (get_handle().createInfos.count(parameter.unqualifiedType)) {
                createInfoArg = parameter.name;
            }
        }

        auto handleId = parameters.back().flags & xml::Array ? parameters.back().name + "[i]" : "*" + parameters.back().name;
        if (!get_handle().isDispatchable) {
            auto dispatchableHandle = get_handle().get_dispatchable_handle(manifest);
            for (const auto& parameter : parameters) {
                if (parameter.type == dispatchableHandle) {
                    handleId = get_handle_id_type(manifest, get_handle()) + "(" + parameter.name + ", " + handleId + ")";
                    break;
                }
            }
        }

        std::vector<string::Replacement> replacements {
            { "{handleName}", get_handle_name() },
            { "{handleCount}", parameters.back().flags & xml::Array ? ctor.parameters.back().length : "1" },
            { "{outArg}", ctor.parameters.back().name },
            { "{createInfoArg}", createInfoArg },
            { "{ctorName}", get_ctor_name(ctor) },
            { "{ctorParameterList}", get_parameter_list(get_ctor_parameters(manifest, ctor)) },
            { "{vkCreateCommand}", ctor.name },
            { "{vkCreateCommandArgs}", get_parameter_list(parameters, false, true) },
            { "{handleId}", handleId },
            { "{initializeDispatchTableExpression}",
                get_handle().name == "VkInstance" ?
                "DispatchTable::load_global_entry_points(&dispatchTable)" :
                "dispatchTable = " + ctor.parameters.front().name + ".get<DispatchTable>()" },
        };

        if (parameters.back().flags & xml::Array) {
            file << string::replace(
R"(VkResult {handleName}::{ctorName}({ctorParameterList})
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        assert({createInfoArg});
        assert({handleCount});
        assert({outArg});
        for (uint32_t i = 0; i < {handleCount}; ++i) {
            {outArg}[i].reset();
        }
        DispatchTable dispatchTable { };
        {initializeDispatchTableExpression};
        assert(dispatchTable.g{vkCreateCommand});
        auto pVk{handleName}s = (Vk{handleName}*)detail::get_transient_storage({handleCount} * sizeof(Vk{handleName}));
        gvk_result(dispatchTable.g{vkCreateCommand}({vkCreateCommandArgs}));
        for (uint32_t i = 0; i < {handleCount}; ++i) {
            {outArg}[i].mReference.reset(gvk::newref, {handleId});
            auto& controlBlock = {outArg}[i].mReference.get_obj();
            controlBlock.mVk{handleName} = pVk{handleName}s[i];)", replacements) << std::endl;
            for (const auto& memberInfo : get_members()) {
                auto assignmentExpression = get_member_assignment_expression(manifest, ctor, memberInfo);
                if (!assignmentExpression.empty()) {
                    file << "            controlBlock." << memberInfo.storageName << " = " << assignmentExpression << ";" << std::endl;
                }
            }
            file << string::replace("            gvk_result(gvk::detail::initialize_control_block({outArg}[i]));", replacements) << std::endl;
            file << "        }" << std::endl;
            file << "    } gvk_result_scope_end;" << std::endl;
            file << "    return gvkResult;" << std::endl;
            file << "}" << std::endl;
        } else {
            file << string::replace(
R"(VkResult {handleName}::{ctorName}({ctorParameterList})
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        assert({createInfoArg});
        assert({outArg});
        DispatchTable dispatchTable { };
        {initializeDispatchTableExpression};
        assert(dispatchTable.g{vkCreateCommand});
        Vk{handleName} vk{handleName} = VK_NULL_HANDLE;
        auto pVk{handleName} = &vk{handleName};
        gvk_result(dispatchTable.g{vkCreateCommand}({vkCreateCommandArgs}));
        {outArg}->mReference.reset(gvk::newref, {handleId});
        auto& controlBlock = {outArg}->mReference.get_obj();
        controlBlock.mVk{handleName} = *pVk{handleName};)", replacements) << std::endl;
            for (const auto& memberInfo : get_members()) {
                auto assignmentExpression = get_member_assignment_expression(manifest, ctor, memberInfo);
                if (!assignmentExpression.empty()) {
                    file << "        controlBlock." << memberInfo.storageName << " = " << assignmentExpression << ";" << std::endl;
                }
            }
            file << string::replace("        gvk_result(gvk::detail::initialize_control_block(*{outArg}));", replacements) << std::endl;
            file << "    } gvk_result_scope_end;" << std::endl;
            file << "    return gvkResult;" << std::endl;
            file << "}" << std::endl;
        }
    }

    void generate_dtor(FileGenerator& file, const xml::Manifest& manifest, const xml::Command& dtor) const override final
    {
        if (dtor.name.empty()) {
            BasicHandleGenerator::generate_dtor(file, manifest, dtor);
        } else {
            auto parameters = dtor.parameters;
            for (auto& parameter : parameters) {
                if (parameter.unqualifiedType == dtor.target) {
                    if (parameter.length.empty()) {
                        parameter.name = "m" + parameter.unqualifiedType;
                    } else {
                        parameter.name = "&m" + parameter.unqualifiedType;
                    }
                } else if (parameter.unqualifiedType == "VkAllocationCallbacks") {
                    parameter.name = "(mAllocationCallbacks.pfnFree ? &mAllocationCallbacks : nullptr)";
                } else if (string::contains(parameter.name, "Count")) {
                    parameter.name = "1";
                } else {
                    parameter.name = "m" + string::strip_vk(parameter.unqualifiedType);
                }
            }
            std::vector<string::Replacement> replacements {
                { "{handleName}", get_handle_name() },
                { "{vkDestroyCommand}", dtor.name },
                { "{vkDestroyCommandArgs}", get_parameter_list(parameters, false, true) },
                { "{initializeDispatchTableExpression}",
                    get_handle().name == "VkInstance" ?
                    "dispatchTable = mDispatchTable" :
                    "dispatchTable = m" + string::strip_vk(dtor.parameters.front().type) + ".get<DispatchTable>()" },
            };
            file << string::replace(
R"({handleName}::ControlBlock::~ControlBlock()
{
    DispatchTable dispatchTable { };
    {initializeDispatchTableExpression};
    assert(dispatchTable.g{vkDestroyCommand});
    dispatchTable.g{vkDestroyCommand}({vkDestroyCommandArgs});
}
)", replacements);
        }
    }
};

class HandlesGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_HANDLES_GENERATED_INCLUDE_PATH,
            GVK_HANDLES_GENERATED_INCLUDE_PREFIX,
            GVK_HANDLES_GENERATED_SOURCE_PATH,
            "handles"
        );
        std::vector<HandleGenerator> generators;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            (void)handle;
            generators.emplace_back(manifest, handleItr.second);
            auto& generator = generators.back();
            (void)generator;


        }
        generate_forward_declarations(generators);
        generate_header(module.header, manifest, generators);
        generate_source(module.source, manifest, generators);
    }

private:
    static void generate_forward_declarations(const std::vector<HandleGenerator>& generators)
    {
        FileGenerator file(GVK_HANDLES_GENERATED_INCLUDE_PATH "/forward-declarations.inl");
        file << std::endl;
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        for (const auto& generator : generators) {
            CompileGuardGenerator compileGuardGenerator(file, generator.get_handle().compileGuards);
            file << "class " << generator.get_handle_name() << ";" << std::endl;
        }
        file << std::endl;
    }

    static void generate_header(
        FileGenerator& file,
        const xml::Manifest& manifest,
        const std::vector<HandleGenerator>& generators
    )
    {
        file << "#include \"gvk-handles/detail/handle-utilities.hpp\"" << std::endl;
        file << "#include \"gvk-handles/generated/forward-declarations.inl\"" << std::endl;
        file << "#include \"gvk-handles/defines.hpp\"" << std::endl;
        file << "#include \"gvk-dispatch-table.hpp\"" << std::endl;
        file << "#include \"gvk-reference.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <type_traits>" << std::endl;
        file << "#include <vector>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& generator : generators) {
            generator.generate_handle_declaration(file, manifest);
        }
        for (const auto& generator : generators) {
            generator.generate_control_block_declaration(file, manifest);
        }
        for (const auto& generator : generators) {
            generator.generate_accessors(file, manifest);
        }
        file << std::endl;
    }

    static void generate_source(
        FileGenerator& file,
        const xml::Manifest& manifest,
        const std::vector<HandleGenerator>& generators
    )
    {
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& generator : generators) {
            generator.generate_handle_definition(file, manifest);
        }
        for (const auto& generator : generators) {
            generator.generate_control_block_definition(file, manifest);
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
