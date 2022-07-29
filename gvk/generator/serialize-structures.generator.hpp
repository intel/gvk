
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

class SerializeStructuresGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("serialize-structures");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        file << "#include <iosfwd>" << std::endl;
        file << std::endl;
        gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace("void serialize(std::ostream& ostrm, const {structureType}& obj);", "{structureType}", structure.name) << std::endl;
            }
        }
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/generated/cerealize-structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <ostream>" << std::endl;
        file << std::endl;
        gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                file << std::endl;
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace("void serialize(std::ostream& ostrm, const {structureType}& obj)", "{structureType}", structure.name) << std::endl;
                file << "{" << std::endl;
                file << "    cereal::BinaryOutputArchive archive(ostrm);" << std::endl;
                file << "    archive(obj);" << std::endl;
                file << "}" << std::endl;
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
