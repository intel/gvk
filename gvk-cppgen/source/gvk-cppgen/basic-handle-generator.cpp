
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

#include "gvk-cppgen/basic-handle-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/utilities.hpp"
#include "gvk-string/utilities.hpp"

#include <cassert>
#include <sstream>

namespace gvk {
namespace cppgen {

BasicHandleGenerator::MemberInfo::MemberInfo(
    const std::string& memberStorageType,
    const std::string& memberStorageName,
    const std::string& memberAccessorType,
    const std::string& memberAccessExpression,
    const std::set<std::string>& memberCompileGuards
)
    : storageType { memberStorageType }
    , storageName { memberStorageName }
    , accessorType { memberAccessorType }
    , accessExpression { memberAccessExpression }
    , compileGuards { memberCompileGuards }
{
}

bool operator<(const BasicHandleGenerator::MemberInfo& lhs, const BasicHandleGenerator::MemberInfo& rhs)
{
    return lhs.storageType < rhs.storageType;
}

BasicHandleGenerator::BasicHandleGenerator(const xml::Manifest& manifest, const xml::Handle& handle)
    : mHandle { handle }
{
    MemberInfo memberInfo;
    memberInfo.storageType = handle.name;
    memberInfo.storageName = "m" + handle.name;
    add_member(memberInfo);

    for (const auto& commandName : mHandle.createCommands) {
        const auto& commandItr = manifest.commands.find(commandName);
        assert(commandItr != manifest.commands.end());
        add_ctor(manifest, commandItr->second);
    }

    assert(handle.destroyCommands.size() <= 1);
    if (!handle.destroyCommands.empty()) {
        const auto& commandItr = manifest.commands.find(*handle.destroyCommands.begin());
        assert(commandItr != manifest.commands.end());
        mDtor = commandItr->second;
        mDtor.compileGuards = get_inner_scope_compile_guards(mHandle.compileGuards, mDtor.compileGuards);
    }

    // NOTE : VkDescriptorSetAllocateInfo has an array of VkDescriptorSetLayouts
    //  that are referenced by the allocated VkDescriptorSets, but each allocated
    //  DescriptorSet only references one DescriptorSetLayout so the std::vector<>
    //  member is erased() and replaced with a single DescriptorSetLayout reference.
    if (handle.name == "VkDescriptorSet") {
        mMemberInfos.erase(MemberInfo("std::vector<DescriptorSetLayout>"));
        memberInfo.storageType = "DescriptorSetLayout";
        memberInfo.storageName = "mDescriptorSetLayout";
        memberInfo.accessorType = "const " + memberInfo.storageType + "&";
        add_member(memberInfo);
    }
}

BasicHandleGenerator::~BasicHandleGenerator()
{
}

const xml::Handle& BasicHandleGenerator::get_handle() const
{
    return mHandle;
}

std::string BasicHandleGenerator::get_handle_name() const
{
    return string::strip_vk(mHandle.name);
}

std::string BasicHandleGenerator::get_handle_vk_object_type() const
{
    return !mHandle.vkObjectType.empty() ? mHandle.vkObjectType : "{ }";
}

std::string BasicHandleGenerator::get_ctor_name(const xml::Command& ctor) const
{
    if (string::contains(ctor.name, "Create")) {
        return "create";
    }
    if (string::contains(ctor.name, "Allocate")) {
        return "allocate";
    }
    return ctor.name;
}

std::vector<xml::Parameter> BasicHandleGenerator::get_ctor_parameters(const xml::Manifest& manifest, const xml::Command& ctor) const
{
    auto parameters = ctor.parameters;
    for (auto& parameter : parameters) {
        parameter = get_ctor_parameter(manifest, parameter);
    }
    return parameters;
}

xml::Parameter BasicHandleGenerator::get_ctor_parameter(const xml::Manifest&, xml::Parameter parameter) const
{
    return parameter;
}

const std::set<BasicHandleGenerator::MemberInfo>& BasicHandleGenerator::get_members() const
{
    return mMemberInfos;
}

std::string BasicHandleGenerator::get_member_assignment_expression(const xml::Manifest& manifest, const xml::Command& command, const BasicHandleGenerator::MemberInfo& memberInfo) const
{
    if (memberInfo.storageName != "m" + mHandle.name) {
        auto memberVkType = "Vk" + string::strip_vk(string::remove(string::remove(memberInfo.storageType, "gvk::Auto<"), ">"));
        if (manifest.structures.count(memberVkType)) {
            for (const auto& parameter : command.parameters) {
                if (parameter.unqualifiedType == memberVkType) {
                    assert(parameter.flags & xml::Pointer);
                    auto parameterDereference = string::replace(
                        "{dereference}{parameterName}{index}", {
                            { "{dereference}", parameter.flags & xml::Array ? std::string() : "*" },
                            { "{parameterName}", parameter.name },
                            { "{index}", parameter.flags & xml::Array ? "[i]" : std::string() },
                        }
                    );
                    return string::replace(
                        mHandle.createInfos.count(parameter.unqualifiedType) ?
                            "{parameterDereference}" :
                            "{parameterName} ? {parameterDereference} : {parameterUnqualifiedType} { }", {
                                { "{parameterName}", parameter.name },
                                { "{parameterDereference}", parameterDereference },
                                { "{parameterUnqualifiedType}", parameter.unqualifiedType },
                        }
                    );
                }
            }
        }
        const auto& memberHandleItr = manifest.handles.find(memberVkType);
        if (memberHandleItr != manifest.handles.end()) {
            const auto& memberHandle = memberHandleItr->second;
            for (const auto& parameter : command.parameters) {
                if (memberVkType == parameter.unqualifiedType) {
                    assert(!(parameter.flags & xml::Pointer));
                    assert(command.parameters.begin()->type == memberHandle.get_dispatchable_handle(manifest));
                    if (memberInfo.storageType == memberVkType) {
                        return parameter.name;
                    } else {
                        return memberHandle.isDispatchable ? parameter.name : string::replace(
                            "{handleIdType}({dispatchableHandleParameterName}, {paramaterName})", {
                                { "{handleIdType}", get_handle_id_type(manifest, memberHandle) },
                                { "{dispatchableHandleParameterName}", command.parameters.begin()->name },
                                { "{paramaterName}", parameter.name }
                            }
                        );
                    }
                }
                const auto& parameterStructureItr = manifest.structures.find(parameter.unqualifiedType);
                if (parameterStructureItr != manifest.structures.end()) {
                    for (const auto& parameterMember : parameterStructureItr->second.members) {
                        if (memberVkType == parameterMember.unqualifiedType) {
                            assert(!memberHandle.isDispatchable);
                            auto parameterDereference = string::replace(
                                "{parameterName}{dereference}", {
                                    { "{parameterName}", parameter.name },
                                    { "{dereference}", parameter.flags & xml::Array ? "[i]." : "->" },
                                }
                            );
                            auto parameterMemberDereference = string::replace(
                                "{parameterMemberName}{dereference}", {
                                    { "{parameterMemberName}", parameterMember.name },
                                    { "{dereference}", parameterMember.flags & xml::Array ? "[i]" : std::string() },
                                }
                            );
                            if (command.parameters.begin()->type == memberHandle.get_dispatchable_handle(manifest)) {
                                if (memberInfo.storageType == memberVkType) {
                                    return string::replace(
                                        "{parameterName} ? {parameterDereference}{parameterMemberDereference} : VK_NULL_HANDLE", {
                                            { "{parameterName}", parameter.name },
                                            { "{parameterDereference}", parameterDereference },
                                            { "{parameterMemberDereference}", parameterMemberDereference },
                                        }
                                    );
                                } else {
                                    return string::replace(
                                        "{handleIdType}({dispatchableHandleParameterName}, {parameterDereference}{parameterMemberDereference})", {
                                            { "{handleIdType}", get_handle_id_type(manifest, memberHandle) },
                                            { "{dispatchableHandleParameterName}", command.parameters.begin()->name },
                                            { "{parameterName}", parameter.name },
                                            { "{parameterDereference}", parameterDereference },
                                            { "{parameterMemberDereference}", parameterMemberDereference },
                                        }
                                    );
                                }
                            } else {
                                return "{ /* NOTE : Manually initialized */ }";
                            }
                        }
                    }
                }
            }
        }
    }
    return std::string();
}

void BasicHandleGenerator::generate_handle_declaration(FileGenerator& file, const xml::Manifest& manifest) const
{
    std::vector<string::Replacement> replacements {
        { "{handleName}", get_handle_name() },
        { "{handleIdType}", get_handle_id_type(manifest, mHandle) },
        { "{dispatchableVkHandleType}", mHandle.get_dispatchable_handle(manifest) },
    };
    file << "\n";
    CompileGuardGenerator handleCompileGuardGenerator(file, mHandle.compileGuards);
    file << string::replace(
R"(class {handleName} final
{
public:
    using VkHandleType = Vk{handleName};
    using DispatchableVkHandleType = {dispatchableVkHandleType};
    using HandleIdType = {handleIdType};
    {handleName}() = default;
    inline {handleName}(std::nullptr_t) { };
    inline {handleName}(gvk::nullref_t) { };
    inline {handleName}(const HandleIdType& handleId) { *this = {handleName}::get(handleId); }
    {handleName}(const {handleName}&) = default;
    {handleName}({handleName}&&) = default;
    {handleName}& operator=(const {handleName}&) = default;
    {handleName}& operator=({handleName}&&) = default;
    inline operator Vk{handleName}() const { return get<Vk{handleName}>(); };
)", replacements);
    if (mGenerateCtorDeclarations) {
        for (const auto& ctor : mCtors) {
            auto ctorReplacements = get_inner_scope_replacements(replacements, {
                { "{ctorName}", get_ctor_name(ctor) },
                { "{ctorParameterList}", get_parameter_list(get_ctor_parameters(manifest, ctor)) },
                });
            CompileGuardGenerator ctorCompileGuardGenerator(file, ctor.compileGuards);
            file << string::replace("    static VkResult {ctorName}({ctorParameterList});\n", ctorReplacements);
        }
    }
    for (const auto& manuallyImplementedCtorSignature : mManuallyImplementedCtorSignatures) {
        file << "    static " << manuallyImplementedCtorSignature << ";\n";
    }
    for (const auto& methodInfo : mMethodInfos) {
        file << string::replace("    {methodReturnType} {methodName}({methodParameters}) const;\n", {
            { "{methodReturnType}", methodInfo.method.returnType },
            { "{methodName}", methodInfo.method.name },
            { "{methodParameters}", get_parameter_list(methodInfo.method.parameters) },
        });
    }
    file << string::replace(
R"(    static {handleName} get(const HandleIdType& handleId);
    template <typename T> T get() const;
    void reset();
private:
    class ControlBlock;
    gvk::Reference<ControlBlock, HandleIdType> mReference;
)", replacements);
    for (const auto& declaration : mPrivateDeclarations) {
        file << "    " << declaration << ";\n";
    }
    file << "};\n";
}

void BasicHandleGenerator::generate_handle_definition(FileGenerator& file, const xml::Manifest& manifest) const
{
    std::vector<string::Replacement> replacements {
        { "{handleName}", get_handle_name() },
        { "{getHandle}", !mHandle.isDispatchable ? ".get_handle()" : std::string() }
    };
    file << "\n";
    CompileGuardGenerator handleCompileGuardGenerator(file, mHandle.compileGuards);
    if (mGenerateCtorDefinitions) {
        for (const auto& ctor : mCtors) {
            {
                CompileGuardGenerator ctorCompileGuardGenerator(file, ctor.compileGuards);
                generate_ctor(file, manifest, ctor);
            }
            file << "\n";
        }
    }
    for (const auto& methodInfo : mMethodInfos) {
        if (methodInfo.manuallyImplemented) {
            file << "#if 0 // NOTE : Manually implemented\n";
        }
        file << string::replace("{methodReturnType} {handleName}::{methodName}({methodParameters}) const\n", {
            get_inner_scope_replacements(replacements, {
                { "{methodReturnType}", methodInfo.method.returnType },
                { "{methodName}", methodInfo.method.name },
                { "{methodParameters}", get_parameter_list(methodInfo.method.parameters) },
            })
        });
        file << methodInfo.body;
        if (methodInfo.manuallyImplemented) {
            file << "#endif\n";
        }
        file << "\n";
    }
    file << string::replace(
R"({handleName} {handleName}::get(const HandleIdType& handleId)
{
    {handleName} result;
    result.mReference = Reference<{handleName}::ControlBlock, HandleIdType>::get(handleId);
    return result;
}

void {handleName}::reset()
{
    *this = gvk::nullref;
}
)", replacements);
}

void BasicHandleGenerator::generate_control_block_declaration(FileGenerator& file, const xml::Manifest&) const
{
    std::vector<string::Replacement> replacements {
        { "{handleName}", get_handle_name() },
    };
    file << "\n";
    CompileGuardGenerator handleCompileGuardGenerator(file, mHandle.compileGuards);
    file << string::replace(
R"(class {handleName}::ControlBlock final
{
public:
    ControlBlock() = default;
    ~ControlBlock();
)", replacements);
    for (const auto& memberInfo : mMemberInfos) {
        if (!memberInfo.storageType.empty() && !memberInfo.storageName.empty()) {
            auto memberReplacements = get_inner_scope_replacements(replacements, {
                { "{storageType}", memberInfo.storageType },
                { "{storageName}", memberInfo.storageName },
            });
            CompileGuardGenerator memberCompileGuardGenerator(file, memberInfo.compileGuards);
            file << string::replace("    {storageType} {storageName} { };\n", memberReplacements);
        }
    }
    file << string::replace(
R"(private:
    ControlBlock(const ControlBlock&) = delete;
    ControlBlock(ControlBlock&&) = delete;
    ControlBlock& operator=(const ControlBlock&) = delete;
    ControlBlock& operator=(ControlBlock&&) = delete;
};
)", replacements);
}

void BasicHandleGenerator::generate_control_block_definition(FileGenerator& file, const xml::Manifest& manifest) const
{
    if (!mManuallyImplementedDtor) {
        file << "\n";
        CompileGuardGenerator handleCompileGuardGenerator(file, mHandle.compileGuards);
        generate_dtor(file, manifest, mDtor);
    }
}

void BasicHandleGenerator::generate_accessors(FileGenerator& file, const xml::Manifest& manifest) const
{
    (void)manifest;
    std::vector<string::Replacement> replacements {
        { "{handleName}", get_handle_name() },
        { "{handleVkObjectType}", get_handle_vk_object_type() },
    };
    file << "\n";
    CompileGuardGenerator handleCompileGuardGenerator(file, mHandle.compileGuards);
    file << string::replace(
R"(template <typename T>
T {handleName}::get() const
{
    if constexpr (std::is_same_v<T, VkObjectType>) { return {handleVkObjectType}; }
)", replacements);
    if (mHandle.isDispatchable) {
        file << "    if constexpr (std::is_same_v<T, VkHandleType>) { return mReference.get_id(); }" << std::endl;
        file << "    if constexpr (std::is_same_v<T, const VkHandleType&>) { return mReference.get_id(); }" << std::endl;
    } else {
        file << "    if constexpr (std::is_same_v<T, VkHandleType>) { return mReference.get_id().get_handle(); }" << std::endl;
        file << "    if constexpr (std::is_same_v<T, const VkHandleType&>) { return mReference.get_id().get_handle(); }" << std::endl;
        file << "    if constexpr (std::is_same_v<T, DispatchableVkHandleType>) { return mReference.get_id().get_dispatchable_handle(); }\n";
        file << "    if constexpr (std::is_same_v<T, HandleIdType>) { return mReference.get_id(); }\n";
    }
    for (const auto& memberInfo : mMemberInfos) {
        if (memberInfo.accessorType != mHandle.name && !memberInfo.accessorType.empty() && !memberInfo.accessExpression.empty()) {
            auto memberReplacements = get_inner_scope_replacements(replacements, {
                { "{accessorType}", memberInfo.accessorType },
                { "{accessorExpression}", memberInfo.accessExpression },
            });
            CompileGuardGenerator memberCompileGuardGenerator(file, memberInfo.compileGuards);
            file << string::replace("    if constexpr (std::is_same_v<T, {accessorType}>) { assert(mReference && \"Attempting to dereference nullref {handleName}\"); return {accessorExpression}; }\n", memberReplacements);
        }
    }
    file << "}\n";
}

void BasicHandleGenerator::add_ctor(const xml::Manifest& manifest, const xml::Command& ctor)
{
    for (const auto& parameter : ctor.parameters) {
        const auto& structureItr = manifest.structures.find(parameter.unqualifiedType);
        if (structureItr != manifest.structures.end() || manifest.handles.count(parameter.unqualifiedType)) {
            add_member(manifest, parameter);
            if (structureItr != manifest.structures.end()) {
                for (const auto& member : structureItr->second.members) {
                    if (manifest.handles.count(member.unqualifiedType)) {
                        add_member(manifest, member);
                    }
                }
            }
        }
    }
    mCtors.push_back(ctor);
    mCtors.back().compileGuards = get_inner_scope_compile_guards(mHandle.compileGuards, mCtors.back().compileGuards);
}

void BasicHandleGenerator::generate_ctors(bool declaration, bool definition)
{
    mGenerateCtorDeclarations = declaration;
    mGenerateCtorDefinitions = definition;
}

void BasicHandleGenerator::add_member(const xml::Manifest& manifest, const xml::Parameter& member)
{
    MemberInfo memberInfo;
    memberInfo.storageType = member.type;
    memberInfo.storageName = "m" + string::strip_vk(member.unqualifiedType);
    memberInfo.accessorType = member.unqualifiedType;
    if (member.unqualifiedType == mHandle.name) {
        memberInfo.accessorType.clear();
        memberInfo.accessExpression.clear();
        memberInfo.storageType = mHandle.name;
        memberInfo.storageName = "m" + mHandle.name;
    } else {
        const auto& handleItr = manifest.handles.find(member.unqualifiedType);
        if (handleItr != manifest.handles.end()) {
            memberInfo.compileGuards = handleItr->second.compileGuards;
            if (member.length.empty()) {
                memberInfo.storageType = string::strip_vk(member.unqualifiedType);
                memberInfo.accessorType = memberInfo.storageType;
            } else {
                memberInfo.storageType = "std::vector<" + string::strip_vk(member.unqualifiedType) + ">";
                memberInfo.storageName += "s";
                memberInfo.accessorType = "const " + memberInfo.storageType + "&";
            }
        } else {
            const auto& structureItr = manifest.structures.find(member.unqualifiedType);
            if (structureItr != manifest.structures.end()) {
                memberInfo.compileGuards = structureItr->second.compileGuards;
                memberInfo.accessorType = member.unqualifiedType;
                if (member.unqualifiedType == "VkAllocationCallbacks") {
                    memberInfo.storageType = member.unqualifiedType;
                } else {
                    memberInfo.storageType = "gvk::Auto<" + member.unqualifiedType + ">";
                }
            }
        }
    }
    add_member(memberInfo);
}

void BasicHandleGenerator::add_member(MemberInfo memberInfo)
{
    if (!memberInfo.accessorType.empty() && memberInfo.accessExpression.empty()) {
        memberInfo.accessExpression = "mReference.get_obj()." + memberInfo.storageName;
    }
    memberInfo.compileGuards = get_inner_scope_compile_guards(mHandle.compileGuards, memberInfo.compileGuards);
    mMemberInfos.erase(memberInfo);
    mMemberInfos.insert(memberInfo);
}

void BasicHandleGenerator::erase_member(const std::string& storageType)
{
    MemberInfo memberInfo;
    memberInfo.storageType = storageType;
    mMemberInfos.erase(memberInfo);
}

void BasicHandleGenerator::add_method(const MethodInfo& methodInfo)
{
    mMethodInfos.push_back(methodInfo);
}

void BasicHandleGenerator::add_manually_implemented_ctor(const std::string& ctorSignature)
{
    mManuallyImplementedCtorSignatures.push_back(ctorSignature);
}

void BasicHandleGenerator::add_manually_implemented_dtor()
{
    mManuallyImplementedDtor = true;
}

void BasicHandleGenerator::add_private_declaration(const std::string& privateDeclaration)
{
    mPrivateDeclarations.insert(privateDeclaration);
}

void BasicHandleGenerator::generate_ctor(FileGenerator& file, const xml::Manifest& manifest, const xml::Command& ctor) const
{
    auto parameters = get_ctor_parameters(manifest, ctor);
    assert(!parameters.empty());
    std::vector<string::Replacement> replacements {
        { "{handleName}", get_handle_name() },
        { "{ctorName}", get_ctor_name(ctor) },
        { "{ctorParameterList}", get_parameter_list(parameters) },
    };
    CompileGuardGenerator ctorCompileGuardGenerator(file, ctor.compileGuards);
    file << string::replace("VkResult {handleName}::{ctorName}({ctorParameterList})\n", replacements);
    file << "{\n";
    for (const auto& parameter : parameters) {
        file << "    (void)" << get_ctor_parameter(manifest, parameter).name << ";\n";
    }
    file << "    return VK_SUCCESS;\n";
    file << "}\n";
}

void BasicHandleGenerator::generate_dtor(FileGenerator& file, const xml::Manifest&, const xml::Command&) const
{
    file << string::replace("{handleName}::ControlBlock::~ControlBlock()\n", "{handleName}", get_handle_name());
    file << "{\n";
    file << "}\n";
}

std::string get_handle_id_type(const xml::Manifest& manifest, const xml::Handle& handle)
{
    auto handleId = handle.get_dispatchable_handle(manifest);
    if (!handle.isDispatchable) {
        handleId = "gvk::HandleId<" + handleId + ", " + handle.name + ">";
    }
    return handleId;
}

} // namespace cppgen
} // namespace gvk
