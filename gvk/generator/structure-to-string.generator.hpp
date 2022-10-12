
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

class StructureToStringGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("structure-to-string");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    class StructureMemberToStringGenerator final
        : public StructureMemberGenerator
    {
    protected:
        std::string generate_pnext_processor() const override final
        {
            return "print_pnext(printer, obj.pNext);";
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
            return "printer.print_array(\"{memberName}\", obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_dynamic_structure_array_processor() const override final
        {
            return "printer.print_array(\"{memberName}\", obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_dynamic_enumeration_array_processor() const override final
        {
            return "printer.print_array(\"{memberName}\", obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_dynamic_string_processor() const override final
        {
            return "printer.print_field(\"{memberName}\", obj.{memberName});";
        }

        std::string generate_dynamic_string_array_processor() const override final
        {
            return "printer.print_array(\"{memberName}\", obj.{memberLength}, obj.{memberName});";
        }

        std::string generate_dynamic_primitive_array_processor() const override final
        {
            return "printer.print_array(\"{memberName}\", obj.{memberLength}, obj.{memberName});";
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

    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/detail/to-string-utilities.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::print_pnext() declaration...
        file << "void print_pnext(Printer& printer, const void* pNext);" << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::print<>() declarations...
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace("template <> void print<{structureType}>(Printer& printer, const {structureType}& obj);", "{structureType}", structure.name) << std::endl;
            }
        }
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/generated/enum-to-string.hpp\"" << std::endl;
        file << "#include \"gvk/generated/handle-to-string.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::print_pnext() definition...
        file << "void print_pnext(Printer& printer, const void* pNext)" << std::endl;
        file << "{" << std::endl;
        file << "    if (pNext) {" << std::endl;
        generate_pnext_switch(
            file,
            manifest,
            "        ",
            "((const VkBaseInStructure*)pNext)->sType",
            "printer.print_field(\"pNext\", *(const {structureType}*)pNext);",
            "assert(false && \"Unrecognized VkStructureType\");"
        );
        file << "    } else {" << std::endl;
        file << "        printer.print_pointer(\"pNext\", (const VkBaseInStructure*)pNext);" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;

        ////////////////////////////////////////////////////////////////////////////////
        // Generate gvk::print<>() definitions...
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty() && !structure_requires_custom_implementation(structure.name)) {
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
};

} // namespace cppgen
} // namespace gvk
