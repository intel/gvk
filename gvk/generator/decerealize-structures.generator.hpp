
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

namespace gvk {
namespace cppgen {

class DecerealizeStructuresGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        File file("decerealize-structures.hpp");
        file << "#include \"gvk/detail/cerealization-utilities.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <cstdlib>" << std::endl;
        file << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::detail::decerealize_pnext()
        {
            NamespaceGenerator namespaceGenerator(file, "gvk::detail");
            file << std::endl;
            file << "template <typename ArchiveType>" << std::endl;
            file << "void* decerealize_pnext(ArchiveType& archive)" << std::endl;
            file << "{" << std::endl;
            file << "    void* pNext = nullptr;" << std::endl;
            file << "    bool serialized = false;" << std::endl;
            file << "    archive(serialized);" << std::endl;
            file << "    if (serialized) {" << std::endl;
            file << "        VkStructureType sType { };" << std::endl;
            file << "        archive(sType);" << std::endl;
            generate_pnext_switch(
                file,
                manifest,
                "        ",
                "sType",
                "pNext = malloc(sizeof({structureType})); archive(*({structureType}*)pNext);",
                "assert(false && \"Unrecognized VkStructureType\");"
            );
            file << "    }" << std::endl;
            file << "    return pNext;" << std::endl;
            file << "}" << std::endl;
            file << std::endl;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Generate cereal::load() functions
        {
            file << std::endl;
            NamespaceGenerator namespaceGenerator(file, "cereal");
            for (const auto& structureItr : manifest.structures) {
                const auto& structure = structureItr.second;
                if (structure.alias.empty() && !structure_requires_custom_serialization(structure.name)) {
                    file << std::endl;
                    CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                    file << "template <typename ArchiveType>" << std::endl;
                    file << string::replace("inline void load(ArchiveType& archive, {structureType}& obj)", "{structureType}", structure.name) << std::endl;
                    file << "{" << std::endl;
                    file << "    (void)archive;" << std::endl;
                    file << "    (void)obj;" << std::endl;
                    for (const auto& member : structure.members) {
                        DecerealizeStructureMemberGenerator structureMemberGenerator;
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
    class DecerealizeStructureMemberGenerator final
        : public StructureMemberGenerator
    {
    protected:
        std::string generate_pnext_processor() const override final
        {
            return "obj.{memberName} = ({memberType})gvk::detail::decerealize_pnext(archive);";
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
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_handle_array<{memberUnqualifiedType}>(archive);";
        }

        std::string generate_dynamic_structure_array_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_array<{memberUnqualifiedType}>(archive);";
        }

        std::string generate_dynamic_enumeration_array_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_array<{memberUnqualifiedType}>(archive);";
        }

        std::string generate_dynamic_string_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_string(archive);";
        }

        std::string generate_dynamic_string_array_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_string_array(archive);";
        }

        std::string generate_dynamic_primitive_array_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_array<{memberUnqualifiedType}>(archive);";
        }

        std::string generate_handle_pointer_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_handle_array<{memberUnqualifiedType}>(archive);";
        }

        std::string generate_structure_pointer_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_array<{memberUnqualifiedType}>(archive);";
        }

        std::string generate_enumeration_pointer_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_array<{memberUnqualifiedType}>(archive);";
        }

        std::string generate_primitive_pointer_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_dynamic_array<{memberUnqualifiedType}>(archive);";
        }

        std::string generate_static_handle_array_processor() const override final
        {
            return "gvk::detail::decerealize_static_handle_array<{memberLength}>(archive, obj.{memberName});";
        }

        std::string generate_static_structure_array_processor() const override final
        {
            return "gvk::detail::decerealize_static_array<{memberLength}>(archive, obj.{memberName});";
        }

        std::string generate_static_enumeration_array_processor() const override final
        {
            return "gvk::detail::decerealize_static_array<{memberLength}>(archive, obj.{memberName});";
        }

        std::string generate_static_string_processor() const override final
        {
            return "gvk::detail::decerealize_static_array<{memberLength}>(archive, obj.{memberName});";
        }

        std::string generate_static_primitive_array_processor() const override final
        {
            return "gvk::detail::decerealize_static_array<{memberLength}>(archive, obj.{memberName});";
        }

        std::string generate_handle_processor() const override final
        {
            return "obj.{memberName} = gvk::detail::decerealize_handle<{memberUnqualifiedType}>(archive);";
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
