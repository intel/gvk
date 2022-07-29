
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

class DestroyStructureCopyGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("destroy-structure-copy");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    class StructureMemberDestroyCopyGenerator final
        : public StructureMemberGenerator
    {
    protected:
        std::string generate_pnext_processor() const override final
        {
            return "destroy_pnext_copy(obj.pNext, pAllocator);";
        }

        std::string generate_void_pointer_processor() const override final
        {
            return std::string();
        }

        std::string generate_function_pointer_processor() const override final
        {
            return std::string();
        }

        std::string generate_dynamic_handle_array_processor() const override final
        {
            return "destroy_dynamic_array_copy(obj.{memberLength}, obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_structure_array_processor() const override final
        {
            return "destroy_dynamic_array_copy(obj.{memberLength}, obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_enumeration_array_processor() const override final
        {
            return "destroy_dynamic_array_copy(obj.{memberLength}, obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_string_processor() const override final
        {
            return "destroy_dynamic_string_copy(obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_string_array_processor() const override final
        {
            return "destroy_dynamic_string_array_copy(obj.{memberLength}, obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_primitive_array_processor() const override final
        {
            return "destroy_dynamic_array_copy(obj.{memberLength}, obj.{memberName}, pAllocator);";
        }

        std::string generate_handle_pointer_processor() const override final
        {
            return "destroy_dynamic_array_copy(1, obj.{memberName}, pAllocator);";
        }

        std::string generate_structure_pointer_processor() const override final
        {
            return "destroy_dynamic_array_copy(1, obj.{memberName}, pAllocator);";
        }

        std::string generate_enumeration_pointer_processor() const override final
        {
            return "destroy_dynamic_array_copy(1, obj.{memberName}, pAllocator);";
        }

        std::string generate_primitive_pointer_processor() const override final
        {
            return "destroy_dynamic_array_copy(1, obj.{memberName}, pAllocator);";
        }

        std::string generate_static_handle_array_processor() const override final
        {
            return "destroy_static_array_copy<{memberLength}>(obj.{memberName}, pAllocator);";
        }

        std::string generate_static_structure_array_processor() const override final
        {
            return "destroy_static_array_copy<{memberLength}>(obj.{memberName}, pAllocator);";
        }

        std::string generate_static_enumeration_array_processor() const override final
        {
            return "destroy_static_array_copy<{memberLength}>(obj.{memberName}, pAllocator);";
        }

        std::string generate_static_string_processor() const override final
        {
            return std::string();
        }

        std::string generate_static_primitive_array_processor() const override final
        {
            return "destroy_static_array_copy<{memberLength}>(obj.{memberName}, pAllocator);";
        }

        std::string generate_handle_processor() const override final
        {
            return std::string();
        }

        std::string generate_structure_processor() const override final
        {
            return "destroy_structure_copy(obj.{memberName}, pAllocator);";
        }

        std::string generate_enumeration_processor() const override final
        {
            return std::string();
        }

        std::string generate_flags_processor() const override final
        {
            return std::string();
        }

        std::string generate_primitive_processor() const override final
        {
            return std::string();
        }
    };

    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/detail/structure-copy-utilities.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::detail::destroy_pnext_copy() declaration...
        file << "void destroy_pnext_copy(const void* pNext, const VkAllocationCallbacks* pAllocator);" << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::detail::destroy_structure_copy<> declarations...
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace("template <> void destroy_structure_copy<{structureType}>(const {structureType}& obj, const VkAllocationCallbacks* pAllocator);", "{structureType}", structure.name) << std::endl;
            }
        }
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::detail::destroy_pnext_copy() definition...
        file << "void destroy_pnext_copy(const void* pNext, const VkAllocationCallbacks* pAllocator)" << std::endl;
        file << "{" << std::endl;
        file << "    if (pNext) {" << std::endl;
        generate_pnext_switch(
            file,
            manifest,
            "        ",
            "((const VkBaseInStructure*)pNext)->sType",
            "destroy_dynamic_array_copy(1, (const {structureType}*)pNext, pAllocator);",
            "assert(false && \"Unrecognized VkStructureType\");"
        );
        file << "    }" << std::endl;
        file << "}" << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::detail::destroy_structure_copy<> defintitions...
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty() && !structure_requires_custom_implementation(structure.name)) {
                file << std::endl;
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace("template <> void destroy_structure_copy<{structureType}>(const {structureType}& obj, const VkAllocationCallbacks* pAllocator)", "{structureType}", structure.name) << std::endl;
                file << "{" << std::endl;
                file << "    (void)obj;" << std::endl;
                file << "    (void)pAllocator;" << std::endl;
                for (const auto& member : structure.members) {
                    StructureMemberDestroyCopyGenerator structureMemberGenerator;
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
};

} // namespace cppgen
} // namespace gvk
