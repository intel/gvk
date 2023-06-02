
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

#include "gvk-cppgen/structure-enumerate-handles-generator.hpp"
#include "gvk-cppgen/basic-structure-member-processor-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/module-generator.hpp"
#include "gvk-cppgen/namespace-generator.hpp"
#include "gvk-cppgen/utilities.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

class StructureMemberEnumerateHandlesGenerator final
    : public BasicStructureMemberProcessorGenerator
{
protected:
    inline std::string generate_pnext_processor() const override final
    {
        return "enumerate_pnext_handles(obj.{memberName}, callback);";
    }

    inline std::string generate_dynamic_handle_array_processor() const override final
    {
        return "enumerate_dynamic_handle_array(gvk::detail::get_count(obj.{memberLength}), obj.{memberName}, callback);";
    }

    inline std::string generate_dynamic_structure_array_processor() const override final
    {
        return "enumerate_dynamic_structure_array_handles(gvk::detail::get_count(obj.{memberLength}), obj.{memberName}, callback);";
    }

    inline std::string generate_handle_pointer_processor() const override final
    {
        return "enumerate_dynamic_handle_array(1, obj.{memberName}, callback);";
    }

    inline std::string generate_structure_pointer_processor() const override final
    {
        return "enumerate_dynamic_structure_array_handles(1, obj.{memberName}, callback);";
    }

    inline std::string generate_static_handle_array_processor() const override final
    {
        return "enumerate_static_handle_array<{memberLength}>(obj.{memberName}, callback);";
    }

    inline std::string generate_static_structure_array_processor() const override final
    {
        return "enumerate_static_structure_array_handles<{memberLength}>(obj.{memberName}, callback);";
    }

    inline std::string generate_handle_processor() const override final
    {
        return "enumerate_handle(obj.{memberName}, callback);";
    }

    inline std::string generate_structure_processor() const override final
    {
        return "enumerate_structure_handles(obj.{memberName}, callback);";
    }
};

void StructureEnumerateHandlesGenerator::generate(const xml::Manifest& manifest, const ApiElementCollectionInfo& apiElements)
{
    ModuleGenerator module(
        apiElements.includePath,
        apiElements.includePrefix,
        apiElements.sourcePath,
        apiElements.name + "-structure-enumerate-handles"
    );
    generate_header(module.header, apiElements);
    generate_source(module.source, manifest, apiElements);
}

void StructureEnumerateHandlesGenerator::generate_header(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    for (const auto& include : apiElements.headerIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"gvk-structures/detail/handle-enumeration-utilities.hpp\"" << std::endl;
    file << std::endl;
    NamespaceGenerator namespaceGenerator(file, "gvk::detail");
    file << std::endl;
    for (const auto& structure : apiElements.structures) {
        CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
        file << string::replace("template <> void enumerate_structure_handles<{structureType}>(const {structureType}& obj, EnumerateHandlesCallback callback);", "{structureType}", structure.name) << std::endl;
    }
    file << std::endl;
}

void StructureEnumerateHandlesGenerator::generate_source(FileGenerator& file, const xml::Manifest& manifest, const ApiElementCollectionInfo& apiElements)
{
    for (const auto& include : apiElements.sourceIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    if (apiElements.name != "core") {
        file << "#include \"gvk-structures/generated/core-structure-enumerate-handles.hpp\"" << std::endl;
    }
    file << "#include \"gvk-structures/detail/get-count.hpp\"" << std::endl;
    file << std::endl;
    NamespaceGenerator namespaceGenerator(file, "gvk::detail");
    for (const auto& structure : apiElements.structures) {
        if (!apiElements.manuallyImplemented.count(structure.name)) {
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << string::replace("template <> void enumerate_structure_handles<{structureType}>(const {structureType}& obj, EnumerateHandlesCallback callback)", "{structureType}", structure.name) << std::endl;
            file << "{" << std::endl;
            file << "    (void)obj;" << std::endl;
            file << "    (void)callback;" << std::endl;
            for (size_t i = 0; i < structure.members.size(); ++i) {
                const auto& member = structure.members[i];
                CompileGuardGenerator memberCompileGuardGenerator(file, get_inner_scope_compile_guards(structure.compileGuards, member.compileGuards));
                auto source = StructureMemberEnumerateHandlesGenerator().generate(manifest, member);
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
