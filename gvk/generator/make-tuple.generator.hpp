
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

#include "gvk/xml/manifest.hpp"
#include "cppgen-utilities.hpp"

namespace gvk {
namespace cppgen {

class MakeTupleGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("make-tuple");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    class StructureMemberToTupleElementGenerator final
        : public StructureMemberGenerator
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
            return "detail::ArrayTupleElementWrapper { obj.{memberLength}, obj.{memberName} }";
        }

        std::string generate_dynamic_structure_array_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { obj.{memberLength}, obj.{memberName} }";
        }

        std::string generate_dynamic_enumeration_array_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { obj.{memberLength}, obj.{memberName} }";
        }

        std::string generate_dynamic_string_processor() const override final
        {
            return "detail::StringTupleElementWrapper { obj.{memberName} }";
        }

        std::string generate_dynamic_string_array_processor() const override final
        {
            return "detail::StringArrayTupleElementWrapper { obj.{memberLength}, obj.{memberName} }";
        }

        std::string generate_dynamic_primitive_array_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { obj.{memberLength}, obj.{memberName} }";
        }

        std::string generate_handle_pointer_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { 1, obj.{memberName} }";
        }

        std::string generate_structure_pointer_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { 1, obj.{memberName} }";
        }

        std::string generate_enumeration_pointer_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { 1, obj.{memberName} }";
        }

        std::string generate_primitive_pointer_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { 1, obj.{memberName} }";
        }

        std::string generate_static_handle_array_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { {memberLength}, obj.{memberName} }";
        }

        std::string generate_static_structure_array_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { {memberLength}, obj.{memberName} }";
        }

        std::string generate_static_enumeration_array_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { {memberLength}, obj.{memberName} }";
        }

        std::string generate_static_string_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { {memberLength}, obj.{memberName} }";
        }

        std::string generate_static_primitive_array_processor() const override final
        {
            return "detail::ArrayTupleElementWrapper { {memberLength}, obj.{memberName} }";
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

    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/detail/make-tuple-utilities.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty() && !structure_requires_custom_implementation(structure.name)) {
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
                    file << "        " << StructureMemberToTupleElementGenerator{}.generate(manifest, member);
                }
                file << std::endl;
                file << "    );" << std::endl;
                file << "}" << std::endl;
            }
        }
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/detail/make-tuple-utilities.hpp\"" << std::endl;
        file << "#include \"gvk/generated/comparison-operators.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate operator==() definition for PNextTupleElementWrapper...
        file << "bool operator==(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)" << std::endl;
        file << "{" << std::endl;
        file << "    if (lhs.pNext && rhs.pNext) {" << std::endl;
        file << "        auto lhsSType = ((const VkBaseInStructure*)lhs.pNext)->sType;" << std::endl;
        file << "        auto rhsSType = ((const VkBaseInStructure*)rhs.pNext)->sType;" << std::endl;
        file << "        if (lhsSType != rhsSType) {" << std::endl;
        file << "            return false;" << std::endl;
        file << "        }" << std::endl;
        generate_pnext_switch(
            file,
            manifest,
            "        ",
            "lhsSType",
            "return *(const {structureType}*)lhs.pNext == *(const {structureType}*)rhs.pNext;",
            "assert(false && \"Unrecognized VkStructureType\");"
        );
        file << "    }" << std::endl;
        file << "    return !lhs.pNext == !rhs.pNext;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate operator<() definition for PNextTupleElementWrapper...
        file << "bool operator<(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)" << std::endl;
        file << "{" << std::endl;
        file << "    if (lhs.pNext && rhs.pNext) {" << std::endl;
        file << "        auto lhsSType = ((const VkBaseInStructure*)lhs.pNext)->sType;" << std::endl;
        file << "        auto rhsSType = ((const VkBaseInStructure*)rhs.pNext)->sType;" << std::endl;
        file << "        if (lhsSType != rhsSType) {" << std::endl;
        file << "            return lhsSType < rhsSType;" << std::endl;
        file << "        }" << std::endl;
        generate_pnext_switch(
            file,
            manifest,
            "        ",
            "lhsSType",
            "return *(const {structureType}*)lhs.pNext < *(const {structureType}*)rhs.pNext;",
            "assert(false && \"Unrecognized VkStructureType\");"
        );
        file << "    }" << std::endl;
        file << "    return !lhs.pNext == !!rhs.pNext;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
