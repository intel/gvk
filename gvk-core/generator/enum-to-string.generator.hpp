
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

class EnumToStringGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("enum-to-string");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/detail/to-string-utilities.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        for (const auto& enumerationItr : manifest.enumerations) {
            const auto& enumeration = enumerationItr.second;
            if (enumeration.alias.empty() && !enumeration.enumerators.empty() && !static_const_values(enumeration.name)) {
                CompileGuardGenerator compileGuardGenerator(file, enumeration.compileGuards);

                ////////////////////////////////////////////////////////////////////////////////
                // Generate gvk::print<> declarations for every enum type...
                file << string::replace("template <> void print<{enumType}>(Printer& printer, const {enumType}& value);", "{enumType}", enumeration.name) << std::endl;

                ////////////////////////////////////////////////////////////////////////////////
                // Generate gvk::print<>(flags) declarations for bitmask types...
                if (enumeration.isBitmask) {
                    file << string::replace("template <> void print<{enumType}>(Printer& printer, std::underlying_type_t<{enumType}> flags);", "{enumType}", enumeration.name) << std::endl;
                }
            }
        }
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& enumerationItr : manifest.enumerations) {
            const auto& enumeration = enumerationItr.second;
            if (enumeration.alias.empty() && !enumeration.enumerators.empty() && !static_const_values(enumeration.name)) {
                file << std::endl;
                CompileGuardGenerator compileGuardGenerator(file, enumeration.compileGuards);

                ////////////////////////////////////////////////////////////////////////////////
                // Generate gvk::print<> definitions for every enum type...
                file << string::replace("template <> void print<{enumerationType}>(Printer& printer, const {enumerationType}& value)", "{enumerationType}", enumeration.name) << std::endl;
                file << "{" << std::endl;
                file << "    switch (value) {" << std::endl;
                for (const auto& enumerator : enumeration.enumerators) {
                    if (enumerator.alias.empty()) {
                        CompileGuardGenerator enumeratorCompileGuardGenerator(file, enumerator.compileGuards);
                        file << string::replace("    case {enumIdentifier}: printer.print_enum(\"{enumIdentifier}\", value); break;", "{enumIdentifier}", enumerator.name) << std::endl;
                    }
                }
                file << string::replace("    default: printer.print_enum(\"{enumerationType}_UNKNOWN\", value);", "{enumerationType}", enumeration.name) << std::endl;
                file << "    }" << std::endl;
                file << "}" << std::endl;

                ////////////////////////////////////////////////////////////////////////////////
                // Generate gvk::print<>(flags) definitions for bitmask types...
                if (enumeration.isBitmask) {
                    file << std::endl;
                    file << string::replace("template <> void print<{enumerationType}>(Printer& printer, std::underlying_type_t<{enumerationType}> flags)", "{enumerationType}", enumeration.name) << std::endl;
                    file << "{" << std::endl;
                    file << "    std::stringstream strStrm;" << std::endl;
                    for (const auto& enumerator : enumeration.enumerators) {
                        if (enumerator.alias.empty()) {
                            CompileGuardGenerator enumeratorCompileGuardGenerator(file, enumerator.compileGuards);
                            file << string::replace("    if (flags & {enumIdentifier}) strStrm << \"{enumIdentifier}|\";", "{enumIdentifier}", enumerator.name) << std::endl;
                        }
                    }
                    file << "    auto str = strStrm.str();" << std::endl;
                    file << "    if (!str.empty()) {" << std::endl;
                    file << "        str.pop_back();" << std::endl;
                    file << "    }" << std::endl;
                    file << string::replace("    printer.print_enum(str.c_str(), ({enumType})flags);", "{enumType}", enumeration.name) << std::endl;
                    file << "}" << std::endl;
                }
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
