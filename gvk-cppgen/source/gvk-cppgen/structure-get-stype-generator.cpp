
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

#include "gvk-cppgen/structure-get-stype-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/file-generator.hpp"
#include "gvk-cppgen/namespace-generator.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

void StructureGetSTypeGenerator::generate(const ApiElementCollectionInfo& apiElements)
{
    FileGenerator file(apiElements.includePath + "/" + apiElements.name + "-structure-get-stype.hpp");
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    for (const auto& include : apiElements.headerIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"gvk-structures/detail/get-stype-utilities.hpp\"" << std::endl;
    file << std::endl;
    NamespaceGenerator namespaceGenerator(file, "gvk");
    file << std::endl;
    for (const auto& structure : apiElements.structures) {
        if (!structure.vkStructureType.empty()) {
            CompileGuardGenerator compileGuards(file, structure.compileGuards);
            file << string::replace(
                "template <> inline constexpr auto get_stype<{structureType}>() { return {sType}; }", {
                    { "{structureType}", structure.name },
                    { "{sType}", structure.vkStructureType },
            }) << std::endl;
        }
    }
    file << std::endl;
}

} // namespace cppgen
} // namespace gvk
