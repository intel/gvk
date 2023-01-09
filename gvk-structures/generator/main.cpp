
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

#include "gvk-xml/manifest.hpp"
#include "cerealize-pnext.generator.hpp"
#include "command-structures.generator.hpp"
#include "create-pnext-copy.generator.hpp"
#include "decerealize-pnext.generator.hpp"
#include "destroy-pnext-copy.generator.hpp"
#include "enum-to-string.generator.hpp"
#include "handle-to-string.generator.hpp"
#include "pnext-to-string.generator.hpp"
#include "pnext-tuple-element-wrapper.generator.hpp"
#include "structure-utilities.generator.hpp"

int main(int, const char*[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        auto commandStructures = gvk::cppgen::CommandStructuresGenerator::get_command_structures(manifest);
        std::vector<gvk::xml::Structure> coreStructures;
        for (const auto& structureItr : manifest.structures) {
            if (structureItr.second.alias.empty()) {
                coreStructures.push_back(structureItr.second);
            }
        }
        auto commandStructureTypeEnumeration = gvk::cppgen::CommandStructuresGenerator::get_structure_type_enumeration(manifest);
        std::vector<gvk::xml::Enumeration> coreEnumerations;
        for (const auto& enumerationItr : manifest.enumerations) {
            if (enumerationItr.second.alias.empty()) {
                coreEnumerations.push_back(enumerationItr.second);
            }
        }
        gvk::cppgen::CerealizePNextGenerator::generate(manifest);
        gvk::cppgen::CommandStructuresGenerator::generate(manifest);
        gvk::cppgen::CreatePNextCopyGenerator::generate(manifest);
        gvk::cppgen::DecerealizePNextGenerator::generate(manifest);
        gvk::cppgen::DestroyPNextCopyGenerator::generate(manifest);
        gvk::cppgen::HandleToStringGenerator::generate(manifest);
        gvk::cppgen::PNextToStringGenerator::generate(manifest);
        gvk::cppgen::PNextTupleElementWrapperGenerator::generate(manifest);
        gvk::cppgen::StructureUtilitiesGenerator::generate("command-structures", "gvk-structures/generated/command-structures.h", manifest, commandStructures);
        gvk::cppgen::StructureUtilitiesGenerator::generate("core-structures", std::string(), manifest, coreStructures);
        gvk::cppgen::EnumToStringGenerator::generate("command-structure-type", "gvk-structures/generated/command-structures.h", { commandStructureTypeEnumeration });
        gvk::cppgen::EnumToStringGenerator::generate("core-enumerations", std::string(), coreEnumerations);
    }
    return 0;
}
