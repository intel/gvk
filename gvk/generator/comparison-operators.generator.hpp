
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
