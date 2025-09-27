
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

#include "gvk-cppgen/structure-comparison-operators-generator.hpp"
#include "gvk-cppgen/basic-structure-member-processor-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/module-generator.hpp"
#include "gvk-cppgen/namespace-generator.hpp"
#include "gvk-cppgen/utilities.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

void StructureComparisonOperatorsGenerator::generate(const ApiElementCollectionInfo& apiElements)
{
    ModuleGenerator module(
        apiElements.includePath,
        apiElements.includePrefix,
        apiElements.sourcePath,
        apiElements.name + "-structure-comparison-operators"
    );
    generate_header(module.header, apiElements);
    generate_source(module.source, apiElements);
}

void StructureComparisonOperatorsGenerator::generate_header(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    for (const auto& include : apiElements.headerIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << std::endl;
    for (const auto& structure : apiElements.structures) {
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

void StructureComparisonOperatorsGenerator::generate_source(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    for (const auto& include : apiElements.sourceIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"" << apiElements.includePrefix << apiElements.name << "-structure-make-tuple.hpp" << "\"" << std::endl;
    file << std::endl;
    for (const auto& structure : apiElements.structures) {
        CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
        if (apiElements.typeErasedStructures.count(structure.name)) {
            generate_type_erased_structure_source(file, apiElements, structure);
        } else {
            generate_structure_source(file, structure);
        }
    }
}

void StructureComparisonOperatorsGenerator::generate_structure_source(FileGenerator& file, const xml::Structure& structure)
{
    file << string::replace(
R"(bool operator==(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) == gvk::make_tuple(rhs); }
bool operator!=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) != gvk::make_tuple(rhs); }
bool operator<(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) < gvk::make_tuple(rhs); }
bool operator>(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) > gvk::make_tuple(rhs); }
bool operator<=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) <= gvk::make_tuple(rhs); }
bool operator>=(const {structureName}& lhs, const {structureName}& rhs) { return gvk::make_tuple(lhs) >= gvk::make_tuple(rhs); })", "{structureName}", structure.name) << std::endl;
}

void StructureComparisonOperatorsGenerator::generate_type_erased_structure_source(FileGenerator& file, const ApiElementCollectionInfo& apiElements, const xml::Structure& structure)
{
    file << string::replace("bool operator==(const {structureName}& lhs, const {structureName}& rhs)", "{structureName}", structure.name) << std::endl;
    file << "{" << std::endl;
    file << "    if (lhs.sType == rhs.sType) {" << std::endl;
    generate_type_erased_structure_switch(
        file,
        apiElements,
        "        ",
        "lhs.sType",
        "return (const {structureType}&)lhs == (const {structureType}&)rhs;",
        "assert(false && \"Unrecognized structure type\");"
    );
    file << "    }" << std::endl;
    file << "    return false;" << std::endl;
    file << "}" << std::endl;
    file << string::replace("bool operator!=(const {structureName}& lhs, const {structureName}& rhs) { return !(lhs == rhs); }", "{structureName}", structure.name) << std::endl;
    file << string::replace("bool operator<(const {structureName}& lhs, const {structureName}& rhs)", "{structureName}", structure.name) << std::endl;
    file << "{" << std::endl;
    file << "    if (lhs.sType == rhs.sType) {" << std::endl;
    generate_type_erased_structure_switch(
        file,
        apiElements,
        "        ",
        "lhs.sType",
        "return (const {structureType}&)lhs < (const {structureType}&)rhs;",
        "assert(false && \"Unrecognized structure type\");"
    );
    file << "    }" << std::endl;
    file << "    return lhs.sType < rhs.sType;" << std::endl;
    file << "}" << std::endl;
    file << string::replace("bool operator>(const {structureName}& lhs, const {structureName}& rhs) { return rhs < lhs; }", "{structureName}", structure.name) << std::endl;
    file << string::replace("bool operator<=(const {structureName}& lhs, const {structureName}& rhs) { return !(lhs > rhs); }", "{structureName}", structure.name) << std::endl;
    file << string::replace("bool operator>=(const {structureName}& lhs, const {structureName}& rhs) { return !(lhs < rhs); }", "{structureName}", structure.name) << std::endl;
}

} // namespace cppgen
} // namespace gvk
