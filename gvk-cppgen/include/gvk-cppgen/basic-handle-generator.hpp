
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

#include "gvk-cppgen/file-generator.hpp"
#include "gvk-xml.hpp"

#include <set>
#include <string>
#include <vector>

namespace gvk {
namespace cppgen {

std::string get_handle_id_type(const xml::Manifest& manifest, const xml::Handle& handle);

class BasicHandleGenerator
{
public:
    class MemberInfo final
    {
    public:
        MemberInfo(
            const std::string& storageType = std::string(),
            const std::string& storageName = std::string(),
            const std::string& accessorType = std::string(),
            const std::string& accessExpression = std::string(),
            const std::set<std::string>& compileGuards = std::set<std::string>()
        );

        std::string storageType;
        std::string storageName;
        std::string accessorType;
        std::string accessExpression;
        std::set<std::string> compileGuards;

        friend bool operator<(const MemberInfo& lhs, const MemberInfo& rhs);
    };

    class MethodInfo final
    {
    public:
        xml::Command method;
        std::string body;
        bool manuallyImplemented { false };
    };

    BasicHandleGenerator(const xml::Manifest& manifest, const xml::Handle& handle);
    virtual ~BasicHandleGenerator() = 0;

    const xml::Handle& get_handle() const;
    std::string get_handle_name() const;
    std::string get_handle_vk_object_type() const;
    std::string get_ctor_name(const xml::Command& ctor) const;
    virtual std::vector<xml::Parameter> get_ctor_parameters(const xml::Manifest& manifest, const xml::Command& ctor) const;
    virtual xml::Parameter get_ctor_parameter(const xml::Manifest& manifest, xml::Parameter parameter) const;
    const std::set<MemberInfo>& get_members() const;
    std::string get_member_assignment_expression(const xml::Manifest& manifest, const xml::Command& command, const MemberInfo& memberInfo) const;

    void generate_handle_definition(FileGenerator& file, const xml::Manifest& manifest) const;
    void generate_handle_declaration(FileGenerator& file, const xml::Manifest& manifest) const;
    void generate_control_block_declaration(FileGenerator& file, const xml::Manifest& manifest) const;
    void generate_control_block_definition(FileGenerator& file, const xml::Manifest& manifest) const;
    void generate_accessors(FileGenerator& file, const xml::Manifest& manifest) const;

protected:
    void add_ctor(const xml::Manifest& manifest, const xml::Command& ctor);
    void generate_ctors(bool declaration, bool definition);
    void add_member(const xml::Manifest& manifest, const xml::Parameter& member);
    void add_member(MemberInfo memberInfo);
    void erase_member(const std::string& storageType);
    void add_method(const MethodInfo& methodInfo);
    void add_manually_implemented_ctor(const std::string& ctorSignature);
    void add_manually_implemented_dtor();
    void add_private_declaration(const std::string& privateDeclaration);
    virtual void generate_ctor(FileGenerator& file, const xml::Manifest& manifest, const xml::Command& ctor) const;
    virtual void generate_dtor(FileGenerator& file, const xml::Manifest& manifest, const xml::Command& dtor) const;

private:
    xml::Handle mHandle;
    std::vector<xml::Command> mCtors;
    bool mGenerateCtorDeclarations { true };
    bool mGenerateCtorDefinitions { true };
    std::vector<std::string> mManuallyImplementedCtorSignatures;
    xml::Command mDtor;
    bool mManuallyImplementedDtor { false };
    std::set<MemberInfo> mMemberInfos;
    std::vector<MethodInfo> mMethodInfos;
    std::set<std::string> mPrivateDeclarations;
};

} // namespace cppgen
} // namespace gvk
