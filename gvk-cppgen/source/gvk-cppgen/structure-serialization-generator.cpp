
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

#include "gvk-cppgen/structure-serialization-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/module-generator.hpp"
#include "gvk-cppgen/namespace-generator.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

void StructureSerializationGenerator::generate(const ApiElementCollectionInfo& apiElements)
{
    ModuleGenerator module(
        apiElements.includePath,
        apiElements.includePrefix,
        apiElements.sourcePath,
        apiElements.name + "-structure-serialization"
    );
    generate_header(module.header, apiElements);
    generate_source(module.source, apiElements);
}

void StructureSerializationGenerator::generate_header(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    for (const auto& include : apiElements.headerIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    file << std::endl;
    file << "#include <iosfwd>" << std::endl;
    file << std::endl;
    gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
    file << std::endl;
    for (const auto& structure : apiElements.structures) {
        CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
        file << string::replace("void serialize(std::ostream& ostrm, const {structureType}& obj);", "{structureType}", structure.name) << std::endl;
    }
    file << std::endl;
}

void StructureSerializationGenerator::generate_source(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    for (const auto& include : apiElements.sourceIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"" << apiElements.includePrefix << apiElements.name << "-structure-cerealization.hpp" << "\"" << std::endl;
    file << std::endl;
    file << "#include <ostream>" << std::endl;
    file << std::endl;
    gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
    for (const auto& structure : apiElements.structures) {
        file << std::endl;
        CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
        file << string::replace("void serialize(std::ostream& ostrm, const {structureType}& obj)", "{structureType}", structure.name) << std::endl;
        file << "{" << std::endl;
        file << "    cereal::BinaryOutputArchive archive(ostrm);" << std::endl;
        file << "    archive(obj);" << std::endl;
        file << "}" << std::endl;
    }
    file << std::endl;
}

} // namespace cppgen
} // namespace gvk
