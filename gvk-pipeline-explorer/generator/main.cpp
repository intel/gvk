
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

std::string get_structure_type_enumeration(const std::string& structureName)
{
    std::string structureTypeEnumeration = "GVK_STRUCTURE_TYPE";
    for (const auto& token : gvk::string::split_camel_case(gvk::string::remove(structureName, "Gvk"))) {
        structureTypeEnumeration += "_" + gvk::string::to_upper(token);
    }
    return structureTypeEnumeration;
}

void add_stype_members_to_structure(gvk::xml::Structure& structure)
{
    structure.vkStructureType = get_structure_type_enumeration(structure.name);
    structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerInfoStructureType", "sType"));
}

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
        add_stype_members_to_structure(structure);
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
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "warmupRangeCount"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "queryRangeCount"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerMetricId", "sampleMetricIdCount", "pSampleMetricIds", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "getApiCalls"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "getGpuCalls"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPipelineInfo";
        add_stype_members_to_structure(structure);
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
        add_stype_members_to_structure(structure);
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
        structure.name = "GvkPipelineExplorerPerformanceCounterCollection";
        add_stype_members_to_structure(structure);
        gvk::cppgen::add_array_members_to_structure("VkPerformanceCounterKHR", "count", "pCounters", structure);
        structure.members.push_back(gvk::cppgen::get_array_parameters("count", "pDescriptions", "VkPerformanceCounterDescriptionKHR").second);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerCommandCollectionRequestInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pReportPath"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerCommandCollectionResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("GvkCommandCollection", "commands"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPerformanceQueryRequestInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pReportPath"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDevice", "device"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "pipeline"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "warmupRangeCount"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "queryRangeCount"));
        gvk::cppgen::add_array_members_to_structure("VkPerformanceCounterKHR", "counterCount", "pCounters", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPerformanceCounterResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("VkPerformanceCounterKHR", "counter"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPerformanceCounterDescriptionKHR", "description"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "total"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "average"));
        gvk::cppgen::add_array_members_to_structure("double", "valueCount", "pValues", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPerformanceQueryGroupResultInfo";
        add_stype_members_to_structure(structure);
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPerformanceCounterResultInfo", "counterResultCount", "pCounterResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPerformanceQueryResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pName"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pDate"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pTime"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pNote"));
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerPipelineInfo", "pipelineInfo"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPerformanceQueryGroupResultInfo", "groupResultCount", "pGroupResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerTimestampQueryCmdSequenceResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "beginTimestamp"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "endTimestamp"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "totalCmdDuration"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "averageCmdDuration"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint64_t", "firstCmdIndex"));
        gvk::cppgen::add_array_members_to_structure("GvkCommandStructureType", "cmdCount", "pCmdTypes", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerTimestampQueryCmdRangeResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("double", "totalCmdSequenceDuration"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "averageCmdSequenceDuration"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "totalCmdSequenceCmdCount"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "averageCmdSequenceCmdCount"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerTimestampQueryCmdSequenceResultInfo", "cmdSequenceCount", "pCmdSequences", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPipelineTimestampQueryResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerPipelineInfo", "pipelineInfo"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "totalCmdRangeDuration"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "averageCmdRangeDuration"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "totalCmdRangeCmdCount"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "averageCmdRangeCmdCount"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerTimestampQueryCmdRangeResultInfo", "cmdRangeResultCount", "pCmdRangeResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerTimestampQueryResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pName"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pDate"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pTime"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pNote"));
        structure.members.push_back(gvk::cppgen::create_parameter("float", "timestampPeriod"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPipelineTimestampQueryResultInfo", "pipelineTimestampQueryResultCount", "pPipelineTimestampQueryResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerMetricResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerMetricInfo", "metricInfo"));
        gvk::cppgen::add_array_members_to_structure("double", "sampleCount", "pValues", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("double", "total"));
        structure.members.push_back(gvk::cppgen::create_parameter("double", "average"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPipelineResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerPipelineInfo", "pipelineInfo"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerMetricResultInfo", "metricResultCount", "pMetricResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerResultInfo";
        add_stype_members_to_structure(structure);
        gvk::cppgen::add_string_array_members_to_structure("messageCount", "ppMessages", structure);
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPipelineResultInfo", "pipelineResultCount", "pPipelineResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPipelineStatisticsQueryRequestInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pReportPath"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDevice", "device"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkPipeline", "pipeline"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "warmupRangeCount"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "queryRangeCount"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPipelineStatisticsQueryResultInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pName"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pDate"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pTime"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pNote"));
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerPipelineInfo", "pipelineInfo"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPerformanceCounterResultInfo", "counterResultCount", "pCounterResults", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerAvailableMetricsInfo";
        add_stype_members_to_structure(structure);
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerMetricInfo", "metricInfoCount", "pMetricInfos", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerCollectionRange";
        add_stype_members_to_structure(structure);
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
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "waitForDebugger"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "openTerminal"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "autoWorkingDirectory"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "autoWorkspace"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "autoLogPath"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "logToStdOut"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "logToFile"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "record"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pLaunch"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pTarget"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pArgs"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pWorkingDirectory"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pWorkspace"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pLogPath"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pGits"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerGuiInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_parameter("VkExtent2D", "extent"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkOffset2D", "position"));
        structure.members.push_back(gvk::cppgen::create_parameter("float", "fontScale"));
        structure.members.push_back(gvk::cppgen::create_parameter("GvkPipelineExplorerWorkspaceInfo", "workspace"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerWorkspaceInfo", "recentWorkspaceCount", "pRecentWorkspaces", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerAutoQueryResultInfo";
        add_stype_members_to_structure(structure);
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPipelineResultInfo", "pipelineResultCount", "pPipelineResults", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("GvkCommandCollection", "commands"));
        gvk::cppgen::add_array_members_to_structure("double", "commandCount", "pCmdDurations", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPerformanceCounterSet";
        add_stype_members_to_structure(structure);
        gvk::cppgen::add_static_array_member_to_structure("uint8_t", "VK_UUID_SIZE", "uuid", structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pName"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pDescription"));
        gvk::cppgen::add_array_members_to_structure("VkPerformanceCounterKHR", "counterCount", "pCounters", structure);
        structure.members.push_back(gvk::cppgen::get_array_parameters("counterCount", "pDescriptions", "VkPerformanceCounterDescriptionKHR").second);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPerformanceCounterGroup";
        add_stype_members_to_structure(structure);
        gvk::cppgen::add_static_array_member_to_structure("uint8_t", "VK_UUID_SIZE", "uuid", structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pName"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pDescription"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPerformanceCounterSet", "setCount", "pSets", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPluginCounterInfo";
        add_stype_members_to_structure(structure);
        gvk::cppgen::add_static_array_member_to_structure("uint8_t", "VK_UUID_SIZE", "uuid", structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pName"));
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pDescription"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerPerformanceCounterGroup", "groupCount", "pGroups", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPluginInitializeInfo";
        add_stype_members_to_structure(structure);
        structure.members.push_back(gvk::cppgen::create_const_string_parameter("pWorkspace"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_vkGetInstanceProcAddr", "pfnGetInstanceProcAddr"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerPluginInfo";
        gvk::cppgen::add_static_array_member_to_structure("char", "VK_MAX_DESCRIPTION_SIZE", "name", structure);
        gvk::cppgen::add_static_array_member_to_structure("char", "VK_MAX_DESCRIPTION_SIZE", "description", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkGetPipelineExplorerPluginStatus", "pfnGetStatus"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkGetPipelineExplorerPluginCounterInfo", "pfnGetCounterInfo"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkSubmitPipelineExplorerPluginRequest", "pfnSubmitRequest"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkSubmitPipelineExplorerPluginToolCmd", "pfnToolCmd"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerPluginCommandCallback", "pfnPreProcessCreateInstance"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerPluginCommandCallback", "pfnPostProcessCreateInstance"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerPluginCommandCallback", "pfnPreProcessCreateDevice"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerPluginCommandCallback", "pfnPostProcessCreateDevice"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCallbackEx", "pfnPreProcessRange"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCommandBufferCallbackEx", "pfnPreProcessCommandBuffers"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCommandBufferCallbackEx", "pfnPreProcessCmd"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCommandBufferCallbackEx", "pfnPostProcessCmd"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCommandBufferCallbackEx", "pfnPostProcessCommandBuffers"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolQueueCallbackEx", "pfnPreProcessQueueSubmission"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolQueueCallbackEx", "pfnPostProcessQueueSubmission"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCallbackEx", "pfnPostProcessRange"));
        structure.members.push_back(gvk::cppgen::create_parameter("void*", "pUserData"));
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerToolQueueInfoEx";
        structure.members.push_back(gvk::cppgen::create_parameter("VkPhysicalDevice", "physicalDevice"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDevice", "device"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkQueue", "queue"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "queueFamilyIndex"));
        structure.members.push_back(gvk::cppgen::create_const_pointer_parameter("GvkCommandBaseStructure", "pCommand"));
        gvk::cppgen::add_array_members_to_structure("GvkCommandCmdBaseStructure* const", "cmdCount", "ppCmds", structure);
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerCollectionRange", "collectionRangeCount", "pCollectionRanges", structure);
        gvk::cppgen::add_array_members_to_structure("VkPerformanceCounterKHR", "counterCount", "pCounters", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerToolCommandBufferInfoEx";
        structure.members.push_back(gvk::cppgen::create_parameter("VkPhysicalDevice", "physicalDevice"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkDevice", "device"));
        structure.members.push_back(gvk::cppgen::create_parameter("VkQueue", "queue"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "queueFamilyIndex"));
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "cmdIndex"));
        gvk::cppgen::add_array_members_to_structure("GvkCommandCmdBaseStructure* const", "cmdCount", "ppCmds", structure);
        structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "collectionRangeIndex"));
        gvk::cppgen::add_array_members_to_structure("GvkPipelineExplorerCollectionRange", "collectionRangeCount", "pCollectionRanges", structure);
        gvk::cppgen::add_array_members_to_structure("VkPerformanceCounterKHR", "counterCount", "pCounters", structure);
        structures.push_back(structure);
    }
    {
        gvk::xml::Structure structure;
        structure.name = "GvkPipelineExplorerToolCallbackInfoEx";
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCallbackEx", "pfnPreProcessRange"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCommandBufferCallbackEx", "pfnPreProcessCommandBuffers"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCommandBufferCallbackEx", "pfnPreProcessCmd"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCommandBufferCallbackEx", "pfnPostProcessCmd"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCommandBufferCallbackEx", "pfnPostProcessCommandBuffers"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolQueueCallbackEx", "pfnPreProcessQueueSubmission"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolQueueCallbackEx", "pfnPostProcessQueueSubmission"));
        structure.members.push_back(gvk::cppgen::create_parameter("PFN_gvkPipelineExplorerToolCallbackEx", "pfnPostProcessRange"));
        structure.members.push_back(gvk::cppgen::create_parameter("void*", "pUserData"));
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
    {
        gvk::xml::Enumeration enumeration;
        enumeration.name = "GvkPipelineExplorerRequestStatus";
        gvk::xml::Enumerator enumerator;
        enumerator.name = "GVK_PIPELINE_EXPLORER_REQUEST_IDLE";
        enumerator.value = "0";
        enumeration.enumerators.insert(enumerator);
        enumerator.name = "GVK_PIPELINE_EXPLORER_REQUEST_PENDING";
        enumerator.value = "1";
        enumeration.enumerators.insert(enumerator);
        enumerator.name = "GVK_PIPELINE_EXPLORER_REQUEST_READY";
        enumerator.value = "2";
        enumeration.enumerators.insert(enumerator);
        enumerator.name = "GVK_PIPELINE_EXPLORER_REQUEST_FAILED";
        enumerator.value = "3";
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
            "VK_DEFINE_HANDLE(GvkPipelineExplorerPlugin)",
            "struct GvkPipelineExplorerToolQueueInfoEx;",
            "struct GvkPipelineExplorerToolCommandBufferInfoEx;",
            "struct GvkPipelineExplorerPluginInfo;",
            "struct GvkPipelineExplorerPerformanceCounterGroup;",
            "struct GvkPipelineExplorerPluginCounterInfo;",
            "struct GvkPipelineExplorerPluginInitializeInfo;",
            "struct GvkPipelineExplorerPerformanceQueryRequestInfo;",
            "typedef VkResult(VKAPI_PTR* PFN_gvkPipelineExplorerToolCallbackEx)(void* pUserData);",
            "typedef VkResult(VKAPI_PTR* PFN_gvkPipelineExplorerToolQueueCallbackEx)(const GvkPipelineExplorerToolQueueInfoEx* pToolQueueInfo, void* pUserData);",
            "typedef VkResult(VKAPI_PTR* PFN_gvkPipelineExplorerToolCommandBufferCallbackEx)(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolCommandBufferInfo, void* pUserData);",
            "typedef VkResult(VKAPI_PTR* PFN_gvkPipelineExplorerPluginCommandCallback)(const GvkCommandBaseStructure* pCommand, void* pUserData);",
            "typedef VkResult(VKAPI_PTR* PFN_gvkInitializePipelineExplorerPlugin)(const GvkPipelineExplorerPluginInitializeInfo* pInitializeInfo, GvkPipelineExplorerPluginInfo* pPluginInfo);",
            "typedef VkResult(VKAPI_PTR* PFN_gvkGetPipelineExplorerPluginCounterInfo)(GvkPipelineExplorerPluginCounterInfo* pCounterInfo, void* pUserData);",
            "typedef VkResult(VKAPI_PTR* PFN_gvkGetPipelineExplorerPluginStatus)(void* pUserData);",
            "typedef VkResult(VKAPI_PTR* PFN_gvkSubmitPipelineExplorerPluginRequest)(const GvkPipelineExplorerPerformanceQueryRequestInfo* pRequest, void* pUserData);",
            "typedef VkBool32(VKAPI_PTR* PFN_gvkSubmitPipelineExplorerPluginToolCmd)(VkDevice device, VkPipeline pipeline, void* pUserData);",
        };
        gvk::cppgen::ApiElementCollectionDeclarationGenerator::generate(apiElements);
        apiElements.structures.erase(
            std::remove_if(
                apiElements.structures.begin(),
                apiElements.structures.end(),
                [](const auto& structure)
                {
                    return
                        structure.name == "GvkPipelineExplorerPluginInfo" ||
                        structure.name == "GvkPipelineExplorerPluginInitializeInfo" ||
                        structure.name == "GvkPipelineExplorerToolCallbackInfoEx" ||
                        structure.name == "GvkPipelineExplorerToolCommandBufferInfoEx" ||
                        structure.name == "GvkPipelineExplorerToolQueueInfoEx";
                }
            ),
            apiElements.structures.end()
        );

        ////////////////////////////////////////////////////////////////////////////////
        // gvk-pipeline-explorer-info
        gvk::cppgen::StructureComparisonOperatorsGenerator::generate(apiElements);
        gvk::cppgen::StructureMakeTupleGenerator::generate(manifest, apiElements);
        gvk::cppgen::EnumerationToStringGenerator::generate(apiElements);
        gvk::cppgen::StructureCerealizationGenerator::generate(manifest, apiElements, "gvk-command-structures/generated/command-structure-cerealization.hpp");
        gvk::cppgen::StructureCreateCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDecerealizationGenerator::generate(manifest, apiElements, "gvk-command-structures/generated/command-structure-decerealization.hpp");
        gvk::cppgen::StructureDeserializationGenerator::generate(apiElements);
        gvk::cppgen::StructureDestroyCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureGetSTypeGenerator::generate(apiElements);
        gvk::cppgen::StructureSerializationGenerator::generate(apiElements);
        apiElements.manuallyImplemented.insert("GvkPipelineExplorerPipelineInfo");
        apiElements.manuallyImplemented.insert("GvkPipelineExplorerPerformanceCounterSet");
        apiElements.manuallyImplemented.insert("GvkPipelineExplorerPerformanceCounterGroup");
        apiElements.manuallyImplemented.insert("GvkPipelineExplorerPluginCounterInfo");
        gvk::cppgen::StructureToStringGenerator::generate(manifest, apiElements);

        ////////////////////////////////////////////////////////////////////////////////
        // gvk-pipeline-explorer-backend
        gvk::cppgen::BasicPipelineExplorerGenerator::generate(manifest);
    }
    return 0;
}
