
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
