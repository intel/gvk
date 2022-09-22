
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

#include "gvk/xml/manifest.hpp"
#include "cerealize-structures.generator.hpp"
#include "comparison-operators.generator.hpp"
#include "create-structure-copy.generator.hpp"
#include "decerealize-structures.generator.hpp"
#include "deserialize-structures.generator.hpp"
#include "destroy-structure-copy.generator.hpp"
#include "dispatch-table.generator.hpp"
#include "enum-to-string.generator.hpp"
#include "format-utilities.generator.hpp"
#include "forward-declarations.generator.hpp"
#include "get-stype.generator.hpp"
#include "handle-to-string.generator.hpp"
#include "handles.generator.hpp"
#include "make-tuple.generator.hpp"
#include "serialize-structures.generator.hpp"
#include "structure-to-string.generator.hpp"

#include "tinyxml2.h"

int main(int, const char*[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        gvk::cppgen::CerealizeStructuresGenerator::generate(manifest);
        gvk::cppgen::ComparisonOperatorsGenerator::generate(manifest);
        gvk::cppgen::CreateStructureCopyGenerator::generate(manifest);
        gvk::cppgen::DecerealizeStructuresGenerator::generate(manifest);
        gvk::cppgen::DeserializeStructuresGenerator::generate(manifest);
        gvk::cppgen::DestroyStructureCopyGenerator::generate(manifest);
        gvk::cppgen::DispatchTableGenerator::generate(manifest);
        gvk::cppgen::EnumToStringGenerator::generate(manifest);
        gvk::cppgen::FormatUtilitiesGenerator::generate(manifest);
        gvk::cppgen::ForwardDeclarationsGenerator::generate(manifest);
        gvk::cppgen::GetSTypeGenerator::generate(manifest);
        gvk::cppgen::HandleToStringGenerator::generate(manifest);
        gvk::cppgen::HandlesGenerator::generate(manifest);
        gvk::cppgen::MakeTupleGenerator::generate(manifest);
        gvk::cppgen::SerializeStructuresGenerator::generate(manifest);
        gvk::cppgen::StructureToStringGenerator::generate(manifest);
    }
    return 0;
}
