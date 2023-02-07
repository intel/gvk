
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

#include "gvk-cppgen/structure-to-string-generator.hpp"
#include "gvk-cppgen/basic-structure-member-processor-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/module-generator.hpp"
#include "gvk-cppgen/namespace-generator.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

class StructureMemberToStringGenerator final
    : public BasicStructureMemberProcessorGenerator
{
protected:
    std::string generate_pnext_processor() const override final
    {
        return "detail::print_pnext(printer, obj.pNext);";
    }

    std::string generate_void_pointer_processor() const override final
    {
        return "printer.print_field(\"{memberName}\", to_hex_string(obj.{memberName}));";
    }

    std::string generate_function_pointer_processor() const override final
    {
        return "printer.print_field(\"{memberName}\", to_hex_string(obj.{memberName}));";
    }

    std::string generate_dynamic_handle_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_dynamic_structure_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_dynamic_enumeration_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_dynamic_string_processor() const override final
    {
        return "printer.print_field(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_dynamic_string_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_dynamic_primitive_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", gvk::detail::get_count(obj.{memberLength}), obj.{memberName});";
    }

    std::string generate_handle_pointer_processor() const override final
    {
        return "printer.print_pointer(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_structure_pointer_processor() const override final
    {
        return "printer.print_pointer(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_enumeration_pointer_processor() const override final
    {
        return "printer.print_pointer(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_primitive_pointer_processor() const override final
    {
        return "printer.print_pointer(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_static_handle_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", {memberLength}, obj.{memberName});";
    }

    std::string generate_static_structure_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", {memberLength}, obj.{memberName});";
    }

    std::string generate_static_enumeration_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", {memberLength}, obj.{memberName});";
    }

    std::string generate_static_string_processor() const override final
    {
        return "printer.print_field(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_static_primitive_array_processor() const override final
    {
        return "printer.print_array(\"{memberName}\", {memberLength}, obj.{memberName});";
    }

    std::string generate_handle_processor() const override final
    {
        return "printer.print_field(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_structure_processor() const override final
    {
        return "printer.print_field(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_enumeration_processor() const override final
    {
        return "printer.print_field(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_flags_processor() const override final
    {
        return "printer.print_flags<{memberFlagBitsType}>(\"{memberName}\", obj.{memberName});";
    }

    std::string generate_primitive_processor() const override final
    {
        return "printer.print_field(\"{memberName}\", obj.{memberName});";
    }
};

void StructureToStringGeneratorEx::generate(const xml::Manifest& manifest, const ApiElementCollectionInfo& apiElements)
{
    ModuleGenerator module(
        apiElements.includePath,
        apiElements.includePrefix,
        apiElements.sourcePath,
        apiElements.name + "-structure-to-string"
    );
    generate_header(module.header, apiElements);
    generate_source(module.source, manifest, apiElements);
}

void StructureToStringGeneratorEx::generate_header(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    for (const auto& include : apiElements.headerIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"gvk-structures/detail/to-string-utilities.hpp\"" << std::endl;
    file << std::endl;
    NamespaceGenerator namespaceGenerator(file, "gvk");
    file << std::endl;
    for (const auto& structure : apiElements.structures) {
        CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
        file << string::replace("template <> void print<{structureType}>(Printer& printer, const {structureType}& obj);", "{structureType}", structure.name) << std::endl;
    }
    file << std::endl;
}

void StructureToStringGeneratorEx::generate_source(FileGenerator& file, const xml::Manifest& manifest, const ApiElementCollectionInfo& apiElements)
{
    for (const auto& include : apiElements.sourceIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"" << apiElements.includePrefix << apiElements.name << "-enumerations-to-string.hpp" << "\"" << std::endl;
    file << "#include \"gvk-structures/generated/handle-to-string.hpp\"" << std::endl;
    file << "#include \"gvk-structures/detail/get-count.hpp\"" << std::endl;
    file << std::endl;
    NamespaceGenerator namespaceGenerator(file, "gvk");
    for (const auto& structure : apiElements.structures) {
        if (!apiElements.manuallyImplemented.count(structure.name)) {
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << string::replace("template <> void print<{structureType}>(Printer& printer, const {structureType}& obj)", "{structureType}", structure.name) << std::endl;
            file << "{" << std::endl;
            file << "    printer.print_object(" << std::endl;
            file << "        [&]()" << std::endl;
            file << "        {" << std::endl;
            for (const auto& member : structure.members) {
                file << "            ";
                StructureMemberToStringGenerator structureMemberToStringGenerator;
                file << structureMemberToStringGenerator.generate(manifest, member) << std::endl;
            }
            file << "        }" << std::endl;
            file << "    );" << std::endl;
            file << "}" << std::endl;
        }
    }
    file << std::endl;
}

} // namespace cppgen
} // namespace gvk
