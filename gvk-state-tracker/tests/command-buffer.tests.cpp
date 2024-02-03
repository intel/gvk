
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

#include "state-tracker-test-utilities.hpp"

#include <vector>

TEST(CommandBuffer, CommandBufferResourceLifetime)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    auto expectedInstanceObjects = get_expected_instance_objects(context);
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();

    auto commandPoolCreateInfo = gvk::get_default<VkCommandPoolCreateInfo>();
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    gvk::CommandPool commandPool;
    ASSERT_EQ(gvk::CommandPool::create(context.get_devices()[0], &commandPoolCreateInfo, nullptr, &commandPool), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(commandPool, commandPool.get<VkCommandPoolCreateInfo>(), expectedInstanceObjects));

    auto commandBufferAllocateInfo = gvk::get_default<VkCommandBufferAllocateInfo>();
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 8;
    std::vector<VkCommandBuffer> vkCommandBuffers(commandBufferAllocateInfo.commandBufferCount);
    ASSERT_NE(dispatchTable.gvkAllocateCommandBuffers, nullptr);
    ASSERT_EQ(dispatchTable.gvkAllocateCommandBuffers(context.get_devices()[0], &commandBufferAllocateInfo, vkCommandBuffers.data()), VK_SUCCESS);
    for (const auto& vkCommandBuffer : vkCommandBuffers) {
        GvkStateTrackedObject stateTrackedCommandBuffer { };
        stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
        stateTrackedCommandBuffer.handle = (uint64_t)vkCommandBuffer;
        stateTrackedCommandBuffer.dispatchableHandle = (uint64_t)vkCommandBuffer;
        ASSERT_TRUE(create_state_tracked_object_record(stateTrackedCommandBuffer, commandBufferAllocateInfo, expectedInstanceObjects));
    }

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    std::vector<VkCommandBuffer> vkCommandBuffersToFree(3);
    for (uint32_t i = 0; i < vkCommandBuffersToFree.size(); ++i) {
        GvkStateTrackedObject stateTrackedCommandBuffer { };
        stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
        stateTrackedCommandBuffer.handle = (uint64_t)vkCommandBuffers.back();
        stateTrackedCommandBuffer.dispatchableHandle = (uint64_t)vkCommandBuffers.back();
        expectedInstanceObjects.erase(stateTrackedCommandBuffer);
        vkCommandBuffersToFree[i] = vkCommandBuffers.back();
        vkCommandBuffers.pop_back();
    }
    ASSERT_NE(dispatchTable.gvkFreeCommandBuffers, nullptr);
    dispatchTable.gvkFreeCommandBuffers(context.get_devices()[0], commandPool, (uint32_t)vkCommandBuffersToFree.size(), vkCommandBuffersToFree.data());

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    GvkStateTrackedObject stateTrackedCommandPool { };
    stateTrackedCommandPool.type = VK_OBJECT_TYPE_COMMAND_POOL;
    stateTrackedCommandPool.handle = (uint64_t)(VkCommandPool)commandPool;
    stateTrackedCommandPool.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
    expectedInstanceObjects.erase(stateTrackedCommandPool);
    for (auto vkCommandBuffer : vkCommandBuffers) {
        GvkStateTrackedObject stateTrackedCommandBuffer { };
        stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
        stateTrackedCommandBuffer.handle = (uint64_t)vkCommandBuffer;
        stateTrackedCommandBuffer.dispatchableHandle = (uint64_t)vkCommandBuffer;
        expectedInstanceObjects.erase(stateTrackedCommandBuffer);
    }
    commandPool.reset();

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);
}
