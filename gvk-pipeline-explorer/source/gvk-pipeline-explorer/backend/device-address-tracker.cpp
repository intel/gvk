
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

#include "gvk-pipeline-explorer/backend/device-address-tracker.hpp"
#include "gvk-pipeline-explorer/backend/handle-info.hpp"
#include "gvk-handles.hpp"

namespace gvk {
namespace pipeline_explorer {

VkResult DeviceAddressTracker::add_buffer_binding(VkDevice device, const VkBindBufferMemoryInfo* pBinding)
{
    std::lock_guard<std::mutex> lock(mMutex);
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk::Device gvkDevice(device);
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        pipeline_explorer::DeviceInfo deviceInfo(device);
        gvk_result(deviceInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pBinding ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        pipeline_explorer::BufferInfo bufferInfo({ device, pBinding->buffer });
        gvk_result(bufferInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(!pBinding->pNext ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT);

        // Get buffer device address
        auto bufferDeviceAddressInfo = gvk::get_default<VkBufferDeviceAddressInfo>();
        bufferDeviceAddressInfo.buffer = pBinding->buffer;
        auto deviceAddress = gvkDevice.GetBufferDeviceAddressKHR(&bufferDeviceAddressInfo);
        Interval<VkDeviceAddress> interval{ deviceAddress, deviceAddress + bufferInfo->bufferCreateInfo->size - 1 };

        // Cache binding
        mBindings[interval].insert(*pBinding);
        mBuffers[pBinding->buffer].insert(interval);
        mMemories[pBinding->memory].insert(interval);

    } gvk_result_scope_end;
    return gvkResult;
}

void DeviceAddressTracker::get_buffer_bindings(VkDeviceAddress deviceAddress, uint32_t* pBindingCount, VkBindBufferMemoryInfo* pBindings)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (deviceAddress && pBindingCount) {
        uint32_t binding_i = 0;
        mBindings.enumerate(deviceAddress,
            [&](auto, const auto& bindings)
            {
                if (pBindings) {
                    auto bindingItr = bindings.begin();
                    for (; binding_i < *pBindingCount && bindingItr != bindings.end(); ++binding_i, ++bindingItr) {
                        pBindings[binding_i] = *bindingItr;
                    }
                } else {
                    *pBindingCount += (uint32_t)bindings.size();
                }
            }
        );
    }
}

VkResult DeviceAddressTracker::erase_buffer_bindings(VkDevice device, VkBuffer buffer)
{
    std::lock_guard<std::mutex> lock(mMutex);
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(device ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(buffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        // TODO :
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult DeviceAddressTracker::erase_memory_bindings(VkDevice device, VkDeviceMemory memory)
{
    std::lock_guard<std::mutex> lock(mMutex);
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(device ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(memory ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        // TODO :
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace pipeline_explorer
} // namespace gvk
