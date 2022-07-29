
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

#include "cppgen-utilities.hpp"

namespace gvk {
namespace cppgen {

class GetSTypeGenerator final
{
public:
    static void generate(const gvk::xml::Manifest& manifest)
    {
        File file("get-stype.hpp");
        file << std::endl;
        file << "#include <type_traits>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "template <typename ObjectType>" << std::endl;
        file << "inline constexpr VkStructureType get_stype()" << std::endl;
        file << "{" << std::endl;
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty() && !structure.vkStructureType.empty()) {
                CompileGuardGenerator compileGuards(file, structure.compileGuards);
                file << string::replace(
                    "    if constexpr (std::is_same_v<ObjectType, {structureType}>) { return {sType}; }",
                    {
                        { "{structureType}", structure.name },
                        { "{sType}", structure.vkStructureType },
                    }
                ) << std::endl;
            }
        }
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
