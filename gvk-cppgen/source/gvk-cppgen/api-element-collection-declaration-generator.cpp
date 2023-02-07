
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

#include "gvk-cppgen/api-element-collection-declaration-generator.hpp"
#include "gvk-cppgen/compile-guard-generator.hpp"
#include "gvk-cppgen/file-generator.hpp"
#include "gvk-cppgen/header-guard-generator.hpp"
#include "gvk-string.hpp"

namespace gvk {
namespace cppgen {

void ApiElementCollectionDeclarationGenerator::generate(const ApiElementCollectionInfo& apiElements)
{
    FileGenerator file(apiElements.includePath + "/" + apiElements.name + ".h");
    file << std::endl;
    HeaderGuardGenerator headerGuardGenerator(file, apiElements.headerGuard);
    file << std::endl;
    file << "#include \"vulkan/vulkan.h\"" << std::endl;
    generate_enumeration_declarations(file, apiElements);
    generate_structure_declarations(file, apiElements);
    file << std::endl;
}

void ApiElementCollectionDeclarationGenerator::generate_enumeration_declarations(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    for (const auto& enumeration : apiElements.enumerations) {
        file << std::endl;
        file << "typedef enum " << enumeration.name << " {" << std::endl;
        for (const auto& enumerator : enumeration.enumerators) {
            file << "    " << enumerator.name << " = " << enumerator.value << "," << std::endl;
        }
        file << "    ";
        for (const auto& token : string::split_camel_case(enumeration.name)) {
            file << string::to_upper(token) << "_";
        }
        file << "MAX_ENUM = 0x7FFFFFFF" << std::endl;
        file << "} " << enumeration.name << ";" << std::endl;
    }
}

void ApiElementCollectionDeclarationGenerator::generate_structure_declarations(FileGenerator& file, const ApiElementCollectionInfo& apiElements)
{
    for (const auto& structure : apiElements.structures) {
        file << std::endl;
        CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
        file << "typedef struct " << structure.name << " {" << std::endl;
        for (const auto& member : structure.members) {
            if (member.flags & xml::Static && member.flags & xml::Array) {
                file << "    " << member.unqualifiedType << " " << member.name << member.length << ";" << std::endl;
            } else {
                file << "    " << member.type << " " << member.name << ";" << std::endl;
            }
        }
        file << "} " << structure.name << ";" << std::endl;
    }
}

} // namespace cppgen
} // namespace gvk
