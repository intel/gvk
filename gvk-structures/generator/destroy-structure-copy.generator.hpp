
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

#include "gvk-cppgen/include.hpp"

namespace gvk {
namespace cppgen {

class DestroyStructureCopyGenerator final
{
public:
    static void generate(
        const std::string& structureCollectionName,
        const std::string& structureCollectionInclude,
        const xml::Manifest& manifest,
        const std::vector<xml::Structure>& structures
    )
    {
        ModuleGenerator module(
            GVK_STRUCTURES_GENERATED_INCLUDE_PATH,
            GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX,
            GVK_STRUCTURES_GENERATED_SOURCE_PATH,
            structureCollectionName + "-destroy-copy"
        );
        module.header << "#include \"gvk-structures/defines.hpp\"" << std::endl;
        if (!structureCollectionInclude.empty()) {
            module.header << "#include \"" << structureCollectionInclude << "\"" << std::endl;
        }
        generate_header(module.header, structures);
        if (structureCollectionName != "core-structures") {
            module.source << "#include \"gvk-structures/generated/core-structures-destroy-copy.hpp\"" << std::endl;
        }
        generate_source(module.source, manifest, structures);
    }

private:
    static void generate_header(FileGenerator& file, const std::vector<xml::Structure>& structures)
    {
        file << "#include \"gvk-structures/detail/copy-utilities.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;
        for (const auto& structure : structures) {
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << string::replace("template <> void destroy_structure_copy<{structureType}>(const {structureType}& obj, const VkAllocationCallbacks* pAllocator);", "{structureType}", structure.name) << std::endl;
        }
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest, const std::vector<xml::Structure>& structures)
    {
        file << "#include \"gvk-structures/detail/get-count.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        for (const auto& structure : structures) {
            if (!structure_requires_custom_implementation(structure.name)) {
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

    class StructureMemberDestroyCopyGenerator final
        : public BasicStructureMemberProcessorGenerator
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
            return "destroy_dynamic_array_copy(gvk::detail::get_count(obj.{memberLength}), obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_structure_array_processor() const override final
        {
            return "destroy_dynamic_array_copy(gvk::detail::get_count(obj.{memberLength}), obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_enumeration_array_processor() const override final
        {
            return "destroy_dynamic_array_copy(gvk::detail::get_count(obj.{memberLength}), obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_string_processor() const override final
        {
            return "destroy_dynamic_string_copy(obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_string_array_processor() const override final
        {
            return "destroy_dynamic_string_array_copy(gvk::detail::get_count(obj.{memberLength}), obj.{memberName}, pAllocator);";
        }

        std::string generate_dynamic_primitive_array_processor() const override final
        {
            return "destroy_dynamic_array_copy(gvk::detail::get_count(obj.{memberLength}), obj.{memberName}, pAllocator);";
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
};

} // namespace cppgen
} // namespace gvk
