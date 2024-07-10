
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
#include "gvk-restore-point/layer.hpp"

#include <iostream>

namespace gvk {
namespace restore_point {

VkResult Creator::process_VkQueue(GvkQueueRestoreInfo& restoreInfo)
{
    restoreInfo.deviceQueueCreateInfo = mDeviceQueueCreateInfos[(VkQueue)restoreInfo.handle];
    return BasicCreator::process_VkQueue(restoreInfo);
}

VkResult Layer::process_queue_submission(VkCommandBuffer vkCommandBuffer)
{
#if 0
    std::cout << "VkResult Layer::process_queue_submission()" << std::endl;
    gvk_result_scope_begin(VK_SUCCESS) {
        // TODO : Documentation
        auto stateTrackedCommandBuffer = get_default<GvkStateTrackedObject>();
        stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
        stateTrackedCommandBuffer.handle = (uint64_t)vkCommandBuffer;
        stateTrackedCommandBuffer.dispatchableHandle = (uint64_t)vkCommandBuffer;

        // TODO : Documentation
        std::set<GvkStateTrackedObject> modifiedObjects;
        auto enumerateInfo = get_default<GvkStateTrackedObjectEnumerateInfo>();
        enumerateInfo.pfnCallback = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
        {
            auto pCmdInfo = (const GvkCommandBaseStructure*)pInfo;
            auto pModifiedObjects = (std::set<GvkStateTrackedObject>*)pUserData;
            (void)pModifiedObjects;
            switch (pCmdInfo->sType) {
            default: {
                // TODO : Mark objects for data restoration
                // std::cout << to_string(pCmdInfo->sType, Printer::EnumIdentifier) << std::endl;
            } break;
            }
        };
        enumerateInfo.pUserData = &modifiedObjects;
        gvkEnumerateStateTrackedCommandBufferCmds(&stateTrackedCommandBuffer, &enumerateInfo);

        // TODO : Documentation
        for (const auto& object : modifiedObjects) {
            (void)object;
            // TODO : Dynamic restoration
            gvk_result(VK_SUCCESS);
        }

        // TODO : Documentation
        for (auto& gvkRestorePoint : get_restore_points()) {
            gvkRestorePoint->dataRestorationRequired.insert(modifiedObjects.begin(), modifiedObjects.end());
        }
    } gvk_result_scope_end;
    std::cout << std::endl;
    return gvkResult;
#else
    (void)vkCommandBuffer;
    return VK_SUCCESS;
#endif
}

VkResult Layer::pre_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult gvkResult)
{
    (void)queue;
    (void)fence;
    if (!get_restore_points().empty()) {
        // TODO : Need to check for GVK_RESTORE_POINT_CREATE_DYNAMIC_DATA_BIT
        for (uint32_t submit_i = 0; submit_i < submitCount && gvkResult == VK_SUCCESS; ++submit_i) {
            const auto& submit = pSubmits[submit_i];
            for (uint32_t commandBuffer_i = 0; commandBuffer_i < submit.commandBufferCount && gvkResult == VK_SUCCESS; ++commandBuffer_i) {
                gvkResult = process_queue_submission(submit.pCommandBuffers[commandBuffer_i]);
            }
        }
    }
    return gvkResult;
}

VkResult Layer::pre_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, VkResult gvkResult)
{
    (void)queue;
    (void)fence;
    if (!get_restore_points().empty()) {
        // TODO : Need to check for GVK_RESTORE_POINT_CREATE_DYNAMIC_DATA_BIT
        for (uint32_t submit_i = 0; submit_i < submitCount && gvkResult == VK_SUCCESS; ++submit_i) {
            const auto& submit = pSubmits[submit_i];
            for (uint32_t commandBuffer_i = 0; commandBuffer_i < submit.commandBufferInfoCount && gvkResult == VK_SUCCESS; ++commandBuffer_i) {
                gvkResult = process_queue_submission(submit.pCommandBufferInfos[commandBuffer_i].commandBuffer);
            }
        }
    }
    return gvkResult;
}

VkResult Layer::pre_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, VkResult gvkResult)
{
    (void)queue;
    (void)fence;
    if (!get_restore_points().empty()) {
        // TODO : Need to check for GVK_RESTORE_POINT_CREATE_DYNAMIC_DATA_BIT
        for (uint32_t submit_i = 0; submit_i < submitCount && gvkResult == VK_SUCCESS; ++submit_i) {
            const auto& submit = pSubmits[submit_i];
            for (uint32_t commandBuffer_i = 0; commandBuffer_i < submit.commandBufferInfoCount && gvkResult == VK_SUCCESS; ++commandBuffer_i) {
                gvkResult = process_queue_submission(submit.pCommandBufferInfos[commandBuffer_i].commandBuffer);
            }
        }
    }
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk
