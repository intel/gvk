
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
#include "gvk-string.hpp"
#include "gvk-xml.hpp"

std::string get_structure_type_enumerator(const std::string& structureName)
{
    std::string structureTypeEnumeration = "GVK_STRUCTURE_TYPE";
    for (const auto& token : gvk::string::split_camel_case(gvk::string::remove(structureName, "Gvk"))) {
        structureTypeEnumeration += "_" + gvk::string::to_upper(token);
    }
    return structureTypeEnumeration;
}

void add_stype_member_to_structure(gvk::xml::Structure& structure)
{
    structure.vkStructureType = get_structure_type_enumerator(structure.name);
    structure.members.push_back(gvk::cppgen::create_parameter("GvkBindingInfoStructureType", "sType"));
}

std::vector<gvk::xml::Structure> get_structures()
{
    std::vector<gvk::xml::Structure> structures;
    {
        gvk::xml::Structure structure;
        structure.name = "GvkResourceInfo";
        structure.members.push_back(gvk::cppgen::create_parameter("VkObjectType", "type"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "handle"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "dispatchableHandle"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("VkBaseOutStructure", "pCreateInfo"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkBindingInfoBaseStructure";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkBindingInfoStructureType", "sType"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pResourceInfo"));
        structures.push_back(structure);
    }
    // IndirectBufferInfo
    {
        gvk::xml::Structure structure;
        structure.name = "GvkDescriptorBufferInfo";
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pBufferViewResourceInfo"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDeviceSize", "offset"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDeviceSize", "size"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkDescriptorImageInfo";
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pImageViewResourceInfo"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pSamplerResourceInfo"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkDescriptorBindingInfo"; // InputBindingInfo
        add_stype_member_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pResourceInfo"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "setIndex"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "bindIndex"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "arrayIndex"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkShaderStageFlagBits", "stage"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkDescriptorBufferInfo", "pBufferInfo"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkDescriptorImageInfo", "pImageInfo"));
        structures.push_back(structure);
    }
    // MemoryBarrierInfo
    // PipelineConstantInfo
    {
        gvk::xml::Structure structure;
        structure.name = "GvkShaderBindingInfo";
        add_stype_member_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pResourceInfo"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkShaderStageFlagBits", "stage"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineBindingInfo";
        add_stype_member_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pResourceInfo"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipelineBindPoint", "bindPoint"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkRenderTargetBindingInfo";
        add_stype_member_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pResourceInfo"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pResourceViewInfo"));
        structures.push_back(structure);
    }
    // ResourceTransferInfo
    // ScissorRectInfo
    // ShaderTableInfo
    // ShadingRateImageInfo
    // ShadingRateInfo
    {
        gvk::xml::Structure structure;
        structure.name = "GvkIndexBufferBindingInfo";
        add_stype_member_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pResourceInfo"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "offset"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkIndexType", "indexType"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkVertexBufferBindingInfo";
        add_stype_member_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkResourceInfo", "pResourceInfo"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "offset"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkVertexInputBindingDescription", "bindingDescription"));
        gvk::cppgen::add_array_members_to_structure("VkVertexInputAttributeDescription", "attributeDescriptionCount", "pAttributeDescriptions", structure);
        structures.push_back(structure);
    }
    // ViewportInfo
    {
        gvk::xml::Structure structure;
        structure.name = "GvkComputeBindingInfo";
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkPipelineBindingInfo", "pPipelineBindingInfo"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkShaderBindingInfo", "pShaderBindingInfo"));
        gvk::cppgen::add_array_members_to_structure("GvkDescriptorBindingInfo", "descriptorCount", "pDescriptorBindingInfos", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkGraphicsBindingInfo";
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkPipelineBindingInfo", "pPipelineBindingInfo"));
        gvk::cppgen::add_array_members_to_structure("GvkShaderBindingInfo", "shaderCount", "pShaderBindingInfos", structure);
        gvk::cppgen::add_array_members_to_structure("GvkDescriptorBindingInfo", "descriptorCount", "pDescriptorBindingInfos", structure);
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkIndexBufferBindingInfo", "pIndexBufferBindingInfo"));
        gvk::cppgen::add_array_members_to_structure("GvkVertexBufferBindingInfo", "vertexBufferCount", "pVertexBufferBindingInfos", structure);
        gvk::cppgen::add_array_members_to_structure("GvkRenderTargetBindingInfo", "renderTargetCount", "pRenderTargetBindingInfos", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkRayTracingBindingInfo";
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkPipelineBindingInfo", "pPipelineBindingInfo"));
        gvk::cppgen::add_array_members_to_structure("GvkShaderBindingInfo", "shaderCount", "pShaderBindingInfos", structure);
        gvk::cppgen::add_array_members_to_structure("GvkDescriptorBindingInfo", "descriptorCount", "pDescriptorBindingInfos", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkBindingInfo";
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkComputeBindingInfo", "pComputeBindingInfo"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkGraphicsBindingInfo", "pGraphicsBindingInfo"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkRayTracingBindingInfo", "pRayTracingBindingInfo"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkBindingInfoCollection";
        auto arrayMembers = gvk::cppgen::get_array_parameters("bindingInfoCount", "pBindingInfos", "GvkBindingInfo");
        arrayMembers.first.type = "uint64_t";
        arrayMembers.first.unqualifiedType = "uint64_t";
        structure.members.push_back(arrayMembers.first);
        structure.members.push_back(arrayMembers.second);
        structures.push_back(structure);
    }
    return structures;
}

gvk::xml::Enumeration get_structure_type_enumeration()
{
    std::set<std::string> sTypeValues;
    gvk::xml::Enumeration enumeration;
    enumeration.name = "GvkBindingInfoStructureType";

    gvk::xml::Enumerator enumerator;
    enumerator.name = "GVK_STRUCTURE_TYPE_UNDEFINED";
    enumerator.value = "0";
    sTypeValues.insert(enumerator.value);
    enumeration.enumerators.insert(enumerator);

    for (const auto& commandStructure : get_structures()) {
        if (!commandStructure.vkStructureType.empty()) {
            enumerator.name = commandStructure.vkStructureType;
            enumerator.value = gvk::to_hex_string(gvk::string::hash(enumerator.name));
            enumerator.compileGuards = commandStructure.compileGuards;
            if (!sTypeValues.insert(enumerator.value).second) {
                enumerator.value += " GVK_PIPELINE_EXPLORER_INFO_STRUCTURE_TYPE collision!";
            }
            enumeration.enumerators.insert(enumerator);
        }
    }
    return enumeration;
}

std::vector<gvk::xml::Enumeration> get_enumerations()
{
    std::vector<gvk::xml::Enumeration> enumerations;
    enumerations.push_back(get_structure_type_enumeration());
    return enumerations;
}

int main(int, const char*[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        gvk::cppgen::ApiElementCollectionInfo apiElements { };
        apiElements.name = "binding-info";
        apiElements.headerGuard = "gvk_binding_info_h";
        apiElements.includePath = GVK_BINDING_INFO_GENERATED_INCLUDE_PATH;
        apiElements.includePrefix = GVK_BINDING_INFO_GENERATED_INCLUDE_PREFIX;
        apiElements.sourcePath = GVK_BINDING_INFO_GENERATED_SOURCE_PATH;
        apiElements.enumerations = get_enumerations();
        apiElements.structures = get_structures();
        apiElements.declarationIncludes = {
        };
        apiElements.headerIncludes = {
            GVK_BINDING_INFO_GENERATED_INCLUDE_PREFIX "binding-info.h",
        };
        apiElements.sourceIncludes = {
            "gvk-structures.hpp",
        };
        apiElements.manuallyImplemented = {
            "GvkResourceInfo",
        };
        apiElements.typeErasedStructures = {
            "GvkBindingInfoBaseStructure",
        };
        gvk::cppgen::ApiElementCollectionDeclarationGenerator::generate(apiElements);
        gvk::cppgen::StructureComparisonOperatorsGenerator::generate(apiElements);
        gvk::cppgen::StructureMakeTupleGenerator::generate(manifest, apiElements, "gvk-binding-info/detail/make-tuple-manual.hpp");
        gvk::cppgen::EnumerationToStringGenerator::generate(apiElements);
        gvk::cppgen::StructureCerealizationGenerator::generate(manifest, apiElements, "gvk-binding-info/detail/cerealization-manual.hpp");
        gvk::cppgen::StructureCreateCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDecerealizationGenerator::generate(manifest, apiElements, "gvk-binding-info/detail/cerealization-manual.hpp");
        gvk::cppgen::StructureDeserializationGenerator::generate(apiElements);
        gvk::cppgen::StructureDestroyCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureGetSTypeGenerator::generate(apiElements);
        gvk::cppgen::StructureSerializationGenerator::generate(apiElements);
        gvk::cppgen::StructureToStringGenerator::generate(manifest, apiElements);
    }
    return 0;
}
