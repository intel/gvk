
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

#include "gvk-cppgen/structure-deserialization-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/module-generator.hpp"
#include "gvk-cppgen/namespace-generator.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

void StructureDeserializationGenerator::generate(const ApiElementCollectionInfo& apiElements)
{
    ModuleGenerator module(
        apiElements.includePath,
        apiElements.includePrefix,
        apiElements.sourcePath,
        apiElements.name + "-structure-deserialization"
    );
    generate_header(module.header, apiElements);
    generate_source(module.source, apiElements);
}

void StructureDeserializationGenerator::generate_header(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    file << "#include \"gvk-defines.hpp\"" << std::endl;
    for (const auto& include : apiElements.headerIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"gvk-structures/auto.hpp\"" << std::endl;
    file << std::endl;
    file << "#include <iosfwd>" << std::endl;
    file << std::endl;
    gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
    file << std::endl;
    for (const auto& structure : apiElements.structures) {
        CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
        file << string::replace("void deserialize(std::istream& istrm, const VkAllocationCallbacks* pAllocator, Auto<{structureType}>& obj);", "{structureType}", structure.name) << std::endl;
    }
    file << std::endl;
}

void StructureDeserializationGenerator::generate_source(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    for (const auto& include : apiElements.sourceIncludes) {
        file << "#include \"" << include << "\"" << std::endl;
    }
    file << "#include \"" << apiElements.includePrefix << apiElements.name << "-structure-decerealization.hpp" << "\"" << std::endl;
    file << "#include \"" << apiElements.includePrefix << apiElements.name << "-structure-create-copy.hpp" << "\"" << std::endl;
    file << "#include \"" << apiElements.includePrefix << apiElements.name << "-structure-destroy-copy.hpp" << "\"" << std::endl;
    file << std::endl;
    file << "#include <istream>" << std::endl;
    file << std::endl;
    gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
    for (const auto& structure : apiElements.structures) {
        if (structure.alias.empty()) {
            file << std::endl;
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            std::stringstream strStrm;
            strStrm << "void deserialize(std::istream& istrm, const VkAllocationCallbacks* pAllocator, Auto<{structureType}>& obj)" << std::endl;
            strStrm << "{"                                                                                                          << std::endl;
            strStrm << "    assert(!pAllocator && \"TODO : VkAllocationCallbacks need to be hooked up in gvk::Auto <>\");"          << std::endl;
            strStrm << "    obj.reset();"                                                                                           << std::endl;
            strStrm << "    cereal::BinaryInputArchive archive(istrm);"                                                             << std::endl;
            strStrm << "    detail::tlpDecerealizationAllocator = detail::validate_allocation_callbacks(pAllocator);"               << std::endl;
            strStrm << "    archive(const_cast<{structureType}&>(*obj));"                                                           << std::endl;
            strStrm << "}"                                                                                                          << std::endl;
            file << string::replace(strStrm.str(), "{structureType}", structure.name);
        }
    }
    file << std::endl;
}

} // namespace cppgen
} // namespace gvk
