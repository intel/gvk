
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

#include "gvk-cppgen/structure-make-tuple-generator.hpp"
#include "gvk-cppgen/basic-structure-member-processor-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/namespace-generator.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

class StructureMemberToTupleElementGenerator final
    : public BasicStructureMemberProcessorGenerator
{
protected:
    std::string generate_pnext_processor() const override final
    {
        return "detail::PNextTupleElementWrapper { obj.{memberName} }";
    }

    std::string generate_void_pointer_processor() const override final
    {
        return "(uint64_t)obj.{memberName}";
    }

    std::string generate_function_pointer_processor() const override final
    {
        return "(uint64_t)obj.{memberName}";
    }

    std::string generate_dynamic_handle_array_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { gvk::detail::get_count(obj.{memberLength}), obj.{memberName} }";
    }

    std::string generate_dynamic_structure_array_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { gvk::detail::get_count(obj.{memberLength}), obj.{memberName} }";
    }

    std::string generate_dynamic_enumeration_array_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { gvk::detail::get_count(obj.{memberLength}), obj.{memberName} }";
    }

    std::string generate_dynamic_string_processor() const override final
    {
        return "detail::StringTupleElementWrapper { obj.{memberName} }";
    }

    std::string generate_dynamic_string_array_processor() const override final
    {
        return "detail::StringArrayTupleElementWrapper { gvk::detail::get_count(obj.{memberLength}), obj.{memberName} }";
    }

    std::string generate_dynamic_primitive_array_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { gvk::detail::get_count(obj.{memberLength}), obj.{memberName} }";
    }

    std::string generate_handle_pointer_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { 1, obj.{memberName} }";
    }

    std::string generate_structure_pointer_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { 1, obj.{memberName} }";
    }

    std::string generate_enumeration_pointer_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { 1, obj.{memberName} }";
    }

    std::string generate_primitive_pointer_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { 1, obj.{memberName} }";
    }

    std::string generate_static_handle_array_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { {memberLength}, obj.{memberName} }";
    }

    std::string generate_static_structure_array_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { {memberLength}, obj.{memberName} }";
    }

    std::string generate_static_enumeration_array_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { {memberLength}, obj.{memberName} }";
    }

    std::string generate_static_string_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { {memberLength}, obj.{memberName} }";
    }

    std::string generate_static_primitive_array_processor() const override final
    {
        return "detail::ArrayTupleElementWrapper<{memberUnqualifiedType}> { {memberLength}, obj.{memberName} }";
    }

    std::string generate_handle_processor() const override final
    {
        return "obj.{memberName}";
    }

    std::string generate_structure_processor() const override final
    {
        return "obj.{memberName}";
    }

    std::string generate_enumeration_processor() const override final
    {
        return "obj.{memberName}";
    }

    std::string generate_flags_processor() const override final
    {
        return "obj.{memberName}";
    }

    std::string generate_primitive_processor() const override final
    {
        return "obj.{memberName}";
    }
};

void StructureMakeTupleGenerator::generate(
    const xml::Manifest& manifest,
    const ApiElementCollectionInfo& apiElements,
    const std::string& manualImplementationInclude
)
{
    FileGenerator file(apiElements.includePath + "/" + apiElements.name + "-structure-make-tuple.hpp");
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    file << "#include \"gvk-structures/detail/get-count.hpp\"" << std::endl;
    file << "#include \"gvk-structures/detail/make-tuple-manual.hpp\"" << std::endl;
    file << "#include \"gvk-structures/detail/make-tuple-utilities.hpp\"" << std::endl;
    if (!manualImplementationInclude.empty()) {
        file << "#include \"" << manualImplementationInclude << "\"" << std::endl;
    }
    file << std::endl;
    file << "#include <tuple>" << std::endl;
    file << std::endl;
    NamespaceGenerator namespaceGenerator(file, "gvk");
    for (const auto& structure : apiElements.structures) {
        if (structure.alias.empty() && !apiElements.manuallyImplemented.count(structure.name)) {
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << string::replace("inline auto make_tuple(const {structureType}& obj)", "{structureType}", structure.name) << std::endl;
            file << "{" << std::endl;
            file << "    return std::make_tuple(" << std::endl;
            int memberCount = 0;
            for (const auto& member : structure.members) {
                if (memberCount++) {
                    file << "," << std::endl;
                }
                file << "        " << StructureMemberToTupleElementGenerator().generate(manifest, member);
            }
            file << std::endl;
            file << "    );" << std::endl;
            file << "}" << std::endl;
        }
    }
    file << std::endl;
}

} // namespace cppgen
} // namespace gvk
