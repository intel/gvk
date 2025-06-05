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

#include "basic-pipeline-explorer.generator.hpp"

#include "gvk-cppgen.hpp"
#include "gvk-string.hpp"
#include "gvk-xml.hpp"

std::vector<gvk::xml::Structure> get_pipeline_explorer_info_structures()
{
    std::vector<gvk::xml::Structure> structures;
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerMetricId";
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "x"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "y"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "z"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "w"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerRequestInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_REQUEST_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "refreshActivePipelines"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "refreshAvailableMetrics"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pReportPath"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDevice", "device"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "sampleMetricsPipeline"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pDecompilePipelinePath"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "decompilePipeline"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pRecompilePipelinePath"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "recompilePipeline"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "experimentEnabled"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pExperimentPipelinePath"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "experimentPipeline"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "highlightEnabled"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pHighlightPipelinePath"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "highlightPipeline"));
        gvk::cppgen::add_static_array_member_to_structure("float", "4", "highlightColor", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "warmupFrameCount"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "sampleFrameCount"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerMetricId", "sampleMetricIdCount", "pSampleMetricIds", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPipelineInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_PIPELINE_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        gvk::cppgen::add_static_array_member_to_structure("uint8_t", "GVK_PIPELINE_EXPLORER_UUID_SIZE", "uuid", structure);
        gvk::cppgen::add_static_array_member_to_structure("uint8_t", "GVK_PIPELINE_EXPLORER_UUID_SIZE", "driverUUID", structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pName"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDevice", "device"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "pipeline"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipelineBindPoint", "bindPoint"));
        gvk::cppgen::add_string_array_members_to_structure("labelCount", "pLabels", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "experimentEnabled"));
        gvk::cppgen::add_static_array_member_to_structure("uint8_t", "GVK_PIPELINE_EXPLORER_UUID_SIZE", "experimentUUID", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "highlightEnabled"));
        gvk::cppgen::add_static_array_member_to_structure("float", "4", "highlightColor", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerMetricInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_METRIC_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerMetricId", "id"));
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerMetricType", "type"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pUri"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pName"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pDescription"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pUnits"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pSymbolicName"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pGroupName"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerMetricResultInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_METRIC_RESULT_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerMetricInfo", "metricInfo"));
        gvk::cppgen::add_array_members_to_structure("double", "sampleCount", "pValues", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("double", "total"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "average"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPipelineResultInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_PIPELINE_RESULT_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerPipelineInfo", "pipelineInfo"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerMetricResultInfo", "metricResultCount", "pMetricResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerResultInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_SAMPLE_RESULT_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        gvk::cppgen::add_string_array_members_to_structure("messageCount", "ppMessages", structure);
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPipelineResultInfo", "pipelineResultCount", "pPipelineResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerAvailableMetricsInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_AVAILABLE_METRICS_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerMetricInfo", "metricInfoCount", "pMetricInfos", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerCollectionRange";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_COLLECTION_RANGE";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDevice", "device"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "pipeline"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipelineBindPoint", "bindPoint"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "begin"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "end"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerWorkspaceInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_WORKSPACE_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "waitForDebugger"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "openTerminal"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "autoWorkingDirectory"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "autoWorkspace"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pLaunch"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pTarget"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pArgs"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pWorkingDirectory"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pWorkspace"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerGuiInfo";
        structure.vkStructureType = "GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_GUI_INFO";
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkExtent2D", "extent"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkOffset2D", "position"));
        structure.members.push_back(gvk::cppgen::create_parameter("float", "fontScale"));
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerWorkspaceInfo", "workspace"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerWorkspaceInfo", "recentWorkspaceCount", "pRecentWorkspaces", structure);
        structures.push_back(structure);
    }
    return structures;
}

gvk::xml::Enumeration get_pipeline_explorer_info_structure_type_enumeration()
{
    std::set<std::string> sTypeValues;
    gvk::xml::Enumeration enumeration;
    enumeration.name = "GvkPipelineExplorerInfoStructureType";

    gvk::xml::Enumerator enumerator;
    enumerator.name = "GVK_STRUCTURE_TYPE_UNDEFINED";
    enumerator.value = "0";
    sTypeValues.insert(enumerator.value);
    enumeration.enumerators.insert(enumerator);

    for (const auto& commandStructure : get_pipeline_explorer_info_structures()) {
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

std::vector<gvk::xml::Enumeration> get_pipeline_explorer_info_enumerations()
{
    std::vector<gvk::xml::Enumeration> enumerations;
    enumerations.push_back(get_pipeline_explorer_info_structure_type_enumeration());
    {
        gvk::xml::Enumeration enumeration;
        enumeration.name = "GvkPipelineExplorerMetricType";
        gvk::xml::Enumerator enumerator;
        enumerator.name = "GVK_PIPELINE_EXPLORER_METRIC_TYPE_UNKNOWN";
        enumerator.value = "0";
        enumeration.enumerators.insert(enumerator);
        enumerator.name = "GVK_PIPELINE_EXPLORER_METRIC_TYPE_PERCENT";
        enumerator.value = "1";
        enumeration.enumerators.insert(enumerator);
        enumerator.name = "GVK_PIPELINE_EXPLORER_METRIC_TYPE_TIME";
        enumerator.value = "2";
        enumeration.enumerators.insert(enumerator);
        enumerator.name = "GVK_PIPELINE_EXPLORER_METRIC_TYPE_VALUE";
        enumerator.value = "3";
        enumeration.enumerators.insert(enumerator);
        enumerator.name = "GVK_PIPELINE_EXPLORER_METRIC_TYPE_COUNT";
        enumerator.value = "4";
        enumeration.enumerators.insert(enumerator);
        enumerator.name = "GVK_PIPELINE_EXPLORER_METRIC_TYPE_NUMBER";
        enumerator.value = "5";
        enumeration.enumerators.insert(enumerator);
        enumerations.push_back(enumeration);
    }
    return enumerations;
}

int main(int, const char*[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        gvk::cppgen::ApiElementCollectionInfo apiElements { };
        apiElements.name = "pipeline-explorer";
        apiElements.headerGuard = "gvk_pipeline_explorer_h";
        apiElements.includePath = GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PATH;
        apiElements.includePrefix = GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PREFIX;
        apiElements.sourcePath = GVK_PIPELINE_EXPLORER_GENERATED_SOURCE_PATH;
        apiElements.enumerations = get_pipeline_explorer_info_enumerations();
        apiElements.structures = get_pipeline_explorer_info_structures();
        apiElements.declarationIncludes = {
            "gvk-command-structures/generated/command.h",
        };
        apiElements.headerIncludes = {
            GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PREFIX "pipeline-explorer.h",
        };
        apiElements.sourceIncludes = {
            "gvk-command-structures.hpp",
            "gvk-structures.hpp",
        };
        apiElements.definitions = {
            "#define GVK_PIPELINE_EXPLORER_UUID_SIZE 32",
            "#define GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT (UINT32_MAX - 0)",
            "#define GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY (UINT32_MAX - 1)",
            "#define GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY (UINT32_MAX - 2)",
            "#define GVK_PIPELINE_EXPLORER_METRIC_ID_PERFORMANCE_QUERY (UINT32_MAX - 3)",
        };
        gvk::cppgen::BasicPipelineExplorerGenerator::generate(manifest);
        gvk::cppgen::ApiElementCollectionDeclarationGenerator::generate(apiElements);
        gvk::cppgen::StructureComparisonOperatorsGenerator::generate(apiElements);
        gvk::cppgen::StructureMakeTupleGenerator::generate(manifest, apiElements);
        gvk::cppgen::EnumerationToStringGenerator::generate(apiElements);
        gvk::cppgen::StructureCerealizationGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureCreateCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDecerealizationGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDeserializationGenerator::generate(apiElements);
        gvk::cppgen::StructureDestroyCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureGetSTypeGenerator::generate(apiElements);
        gvk::cppgen::StructureSerializationGenerator::generate(apiElements);
        apiElements.manuallyImplemented.insert("GvkPipelineExplorerPipelineInfo");
        gvk::cppgen::StructureToStringGenerator::generate(manifest, apiElements);
    }
    return 0;
}
