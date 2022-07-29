
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

class ComparisonOperatorsGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("comparison-operators");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/detail/comparison-operator-utilities.hpp\"" << std::endl;
        file << std::endl;
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace(
R"(bool operator==(const {structureName}& lhs, const {structureName}& rhs);
bool operator!=(const {structureName}& lhs, const {structureName}& rhs);
bool operator<(const {structureName}& lhs, const {structureName}& rhs);
bool operator>(const {structureName}& lhs, const {structureName}& rhs);
bool operator<=(const {structureName}& lhs, const {structureName}& rhs);
bool operator>=(const {structureName}& lhs, const {structureName}& rhs);)", "{structureName}", structure.name) << std::endl;
            }
        }
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/generated/make-tuple.hpp\"" << std::endl;
        file << std::endl;
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace(
R"(bool operator==(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) == gvk::make_tuple(rhs); };
bool operator!=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) != gvk::make_tuple(rhs); };
bool operator<(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) < gvk::make_tuple(rhs); };
bool operator>(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) > gvk::make_tuple(rhs); };
bool operator<=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) <= gvk::make_tuple(rhs); };
bool operator>=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) >= gvk::make_tuple(rhs); };)", "{structureName}", structure.name) << std::endl;
            }
        }
    }
};

} // namespace cppgen
} // namespace gvk
