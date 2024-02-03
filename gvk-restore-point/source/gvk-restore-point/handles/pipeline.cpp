
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

#include "gvk-restore-point/applier.hpp"
#include "gvk-restore-point/creator.hpp"

namespace gvk {
namespace restore_point {

VkResult Applier::process_GvkCommandStructureCreateComputePipelines(const GvkRestorePointObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateComputePipelines& commandStructure)
{
    // TODO : Reset after processing
    (void)restorePointObject;
    (void)restoreInfo;
    commandStructure.pipelineCache = VK_NULL_HANDLE;
    commandStructure.createInfoCount = 1;
    return VK_SUCCESS;
}

VkResult Applier::process_GvkCommandStructureCreateGraphicsPipelines(const GvkRestorePointObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateGraphicsPipelines& commandStructure)
{
    // TODO : Reset after processing
    (void)restorePointObject;
    (void)restoreInfo;
    commandStructure.pipelineCache = VK_NULL_HANDLE;
    commandStructure.createInfoCount = 1;
    return VK_SUCCESS;
}

VkResult Applier::process_GvkCommandStructureCreateRayTracingPipelinesKHR(const GvkRestorePointObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateRayTracingPipelinesKHR& commandStructure)
{
    // TODO : Reset after processing
    (void)restorePointObject;
    (void)restoreInfo;
    commandStructure.pipelineCache = VK_NULL_HANDLE;
    commandStructure.createInfoCount = 1;
    return VK_SUCCESS;
}

} // namespace restore_point
} // namespace gvk
