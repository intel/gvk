
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

#include "gvk-cppgen/structure-cerealization-generator.hpp"
#include "gvk-cppgen/basic-structure-member-processor-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/namespace-generator.hpp"
#include "gvk-cppgen/utilities.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

class CerealizeStructureMemberGenerator final
    : public BasicStructureMemberProcessorGenerator
{
protected:
    std::string generate_pnext_processor() const override final
    {
        return "gvk::detail::cerealize_pnext(archive, obj.{memberName});";
    }

    std::string generate_void_pointer_processor() const override final
    {
        return "gvk::detail::cerealize_handle(archive, obj.{memberName});";
    }

    std::string generate_function_pointer_processor() const override final
    {
        return "// NOPE : Function pointer '{memberName}' not serialized";
    }

    std::string generate_dynamic_handle_array_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_handle_array(archive, gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_dynamic_structure_array_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_array(archive, gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_dynamic_enumeration_array_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_array(archive, gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_dynamic_string_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_string(archive, obj.{memberName});";
    }

    std::string generate_dynamic_string_array_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_string_array(archive, gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_dynamic_primitive_array_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_array(archive, gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_handle_pointer_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_handle_array(archive, 1, obj.{memberName});";
    }

    std::string generate_structure_pointer_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_array(archive, 1, obj.{memberName});";
    }

    std::string generate_enumeration_pointer_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_array(archive, 1, obj.{memberName});";
    }

    std::string generate_primitive_pointer_processor() const override final
    {
        return "gvk::detail::cerealize_dynamic_array(archive, 1, obj.{memberName});";
    }

    std::string generate_static_handle_array_processor() const override final
    {
        return "gvk::detail::cerealize_static_handle_array<{memberLength}>(archive, obj.{memberName});";
    }

    std::string generate_static_structure_array_processor() const override final
    {
        return "gvk::detail::cerealize_static_array<{memberLength}>(archive, obj.{memberName});";
    }

    std::string generate_static_enumeration_array_processor() const override final
    {
        return "gvk::detail::cerealize_static_array<{memberLength}>(archive, obj.{memberName});";
    }

    std::string generate_static_string_processor() const override final
    {
        return "gvk::detail::cerealize_static_array<{memberLength}>(archive, obj.{memberName});";
    }

    std::string generate_static_primitive_array_processor() const override final
    {
        return "gvk::detail::cerealize_static_array<{memberLength}>(archive, obj.{memberName});";
    }

    std::string generate_handle_processor() const override final
    {
        return "gvk::detail::cerealize_handle(archive, obj.{memberName});";
    }

    std::string generate_structure_processor() const override final
    {
        return "archive(obj.{memberName});";
    }

    std::string generate_enumeration_processor() const override final
    {
        return "archive(obj.{memberName});";
    }

    std::string generate_flags_processor() const override final
    {
        return "archive(obj.{memberName});";
    }

    std::string generate_primitive_processor() const override final
    {
        return "archive(obj.{memberName});";
    }
};

void StructureCerealizationGenerator::generate(
    const xml::Manifest& manifest,
    const ApiElementCollectionInfo& apiElements,
    const std::string& manualImplementationInclude
)
{
    FileGenerator file(apiElements.includePath + "/" + apiElements.name + "-structure-cerealization.hpp");
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    for (const auto& include : apiElements.headerIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"gvk-structures/detail/cerealization-utilities.hpp\"" << std::endl;
    file << "#include \"gvk-structures/detail/get-count.hpp\"" << std::endl;
    file << "#include \"gvk-structures/generated/cerealize-pnext.hpp\"" << std::endl;
    if (!manualImplementationInclude.empty()) {
        file << "#include \"" << manualImplementationInclude << "\"" << std::endl;
    }
    file << std::endl;
    NamespaceGenerator namespaceGenerator(file, "cereal");
    for (const auto& structure : apiElements.structures) {
        if (structure.alias.empty() && !apiElements.manuallyImplemented.count(structure.name)) {
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << "template <typename ArchiveType>" << std::endl;
            file << string::replace("inline void save(ArchiveType& archive, const {structureType}& obj)", "{structureType}", structure.name) << std::endl;
            file << "{" << std::endl;
            file << "    (void)archive;" << std::endl;
            file << "    (void)obj;" << std::endl;
            for (size_t i = 0; i < structure.members.size(); ++i) {
                const auto& member = structure.members[i];
                CompileGuardGenerator memberCompileGuardGenerator(file, get_inner_scope_compile_guards(structure.compileGuards, member.compileGuards));
                auto source = CerealizeStructureMemberGenerator().generate(manifest, member);
                if (!source.empty()) {
                    file << "    " << source << std::endl;
                }
            }
            file << "}" << std::endl;
        }
    }
    file << std::endl;
}

} // namespace cppgen
} // namespace gvk
