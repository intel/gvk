
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

#include "gvk-state-tracker/acceleration-structure-geometry-tracker.hpp"

#include "gvk-spirv/gpu-memcpy.hpp"

namespace gvk {
namespace state_tracker {

VkResult AccelerationStructureGeometryTracker::create_resources(const gvk::Device& gvkDevice, const gvk::Queue& gvkQueue)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Create GPU memcpy pipeline if necessary
        gvk_result(gvk::create_gpu_memcpy_pipeline(gvkDevice, &gpuMemcpyPipeline));

        // Create command pool and allocate command buffer if necessary
        auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = gvkQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
        gvk_result(CommandPool::create(gvkDevice, &commandPoolCreateInfo, nullptr, &gvkCommandPool));
        auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
        commandBufferAllocateInfo.commandPool = gvkCommandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
        gvk_result(gvkDevice.get<DispatchTable>().gvkAllocateCommandBuffers(gvkDevice, &commandBufferAllocateInfo, &vkCommandBuffer));
        // NOTE : This initializes the command buffer's dispatch table.  This needs to
        //  be done for command buffers allocated in layers.  At some point this will be
        //  automated in gvk::layer::Registry, but until then layers must handle this.
        // NOTE : See the following for more info regarding dispatchable handles...
        //  https://vulkan.lunarg.com/doc/view/latest/linux/vkspec.html#fundamentals-objectmodel-overview
        //  https://renderdoc.org/vulkan-layer-guide.html
        *(void**)vkCommandBuffer = *(void**)gvkDevice.get<VkDevice>();
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk
