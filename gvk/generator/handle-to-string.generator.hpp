
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

class HandleToStringGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("handle-to-string");
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
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << string::replace("template <> void print<{handleType}>(Printer& printer, const {handleType}& handle);", "{handleType}", handle.name) << std::endl;
            }
        }
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                file << std::endl;
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << string::replace("template <> void print<{handleType}>(Printer& printer, const {handleType}& handle)", "{handleType}", handle.name) << std::endl;
                file << "{" << std::endl;
                file << R"(    printer.mOstrm << "\"" << (handle ? to_hex_string(handle) : "VK_NULL_HANDLE") << "\"";)" << std::endl;
                file << "}" << std::endl;
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
