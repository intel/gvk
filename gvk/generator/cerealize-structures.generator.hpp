
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

class CerealizeStructuresGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        File file("cerealize-structures.hpp");
        file << "#include \"gvk/detail/cerealization-utilities.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::detail::cerealize_pnext()
        {
            NamespaceGenerator namespaceGenerator(file, "gvk::detail");
            file << std::endl;
            file << "template <typename ArchiveType>" << std::endl;
            file << "void cerealize_pnext(ArchiveType& archive, const void* const& pNext)" << std::endl;
            file << "{" << std::endl;
            file << "    if (pNext) {" << std::endl;
            generate_pnext_switch(
                file,
                manifest,
                "        ",
                "((const VkBaseInStructure*)pNext)->sType",
                "archive(true, {sType}, *(const {structureType}*)pNext);",
                "assert(false && \"Unrecognized VkStructureType\");"
            );
            file << "        } else {" << std::endl;
            file << "        archive(false);" << std::endl;
            file << "    }" << std::endl;
            file << "}" << std::endl;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Generate cereal::save() functions
        {
            file << std::endl;
            NamespaceGenerator namespaceGenerator(file, "cereal");
            for (const auto& structureItr : manifest.structures) {
                const auto& structure = structureItr.second;
                if (structure.alias.empty() && !structure_requires_custom_serialization(structure.name)) {
                    file << std::endl;
                    CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                    file << "template <typename ArchiveType>" << std::endl;
                    file << string::replace("inline void save(ArchiveType& archive, const {structureType}& obj)", "{structureType}", structure.name) << std::endl;
                    file << "{" << std::endl;
                    file << "    (void)archive;" << std::endl;
                    file << "    (void)obj;" << std::endl;
                    for (const auto& member : structure.members) {
                        CerealizeStructureMemberGenerator structureMemberGenerator;
                        auto source = structureMemberGenerator.generate(manifest, member);
                        if (!source.empty()) {
                            file << "    " << source << std::endl;
                        }
                    }
                    file << "}" << std::endl;
                }
            }
            file << std::endl;
        }
    }

private:
    class CerealizeStructureMemberGenerator final
        : public StructureMemberGenerator
    {
    protected:
        std::string generate_pnext_processor() const override final
        {
            return "gvk::detail::cerealize_pnext(archive, obj.{memberName});";
        }

        std::string generate_void_pointer_processor() const override final
        {
            return "// NOPE : Void pointer '{memberName}' not serialized";
        }

        std::string generate_function_pointer_processor() const override final
        {
            return "// NOPE : Function pointer '{memberName}' not serialized";
        }

        std::string generate_dynamic_handle_array_processor() const override final
        {
            return "gvk::detail::cerealize_dynamic_handle_array(archive, obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_dynamic_structure_array_processor() const override final
        {
            return "gvk::detail::cerealize_dynamic_array(archive, obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_dynamic_enumeration_array_processor() const override final
        {
            return "gvk::detail::cerealize_dynamic_array(archive, obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_dynamic_string_processor() const override final
        {
            return "gvk::detail::cerealize_dynamic_string(archive, obj.{memberName});";
        }

        std::string generate_dynamic_string_array_processor() const override final
        {
            return "gvk::detail::cerealize_dynamic_string_array(archive, obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_dynamic_primitive_array_processor() const override final
        {
            return "gvk::detail::cerealize_dynamic_array(archive, obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_handle_pointer_processor() const override final
        {
            return "gvk::detail::cerealize_dynamic__handle_array(archive, 1, obj.{memberName});";
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
};

} // namespace cppgen
} // namespace gvk
