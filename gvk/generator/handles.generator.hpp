
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

#include <algorithm>
#include <set>
#include <sstream>

namespace gvk {
namespace cppgen {

class HandleGenerator final
{
public:
    static inline const std::vector<std::string>& get_additional_ctors(const std::string& handleName)
    {
        // NOTE : These ctors are added to the handle declaration as is, they must be
        //  implemented in:
        //      gvk/source/gvk/detail/handle-utilities.cpp
        static const std::vector<std::string> sEmptyAdditionalCtors{ };
        static const std::map<std::string, std::vector<std::string>> sAdditionalCtors{
            {
                "VkBuffer",
                {
                    { "static VkResult create(const Device& device, const VkBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, Buffer* pBuffer)" },
                },
            },
            {
                "VkImage",
                {
                    { "static VkResult create(const Device& device, const VkImageCreateInfo* pImageCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, Image* pImage)" },
                },
            },
        };
        auto itr = sAdditionalCtors.find(handleName);
        return itr != sAdditionalCtors.end() ? itr->second : sEmptyAdditionalCtors;
    }

    static inline bool custom_dtor_required(const std::string& handleName)
    {
        // NOTE : Any handles that need a custom dtor must be listed here and the dtor
        //  must be implemented in:
        //      gvk/source/gvk/detail/handle-utilities.cpp
        //      detail::ControlBlockType::~ControlBlockType()
        static const std::set<std::string> sCustomDtorRequired{
            "VkBuffer",
            "VkDevice",
            "VkImage",
        };
        return sCustomDtorRequired.count(handleName);
    }

    static inline const std::vector<std::pair<std::string, std::string>>& get_additional_members(const std::string& handleName)
    {
        // NOTE : The majority of members are created by crawling create commands and
        //  infos.  Members that aren't provided as arguments to a create command must
        //  be listed here and initialized manually in:
        //      gvk/include/gvk/detail/handle-utilities.hpp
        //      gvk/source/gvk/detail/handle-utilities.cpp
        //      detail::initialize_control_block<ControlBlockType>()
        static const std::vector<std::pair<std::string, std::string>> sEmptyAdditionalMembers{ };
        static const std::map<std::string, std::vector<std::pair<std::string, std::string>>> sAdditionalMembers{
            {
                "VkBuffer",
                {
                    { "VmaAllocation", "mVmaAllocation" },
                }
            },
            {
                "VkDevice",
                {
                    { "Instance", "mInstance" },
                    { "std::vector<QueueFamily>", "mQueueFamilies" },
                    { "VmaAllocator", "mVmaAllocator" },
                }
            },
            {
                "VkImage",
                {
                    { "VkSwapchainKHR", "mVkSwapchainKHR" },
                    { "VmaAllocation", "mVmaAllocation" },
                }
            },
            {
                "VkInstance",
                {
                    { "std::vector<PhysicalDevice>", "mPhysicalDevices" },
                }
            },
            {
                "VkPhysicalDevice",
                {
                    { "VkInstance", "mVkInstance" },
                }
            },
            {
                "VkQueue",
                {
                    { "VkDevice", "mVkDevice" },
                    { "VkDeviceQueueCreateInfo", "mDeviceQueueCreateInfo" },
                }
            },
            {
                "VkSwapchainKHR",
                {
                    { "std::vector<Image>", "mImages" },
                }
            },
        };
        auto itr = sAdditionalMembers.find(handleName);
        return itr != sAdditionalMembers.end() ? itr->second : sEmptyAdditionalMembers;
    }

    inline HandleGenerator(const xml::Manifest& manifest, const xml::Handle& handle)
        : mHandle{ handle }
    {
        // Vk handle member
        xml::Parameter handleMember;
        handleMember.type = mHandle.name;
        handleMember.name = "m" + mHandle.name;
        mMembers.push_back(handleMember);

        // Ctors
        for (const auto& commandName : mHandle.createCommands) {
            const auto& commandItr = manifest.commands.find(commandName);
            assert(commandItr != manifest.commands.end());
            add_ctor(manifest, commandItr->second);
        }

        // Dtor
        assert(handle.destroyCommands.size() <= 1);
        if (!handle.destroyCommands.empty()) {
            const auto& commandItr = manifest.commands.find(*handle.destroyCommands.begin());
            assert(commandItr != manifest.commands.end());
            mDtor = commandItr->second;
        }

        // Create infos
        for (const auto& structureName : mHandle.createInfos) {
            const auto& structureItr = manifest.structures.find(structureName);
            assert(structureItr != manifest.structures.end());
            const auto& structure = structureItr->second;
            xml::Parameter createInfoMember;
            createInfoMember.type = structureName;
            createInfoMember.name = "m" + string::strip_vk(structureName);
            createInfoMember.compileGuards = structure.compileGuards;
            mMembers.push_back(createInfoMember);
        }

        // Update compile guards
        for (auto& member : mMembers) {
            member.compileGuards = get_inner_scope_compile_guards(mHandle.compileGuards, member.compileGuards);
        }
        for (auto& command : mCtors) {
            command.compileGuards = get_inner_scope_compile_guards(mHandle.compileGuards, command.compileGuards);
        }
        mDtor.compileGuards = get_inner_scope_compile_guards(mHandle.compileGuards, mDtor.compileGuards);
    }

    inline void add_ctor(const xml::Manifest& manifest, xml::Command command)
    {
        for (const auto& parameter : command.parameters) {
            const auto& structureItr = manifest.structures.find(parameter.unqualifiedType);
            bool isHandle = manifest.handles.count(parameter.unqualifiedType);
            bool isStructure = structureItr != manifest.structures.end();
            bool isCreateInfo = mHandle.createInfos.count(parameter.unqualifiedType);
            if (isHandle || (isStructure && !isCreateInfo)) {
                add_member(parameter);
            }
            if (structureItr != manifest.structures.end()) {
                for (const auto& member : structureItr->second.members) {
                    if (manifest.handles.count(member.unqualifiedType)) {
                        add_member(member);
                    }
                }
            }
        }
        mCtors.push_back(command);
    }

    inline void add_additional_member(xml::Parameter member)
    {
        if (mAddtionalMemberTypes.insert(member.type).second) {
            member.unqualifiedType = member.type;
            add_member(member);
        }
    }

    inline void add_additional_ctor(const std::string& additionalCtor)
    {
        mAdditionalCtors.push_back(additionalCtor);
    }

    inline void generate_handle_declaration(File& file, const xml::Manifest& manifest) const
    {
        std::vector<string::Replacement> replacements{
            { "{handleTypeName}", string::strip_vk(mHandle.name) },
        };
        file << std::endl;
        CompileGuardGenerator compileGuardGenerator(file, mHandle.compileGuards);
        file << string::replace("class {handleTypeName} final", replacements) << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        file << string::replace("    using VkHandleType = Vk{handleTypeName};", replacements) << std::endl;
        for (const auto& command : mCtors) {
            CompileGuardGenerator commandCompileGuardGenerator(file, command.compileGuards);
            file << string::replace("    static VkResult {ctorName}({ctorDeclarationArgs});",
                get_inner_scope_replacements(replacements, {
                    { "{ctorName}", get_ctor_name(command) },
                    { "{ctorDeclarationArgs}", get_ctor_declaration_args(manifest, command) },
                })
            ) << std::endl;
        }
        for (const auto& additionalCtor : mAdditionalCtors) {
            file << "    " << additionalCtor << ";" << std::endl;
        }
        file << string::replace(
R"(    {handleTypeName}() = default;
    inline {handleTypeName}(std::nullptr_t) { }
    inline {handleTypeName}(Vk{handleTypeName} vk{handleTypeName}) { *this = {handleTypeName}::get(vk{handleTypeName}); };
    {handleTypeName}(const {handleTypeName}&) = default;
    {handleTypeName}({handleTypeName}&&) = default;
    {handleTypeName}& operator=(const {handleTypeName}&) = default;
    {handleTypeName}& operator=({handleTypeName}&&) = default;
    inline operator const Vk{handleTypeName}&() const { return mVk{handleTypeName}; }
    void reset();
    static {handleTypeName} get(Vk{handleTypeName} vk{handleTypeName});
    template <typename ObjectType> ObjectType get() const;

private:
    Vk{handleTypeName} mVk{handleTypeName} { VK_NULL_HANDLE };
    detail::Reference<detail::{handleTypeName}ControlBlock, Vk{handleTypeName}> mControlBlock;
    template <typename ControlBlockType>
    friend VkResult detail::initialize_control_block(ControlBlockType&);
};
)", replacements);
    }

    inline void generate_accessor_definition(File& file, const xml::Manifest& manifest) const
    {
        std::vector<string::Replacement> replacements{
            { "{handleTypeName}", string::strip_vk(mHandle.name) },
            { "{vkObjectType}", !mHandle.vkObjectType.empty() ? mHandle.vkObjectType : "{ }"},
        };
        file << std::endl;
        CompileGuardGenerator compileGuardGenerator(file, mHandle.compileGuards);
        file << string::replace(
R"(template <typename ObjectType>
ObjectType {handleTypeName}::get() const
{
    if constexpr (std::is_same_v<ObjectType, VkObjectType>) { return {vkObjectType}; }
    if constexpr (std::is_same_v<ObjectType, Vk{handleTypeName}>) { return mVk{handleTypeName}; }
)", replacements);
        for (const auto& member : mMembers) {
            if (member.type != mHandle.name) {
                CompileGuardGenerator memberCompileGuardGenerator(file, member.compileGuards);
                file << string::replace("    if constexpr (std::is_same_v<ObjectType, {accessorType}>) { return {accessorExpression}; }",
                    get_inner_scope_replacements(replacements, {
                        { "{accessorType}", get_accessor_type(manifest, member) },
                        { "{accessorExpression}", get_accessor_expression(manifest, member) },
                    })
                ) << std::endl;
            }
        }
        file << "}" << std::endl;
    }

    inline void generate_ctor_definition(File& file, const xml::Manifest& manifest, const xml::Command& command) const
    {
        auto handleCountExpression = command.parameters.back().length;
        std::vector<string::Replacement> replacements{
            { "{handleTypeName}",        string::strip_vk(mHandle.name) },
            { "{ctorName}",              get_ctor_name(command) },
            { "{ctorDeclarationArgs}",   get_ctor_declaration_args(manifest, command) },
            { "{handleCountExpression}", !handleCountExpression.empty() ? handleCountExpression : "1" },
            { "{vkCreateCommand}",       command.name },
            { "{vkCreateCallArgs}",      get_vk_create_call_args(command) },
            { "{outArg}",                command.parameters.back().name },
        };
        CompileGuardGenerator commandCompileGuardGenerator(file, command.compileGuards);
        file << string::replace("VkResult {handleTypeName}::{ctorName}({ctorDeclarationArgs})", replacements) << std::endl;
        file << string::replace(
R"({
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        uint32_t handleCount = {handleCountExpression};
        if (handleCount && {outArg}) {
            for (uint32_t i = 0; i < handleCount; ++i) {
                {outArg}[i].reset();
            }
            auto dispatchTable = DispatchTable::get_global_dispatch_table();
            assert(dispatchTable.g{vkCreateCommand});
            auto pVk{handleTypeName} = (Vk{handleTypeName}*)detail::get_transient_storage(handleCount * sizeof(Vk{handleTypeName}));
            gvk_result(dispatchTable.g{vkCreateCommand}({vkCreateCallArgs}));
            for (uint32_t i = 0; i < handleCount; ++i) {
                {outArg}[i].mVk{handleTypeName} = pVk{handleTypeName}[i];
                {outArg}[i].mControlBlock.reset(detail::newref, pVk{handleTypeName}[i]);
                auto& controlBlock = {outArg}[i].mControlBlock.get_obj();
                controlBlock.mVk{handleTypeName} = pVk{handleTypeName}[i];)", replacements);
        file << std::endl;
        for (const auto& member : mMembers) {
            auto memberAssignmentExpression = get_member_assignment_expression(manifest, command, member);
            if (!memberAssignmentExpression.empty()) {
                file << "                ";
                file << string::replace("controlBlock.{memberName} = {memberAssignmentExpression};",
                    get_inner_scope_replacements(replacements, {
                        { "{memberName}", get_member_name(manifest, member) },
                        { "{memberAssignmentExpression}", memberAssignmentExpression },
                    })
                ) << std::endl;
            }
        }
        file << "                gvk_result(detail::initialize_control_block(controlBlock));" << std::endl;
        file << "            }" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
    }

    inline void generate_handle_definition(File& file, const xml::Manifest& manifest) const
    {
        std::vector<string::Replacement> replacements{
            { "{handleTypeName}", string::strip_vk(mHandle.name) },
            { "{vkObjectType}", mHandle.vkObjectType },
        };
        file << std::endl;
        CompileGuardGenerator compileGuardGenerator(file, mHandle.compileGuards);
        for (const auto& command : mCtors) {
            generate_ctor_definition(file, manifest, command);
            file << std::endl;
        }
        file << string::replace(
R"(void {handleTypeName}::reset()
{
    mVk{handleTypeName} = VK_NULL_HANDLE;
    mControlBlock.reset();
}

{handleTypeName} {handleTypeName}::get(Vk{handleTypeName} vk{handleTypeName})
{
    {handleTypeName} result;
    result.mControlBlock = detail::Reference<detail::{handleTypeName}ControlBlock, Vk{handleTypeName}>::get(vk{handleTypeName});
    result.mVk{handleTypeName} = result.mControlBlock.get_id();
    return result;
}
)", replacements);
    }

    inline void generate_control_block_declaration(File& file, const xml::Manifest& manifest) const
    {
        std::vector<string::Replacement> replacements{
            { "{handleTypeName}", string::strip_vk(mHandle.name) },
        };
        file << std::endl;
        CompileGuardGenerator compileGuardGenerator(file, mHandle.compileGuards);
        file << string::replace("class {handleTypeName}ControlBlock final", replacements) << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        if (!mDtor.name.empty()) {
            file << string::replace("    ~{handleTypeName}ControlBlock();", replacements) << std::endl;
        }
        for (const auto& member : mMembers) {
            CompileGuardGenerator memberCompileGuardGenerator(file, member.compileGuards);
            file << string::replace("    {memberType} {memberName} { };", {
                { "{memberType}", get_member_type(manifest, member) },
                { "{memberName}", get_member_name(manifest, member) },
            }) << std::endl;
        }
        file << "};" << std::endl;
    }

    inline void generate_control_block_definition(File& file) const
    {
        if (!mDtor.name.empty() && !custom_dtor_required(mHandle.name)) {
            std::vector<string::Replacement> replacements{
                { "{handleTypeName}", string::strip_vk(mHandle.name) },
                { "{vkDestroyCommand}", mDtor.name },
                { "{vkDestroyCallArgs}", get_vk_destroy_call_args(mDtor) },
            };
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, mHandle.compileGuards);
            file << string::replace(
R"({handleTypeName}ControlBlock::~{handleTypeName}ControlBlock()
{
    auto dispatchTable = DispatchTable::get_global_dispatch_table();
    assert(dispatchTable.g{vkDestroyCommand});
    dispatchTable.g{vkDestroyCommand}({vkDestroyCallArgs});
}
)", replacements);
        }
    }

private:
    inline void add_member(xml::Parameter member)
    {
        if (member.unqualifiedType != mHandle.name && mMemberTypes.insert(member.unqualifiedType).second) {
            mMembers.push_back(member);
        }
    }

    inline std::string get_ctor_declaration_args(const xml::Manifest& manifest, xml::Command command) const
    {
        for (auto& parameter : command.parameters) {
            if (manifest.handles.count(parameter.unqualifiedType)) {
                if (parameter.type == parameter.unqualifiedType) {
                    parameter.type = "const " + parameter.type + "&";
                }
                parameter.type = string::strip_vk(parameter.type);
            }
        }
        return get_command_args(command, true, true);
    }

    inline std::string get_vk_create_call_args(xml::Command command) const
    {
        assert(command.type == xml::Command::Type::Create);
        command.parameters.back().name = "p" + mHandle.name;
        return get_command_args(command, false, true);
    }

    inline std::string get_ctor_name(const xml::Command& command) const
    {
        return string::contains(command.name, "Create") ? "create" : "allocate";
    }

    inline std::string get_vk_destroy_call_args(xml::Command command) const
    {
        for (auto& parameter : command.parameters) {
            if (parameter.unqualifiedType == command.target) {
                if (parameter.length.empty()) {
                    parameter.name = "m" + parameter.unqualifiedType;
                } else {
                    parameter.name = "&m" + parameter.unqualifiedType;
                }
            } else if (parameter.unqualifiedType == "VkAllocationCallbacks") {
                parameter.name = "(mAllocator.pfnFree ? &mAllocator : nullptr)";
            } else if (string::contains(parameter.name, "Count")) {
                parameter.name = "1";
            } else {
                parameter.name = "m" + string::strip_vk(parameter.unqualifiedType);
            }
        }
        return get_command_args(command, false, true);
    }

    inline std::string get_member_assignment_expression(
        const xml::Manifest& manifest,
        const xml::Command& command,
        const xml::Parameter& member
    ) const
    {
        if (member.type != mHandle.name) {
            for (const auto& parameter : command.parameters) {
                if (parameter.unqualifiedType == member.type) {
                    if (parameter.flags & xml::Pointer) {
                        return "*" + parameter.name;
                    } else {
                        return parameter.name;
                    }
                }
                const auto& structureItr = manifest.structures.find(parameter.unqualifiedType);
                if (structureItr != manifest.structures.end()) {
                    for (const auto& structureMember : structureItr->second.members) {
                        if (structureMember.unqualifiedType == member.type) {
                            if (parameter.length.empty()) {
                                return parameter.name + "->" + structureMember.name;
                            } else {
                                return parameter.name + "[i]." + structureMember.name;
                            }
                        }
                    }
                }
            }
        }
        return std::string();
    }

    inline std::string get_member_type(const xml::Manifest& manifest, const xml::Parameter& member) const
    {
        if (member.type != mHandle.name && !mAddtionalMemberTypes.count(member.type)) {
            if (manifest.handles.count(member.type) || manifest.handles.count(member.unqualifiedType)) {
                if (member.length.empty()) {
                    return string::strip_vk(member.type);
                } else {
                    return "std::vector<" + string::strip_vk(member.unqualifiedType) + ">";
                }
            } else if (manifest.structures.count(member.type) || manifest.structures.count(member.unqualifiedType)) {
                if (member.unqualifiedType == "VkAllocationCallbacks") {
                    return member.unqualifiedType;
                } else {
                    return "Auto<" + member.type + ">";
                }
            }
        }
        return member.type;
    }

    inline std::string get_member_name(const xml::Manifest& manifest, const xml::Parameter& member) const
    {
        if (member.type != mHandle.name && !mAddtionalMemberTypes.count(member.type)) {
            if (manifest.handles.count(member.type) || manifest.handles.count(member.unqualifiedType)) {
                if (member.length.empty()) {
                    return "m" + string::strip_vk(member.type);
                } else {
                    return "m" + string::strip_vk(member.unqualifiedType) + "s";
                }
            } else if (manifest.structures.count(member.type) || manifest.structures.count(member.unqualifiedType)) {
                if (member.unqualifiedType == "VkAllocationCallbacks") {
                    return "mAllocator";
                } else {
                    return "m" + string::strip_vk(member.type);
                }
            }
        }
        return member.name;
    }

    inline std::string get_accessor_type(const xml::Manifest& manifest, const xml::Parameter& member) const
    {
        if (member.type != mHandle.name) {
            if (manifest.handles.count(member.type) || manifest.handles.count(member.unqualifiedType)) {
                if (member.length.empty()) {
                    return string::strip_vk(member.type);
                } else {
                    return "const std::vector<" + string::strip_vk(member.unqualifiedType) + ">&";
                }
            } else if (manifest.structures.count(member.type) || manifest.structures.count(member.unqualifiedType)) {
                if (member.unqualifiedType == "VkAllocationCallbacks") {
                    return member.unqualifiedType;
                } else {
                    return member.type;
                }
            } else if (string::contains(member.type, "std")) {
                return "const " + member.type + "&";
            }
        }
        return member.type;
    }

    inline std::string get_accessor_expression(const xml::Manifest& manifest, const xml::Parameter& member) const
    {
        return "mControlBlock.get_obj()." + get_member_name(manifest, member);
    }

    xml::Handle mHandle;
    std::vector<xml::Command> mCtors;
    std::vector<std::string> mAdditionalCtors;
    xml::Command mDtor;
    std::vector<xml::Parameter> mMembers;
    std::set<std::string> mMemberTypes;
    std::set<std::string> mAddtionalMemberTypes;
};

class HandlesGenerator final
{
public:
    static inline void generate(const gvk::xml::Manifest& manifest)
    {
        Module module("handles");
        std::vector<HandleGenerator> handleGenerators;
        handleGenerators.reserve(manifest.handles.size());
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            handleGenerators.emplace_back(manifest, handle);
            auto& handleGenerator = handleGenerators.back();
            for (const auto& additionalCtor : HandleGenerator::get_additional_ctors(handle.name)) {
                handleGenerator.add_additional_ctor(additionalCtor);
            }
            for (const auto& additionalMember : HandleGenerator::get_additional_members(handle.name)) {
                xml::Parameter member;
                member.type = additionalMember.first;
                member.name = additionalMember.second;
                handleGenerator.add_additional_member(member);
            }
        }
        generate_header(module.header, manifest, handleGenerators);
        generate_source(module.source, manifest, handleGenerators);
    }

private:
    static inline void generate_header(File& file, const xml::Manifest& manifest, const std::vector<HandleGenerator>& handleGenerators)
    {
        file << "#include \"gvk/detail/handle-utilities.hpp\"" << std::endl;
        file << "#include \"gvk/detail/reference.hpp\"" << std::endl;
        file << "#include \"gvk/generated/forward-declarations.hpp\"" << std::endl;
        file << "#include \"gvk/structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <map>" << std::endl;
        file << "#include <type_traits>" << std::endl;
        file << "#include <vector>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& handleGenerator : handleGenerators) {
            handleGenerator.generate_handle_declaration(file, manifest);
        }
        file << std::endl;
        {
            NamespaceGenerator detailNamespaceGenerator(file, "detail");
            for (const auto& handleGenerator : handleGenerators) {
                handleGenerator.generate_control_block_declaration(file, manifest);
            }
            file << std::endl;
        }
        for (const auto& handleGenerator : handleGenerators) {
            handleGenerator.generate_accessor_definition(file, manifest);
        }
        file << std::endl;
    }

    static inline void generate_source(File& file, const xml::Manifest& manifest, const std::vector<HandleGenerator>& handleGenerators)
    {
        file << "#include \"gvk/generated/dispatch-table.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& handleGenerator : handleGenerators) {
            handleGenerator.generate_handle_definition(file, manifest);
        }
        file << std::endl;
        NamespaceGenerator detailNamespaceGenerator(file, "detail");
        for (const auto& handleGenerator : handleGenerators) {
            handleGenerator.generate_control_block_definition(file);
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
