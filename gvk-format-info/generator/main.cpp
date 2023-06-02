
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

#include "gvk-cppgen.hpp"
#include "gvk-xml.hpp"
#include "enumerate-formats.generator.hpp"
#include "get-format-info.generator.hpp"

using namespace gvk;

std::vector<xml::Structure> get_format_info_structures(const xml::Manifest& manifest)
{
    (void)manifest;
    std::vector<xml::Structure> structures;
    xml::Structure structure;
    xml::Parameter parameter;

    structure.name = "GvkFormatPlaneInfo";
    parameter.type = "uint32_t";
    parameter.name = "index";
    structure.members.push_back(parameter);
    parameter.name = "widthDivisor";
    structure.members.push_back(parameter);
    parameter.name = "heightDivisor";
    structure.members.push_back(parameter);
    parameter.type = "VkFormat";
    parameter.name = "compatible";
    structure.members.push_back(parameter);
    structures.push_back(structure);

    structure.members.clear();

    structure.name = "GvkFormatComponentInfo";
    parameter.type = "GvkFormatComponentName";
    parameter.name = "name";
    structure.members.push_back(parameter);
    parameter.type = "uint32_t";
    parameter.name = "bits";
    structure.members.push_back(parameter);
    parameter.type = "GvkNumericFormat";
    parameter.name = "numericFormat";
    structure.members.push_back(parameter);
    parameter.type = "uint32_t";
    parameter.name = "planeIndex";
    structure.members.push_back(parameter);
    structures.push_back(structure);

    structure.members.clear();

    structure.name = "GvkFormatInfo";
    parameter.type = "uint32_t";
    parameter.name = "blockSize";
    structure.members.push_back(parameter);
    parameter.name = "texelsPerBlock";
    structure.members.push_back(parameter);
    parameter.name = "chroma";
    structure.members.push_back(parameter);
    parameter.name = "packed";
    structure.members.push_back(parameter);
    parameter.unqualifiedType = "uint32_t";
    parameter.name = "blockExtent";
    parameter.length = "[3]";
    parameter.flags = xml::Static | xml::Array;
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "GvkFormatCompressionType";
    parameter.name = "compressionType";
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "GvkNumericFormat";
    parameter.name = "numericFormat";
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "const char*";
    parameter.unqualifiedType = "char";
    parameter.name = "pSpirvImageFormat";
    parameter.length = "null-terminated";
    parameter.flags = xml::Dynamic | xml::Const | xml::Pointer | xml::Array | xml::String;
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "uint32_t";
    parameter.name = "classCount";
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "const GvkFormatClass*";
    parameter.unqualifiedType = "GvkFormatClass";
    parameter.name = "pClasses";
    parameter.length = "classCount";
    parameter.flags = xml::Dynamic | xml::Const | xml::Pointer | xml::Array;
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "uint32_t";
    parameter.name = "planeCount";
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "const GvkFormatPlaneInfo*";
    parameter.unqualifiedType = "GvkFormatPlaneInfo";
    parameter.name = "pPlanes";
    parameter.length = "planeCount";
    parameter.flags = xml::Dynamic | xml::Const | xml::Pointer | xml::Array;
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "uint32_t";
    parameter.name = "componentCount";
    structure.members.push_back(parameter);
    parameter = { };
    parameter.type = "const GvkFormatComponentInfo*";
    parameter.unqualifiedType = "GvkFormatComponentInfo";
    parameter.name = "pComponents";
    parameter.length = "componentCount";
    parameter.flags = xml::Dynamic | xml::Const | xml::Pointer | xml::Array;
    structure.members.push_back(parameter);
    structures.push_back(structure);

    return structures;
}

std::string get_format_component_name_enumerator_name(const std::string& componentName)
{
    return "GVK_FORMAT_COMPONENT_NAME_" + componentName;
}

std::string get_format_compression_type_enumerator_name(const std::string& compressionType)
{
    return "GVK_FORMAT_COMPRESSION_TYPE_" + string::replace(compressionType, " ", "_");
}

std::string get_format_class_enumerator_name(const std::string& formatClass)
{
    return "GVK_FORMAT_CLASS_" + string::to_upper(string::replace(formatClass, "-", "_"));
}

std::string get_numeric_format_enumerator_name(const std::string& numericFormat)
{
    return "GVK_NUMERIC_FORMAT_" + numericFormat;
}

std::vector<xml::Enumeration> get_format_info_enumerations(const xml::Manifest& manifest)
{
    std::vector<xml::Enumeration> enumerations;
    xml::Enumeration enumeration;
    xml::Enumerator enumerator;

    enumeration.name = "GvkFormatComponentName";
    enumerator.name = "GVK_FORMAT_COMPONENT_NAME_UNDEFINED";
    enumerator.value = "0";
    enumeration.enumerators.insert(enumerator);
    uint32_t value = 1;
    std::set<std::string> componentNames;
    for (const auto& formatItr : manifest.formats) {
        for (const auto& component : formatItr.second.components) {
            if (componentNames.insert(component.name).second) {
                enumerator.name = get_format_component_name_enumerator_name(component.name);
                enumerator.value = std::to_string(value++);
                enumeration.enumerators.insert(enumerator);
            }
        }
    }
    enumerations.push_back(enumeration);

    enumeration.enumerators.clear();
    enumeration.name = "GvkFormatCompressionType";
    enumerator.name = "GVK_FORMAT_COMPRESSION_TYPE_NONE";
    enumerator.value = "0";
    enumeration.enumerators.insert(enumerator);
    value = 1;
    std::set<std::string> compressionTypes;
    for (const auto& formatItr : manifest.formats) {
        const auto& format = formatItr.second;
        if (!format.compressionType.empty() && compressionTypes.insert(format.compressionType).second) {
            enumerator.name = get_format_compression_type_enumerator_name(format.compressionType);
            enumerator.value = std::to_string(value++);
            enumeration.enumerators.insert(enumerator);
        }
    }
    enumerations.push_back(enumeration);

    enumeration.enumerators.clear();
    enumeration.name = "GvkFormatClass";
    enumerator.name = "GVK_FORMAT_CLASS_UNDEFINED";
    enumerator.value = "0";
    enumeration.enumerators.insert(enumerator);
    value = 1;
    std::set<std::string> formatClasses;
    for (const auto& formatItr : manifest.formats) {
        for (const auto& formatClass : formatItr.second.classes) {
            if (formatClasses.insert(formatClass).second) {
                enumerator.name = get_format_class_enumerator_name(formatClass);
                enumerator.value = std::to_string(value++);
                enumeration.enumerators.insert(enumerator);
            }
        }
    }
    enumerations.push_back(enumeration);

    enumeration.enumerators.clear();
    enumeration.name = "GvkNumericFormat";
    enumerator.name = "GVK_NUMERIC_FORMAT_UNDEFINED";
    enumerator.value = "0";
    enumeration.enumerators.insert(enumerator);
    value = 1;
    std::set<std::string> numericFormats;
    for (const auto& formatItr : manifest.formats) {
        for (const auto& component : formatItr.second.components) {
            if (numericFormats.insert(component.numericFormat).second) {
                enumerator.name = get_numeric_format_enumerator_name(component.numericFormat);
                enumerator.value = std::to_string(value++);
                enumeration.enumerators.insert(enumerator);
            }
        }
    }
    enumerations.push_back(enumeration);

    return enumerations;
}

int main(int, const char*[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        gvk::cppgen::ApiElementCollectionInfo apiElements { };
        apiElements.name = "format-info";
        apiElements.headerGuard = "gvk_format_info_h";
        apiElements.includePath = GVK_FORMAT_INFO_GENERATED_INCLUDE_PATH;
        apiElements.includePrefix = GVK_FORMAT_INFO_GENERATED_INCLUDE_PREFIX;
        apiElements.sourcePath = GVK_FORMAT_INFO_GENERATED_SOURCE_PATH;
        apiElements.enumerations = get_format_info_enumerations(manifest);
        apiElements.structures = get_format_info_structures(manifest);
        gvk::cppgen::ApiElementCollectionDeclarationGenerator::generate(apiElements);
        apiElements.headerIncludes = {
            GVK_FORMAT_INFO_GENERATED_INCLUDE_PREFIX "format-info.h",
        };
        apiElements.sourceIncludes = {
            "gvk-structures.hpp",
        };
        gvk::cppgen::EnumerationToStringGenerator::generate(apiElements);
        gvk::cppgen::StructureCerealizationGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureComparisonOperatorsGenerator::generate(apiElements);
        gvk::cppgen::StructureCreateCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDecerealizationGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDeserializationGenerator::generate(apiElements);
        gvk::cppgen::StructureDestroyCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureGetSTypeGenerator::generate(apiElements);
        gvk::cppgen::StructureMakeTupleGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureSerializationGenerator::generate(apiElements);
        gvk::cppgen::StructureToStringGenerator::generate(manifest, apiElements);
        gvk::cppgen::EnumerateFormatsGenerator::generate(manifest);
        gvk::cppgen::GetFormatInfoGenerator::generate(manifest);
    }
    return 0;
}
