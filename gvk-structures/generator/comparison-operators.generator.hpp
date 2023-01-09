
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

#include "gvk-cppgen/include.hpp"

namespace gvk {
namespace cppgen {

class ComparisonOperatorsGenerator final
{
public:
    static void generate(
        const std::string& structureCollectionName,
        const std::string& structureCollectionInclude,
        const std::vector<xml::Structure>& structures
    )
    {
        ModuleGenerator module(
            GVK_STRUCTURES_GENERATED_INCLUDE_PATH,
            GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX,
            GVK_STRUCTURES_GENERATED_SOURCE_PATH,
            structureCollectionName + "-comparison-operators"
        );
        module.header << "#include \"gvk-structures/defines.hpp\"" << std::endl;
        if (!structureCollectionInclude.empty()) {
            module.header << "#include \"" << structureCollectionInclude << "\"" << std::endl;
        }
        generate_header(module.header, structures);
        module.source << "#include \"" << GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX << structureCollectionName << "-make-tuple.hpp\"" << std::endl;
        if (structureCollectionName != "core-structures") {
            module.source << "#include \"gvk-structures/generated/core-structures-comparison-operators.hpp\"" << std::endl;
        }
        generate_source(module.source, structures);
    }

private:
    static void generate_header(FileGenerator& file, const std::vector<xml::Structure>& structures)
    {
        file << std::endl;
        for (const auto& structure : structures) {
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

    static void generate_source(FileGenerator& file, const std::vector<xml::Structure>& structures)
    {
        file << std::endl;
        for (const auto& structure : structures) {
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << string::replace(
R"(bool operator==(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) == gvk::make_tuple(rhs); }
bool operator!=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) != gvk::make_tuple(rhs); }
bool operator<(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) < gvk::make_tuple(rhs); }
bool operator>(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) > gvk::make_tuple(rhs); }
bool operator<=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) <= gvk::make_tuple(rhs); }
bool operator>=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) >= gvk::make_tuple(rhs); })", "{structureName}", structure.name) << std::endl;
        }
    }
};

} // namespace cppgen
} // namespace gvk
