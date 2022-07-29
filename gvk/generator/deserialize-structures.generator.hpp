
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

class DeserializeStructuresGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("deserialize-structures");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <iosfwd>" << std::endl;
        file << std::endl;
        gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace("void deserialize(std::istream& istrm, const VkAllocationCallbacks* pAllocator, Auto<{structureType}>& obj);", "{structureType}", structure.name) << std::endl;
            }
        }
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/generated/decerealize-structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <istream>" << std::endl;
        file << std::endl;
        gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "thread_local const VkAllocationCallbacks* detail::tlpDecerealizationAllocator;" << std::endl;
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                file << std::endl;
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace(
R"(void deserialize(std::istream& istrm, const VkAllocationCallbacks* pAllocator, Auto<{structureType}>& obj)
{
    assert(!pAllocator && "TODO : VkAllocationCallbacks need to be hooked up in gvk::Auto<>");
    obj.reset();
    cereal::BinaryInputArchive archive(istrm);
    detail::tlpDecerealizationAllocator = detail::validate_allocation_callbacks(pAllocator);
    archive(const_cast<{structureType}&>(*obj));
}
)", "{structureType}", structure.name);
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
