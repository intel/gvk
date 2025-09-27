
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

#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-to-string.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-structures.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-enumerations-to-string.hpp"
#include "gvk-structures/generated/handle-to-string.hpp"
#include "gvk-structures/detail/get-count.hpp"
#include "gvk-string/to-string.hpp"

namespace gvk {

template <> void print<GvkPipelineExplorerPipelineInfo>(Printer& printer, const GvkPipelineExplorerPipelineInfo& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("uuid", gvk::uuid_to_string<GVK_PIPELINE_EXPLORER_UUID_SIZE>(obj.uuid));
            printer.print_field("driverUUID", gvk::uuid_to_string<GVK_PIPELINE_EXPLORER_UUID_SIZE>(obj.driverUUID));
            printer.print_field("pName", obj.pName);
            printer.print_field("device", obj.device);
            printer.print_field("pipeline", obj.pipeline);
            printer.print_field("bindPoint", obj.bindPoint);
            printer.print_field("labelCount", obj.labelCount);
            printer.print_array("pLabels", gvk::detail::get_count(obj.labelCount), obj.pLabels);
            printer.print_field("experimentEnabled", obj.experimentEnabled);
            printer.print_field("experimentUUID", gvk::uuid_to_string<GVK_PIPELINE_EXPLORER_UUID_SIZE>(obj.experimentUUID));
            printer.print_field("highlightEnabled", obj.highlightEnabled);
            printer.print_array("highlightColor", 4, obj.highlightColor);
        }
    );
}

template <> void print<GvkPipelineExplorerPerformanceCounterSet>(Printer& printer, const GvkPipelineExplorerPerformanceCounterSet& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("uuid", gvk::uuid_to_string<VK_UUID_SIZE>(obj.uuid));
            printer.print_field("pName", obj.pName);
            printer.print_field("pDescription", obj.pDescription);
            printer.print_field("counterCount", obj.counterCount);
            printer.print_array("pCounters", gvk::detail::get_count(obj.counterCount), obj.pCounters);
            printer.print_array("pDescriptions", gvk::detail::get_count(obj.counterCount), obj.pDescriptions);
        }
    );
}

template <> void print<GvkPipelineExplorerPerformanceCounterGroup>(Printer& printer, const GvkPipelineExplorerPerformanceCounterGroup& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("uuid", gvk::uuid_to_string<VK_UUID_SIZE>(obj.uuid));
            printer.print_field("pName", obj.pName);
            printer.print_field("pDescription", obj.pDescription);
            printer.print_field("setCount", obj.setCount);
            printer.print_array("pSets", gvk::detail::get_count(obj.setCount), obj.pSets);
        }
    );
}

template <> void print<GvkPipelineExplorerPluginCounterInfo>(Printer& printer, const GvkPipelineExplorerPluginCounterInfo& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("sType", obj.sType);
            printer.print_field("uuid", gvk::uuid_to_string<VK_UUID_SIZE>(obj.uuid));
            printer.print_field("pName", obj.pName);
            printer.print_field("pDescription", obj.pDescription);
            printer.print_field("groupCount", obj.groupCount);
            printer.print_array("pGroups", gvk::detail::get_count(obj.groupCount), obj.pGroups);
        }
    );
}

} // namespace gvk
