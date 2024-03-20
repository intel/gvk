
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

#include "gtest/gtest.h"

// TODO : MultiResourceBinding
// TODO : DedicatedBufferBinding
// TODO : DedicatedImageBinding
// TODO : SparseImageBinding

TEST(DeviceMemoryBindingTracking, BindBufferMemory)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
    bufferCreateInfo.size = 64;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    gvk::Buffer buffer;
    ASSERT_EQ(gvk::Buffer::create(context.get_devices()[0], &bufferCreateInfo, (const VkAllocationCallbacks*)nullptr, &buffer), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(buffer, buffer.get<VkBufferCreateInfo>(), expectedInstanceObjects));

    VkMemoryRequirements memoryRequirements { };
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkGetBufferMemoryRequirements);
    dispatchTable.gvkGetBufferMemoryRequirements(buffer.get<gvk::Device>(), buffer, &memoryRequirements);
    auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    uint32_t memoryTypeCount = VK_MAX_MEMORY_TYPES;
    std::array<uint32_t, VK_MAX_MEMORY_TYPES> memoryTypeIndices;
    gvk::get_compatible_memory_type_indices(buffer.get<gvk::Device>().get<gvk::PhysicalDevice>(), memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, memoryTypeIndices.data());
    ASSERT_TRUE(1 <= memoryTypeCount);

    auto memoryAllocateInfo = gvk::get_default<VkMemoryAllocateInfo>();
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndices[0];
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    gvk::DeviceMemory deviceMemory;
    ASSERT_EQ(gvk::DeviceMemory::allocate(buffer.get<gvk::Device>(), &memoryAllocateInfo, nullptr, &deviceMemory), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(deviceMemory, deviceMemory.get<VkMemoryAllocateInfo>(), expectedInstanceObjects));
    assert(dispatchTable.gvkBindBufferMemory);
    ASSERT_EQ(dispatchTable.gvkBindBufferMemory(buffer.get<gvk::Device>(), buffer, deviceMemory, 0), VK_SUCCESS);

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    auto bindBufferMemoryInfo = gvk::get_default<VkBindBufferMemoryInfo>();
    bindBufferMemoryInfo.buffer = buffer;
    bindBufferMemoryInfo.memory = deviceMemory;

    std::map<GvkStateTrackedObject, ObjectRecord> expectedBufferBindings;
    ASSERT_TRUE(create_state_tracked_object_record(buffer, buffer.get<VkBufferCreateInfo>(), expectedBufferBindings));
    expectedBufferBindings[gvk::get_state_tracked_object(buffer)].mBindBufferMemoryInfo = bindBufferMemoryInfo;

    std::map<GvkStateTrackedObject, ObjectRecord> expectedDeviceMemoryBindings;
    ASSERT_TRUE(create_state_tracked_object_record(deviceMemory, deviceMemory.get<VkMemoryAllocateInfo>(), expectedDeviceMemoryBindings));
    expectedDeviceMemoryBindings[gvk::get_state_tracked_object(deviceMemory)].mBindBufferMemoryInfo = bindBufferMemoryInfo;

    enumerator.records.clear();
    auto stateTrackedBuffer = gvk::get_state_tracked_object(buffer);
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedBuffer, &enumerateInfo);
    validate(gvk_file_line, expectedBufferBindings, enumerator.records);

    enumerator.records.clear();
    auto stateTrackedDeviceMemory = gvk::get_state_tracked_object(deviceMemory);
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedDeviceMemory, &enumerateInfo);
    validate(gvk_file_line, expectedDeviceMemoryBindings, enumerator.records);

    expectedInstanceObjects.erase(stateTrackedDeviceMemory);
    deviceMemory.reset();

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedBuffer, &enumerateInfo);
    validate(gvk_file_line, expectedBufferBindings, enumerator.records);
}

TEST(DeviceMemoryBindingTracking, BindBufferMemory2)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
    bufferCreateInfo.size = 64;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    gvk::Buffer buffer;
    ASSERT_EQ(gvk::Buffer::create(context.get_devices()[0], &bufferCreateInfo, (const VkAllocationCallbacks*)nullptr, &buffer), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(buffer, buffer.get<VkBufferCreateInfo>(), expectedInstanceObjects));

    auto bufferMemoryRequirementsInfo = gvk::get_default<VkBufferMemoryRequirementsInfo2>();
    bufferMemoryRequirementsInfo.buffer = buffer;
    auto memoryRequirements = gvk::get_default<VkMemoryRequirements2>();
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkGetBufferMemoryRequirements2);
    dispatchTable.gvkGetBufferMemoryRequirements2(buffer.get<gvk::Device>(), &bufferMemoryRequirementsInfo, &memoryRequirements);
    auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    uint32_t memoryTypeCount = VK_MAX_MEMORY_TYPES;
    std::array<uint32_t, VK_MAX_MEMORY_TYPES> memoryTypeIndices;
    gvk::get_compatible_memory_type_indices(context.get_devices()[0].get<gvk::PhysicalDevice>(), memoryRequirements.memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, memoryTypeIndices.data());
    ASSERT_TRUE(1 <= memoryTypeCount);

    auto memoryAllocateInfo = gvk::get_default<VkMemoryAllocateInfo>();
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndices[0];
    memoryAllocateInfo.allocationSize = memoryRequirements.memoryRequirements.size;
    gvk::DeviceMemory deviceMemory;
    ASSERT_EQ(gvk::DeviceMemory::allocate(buffer.get<gvk::Device>(), &memoryAllocateInfo, nullptr, &deviceMemory), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(deviceMemory, deviceMemory.get<VkMemoryAllocateInfo>(), expectedInstanceObjects));

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    auto bindBufferMemoryInfo = gvk::get_default<VkBindBufferMemoryInfo>();
    bindBufferMemoryInfo.buffer = buffer;
    bindBufferMemoryInfo.memory = deviceMemory;
    assert(dispatchTable.gvkBindBufferMemory2);
    ASSERT_EQ(dispatchTable.gvkBindBufferMemory2(buffer.get<gvk::Device>(), 1, &bindBufferMemoryInfo), VK_SUCCESS);

    std::map<GvkStateTrackedObject, ObjectRecord> expectedBufferBindings;
    ASSERT_TRUE(create_state_tracked_object_record(buffer, buffer.get<VkBufferCreateInfo>(), expectedBufferBindings));
    expectedBufferBindings[gvk::get_state_tracked_object(buffer)].mBindBufferMemoryInfo = bindBufferMemoryInfo;

    std::map<GvkStateTrackedObject, ObjectRecord> expectedDeviceMemoryBindings;
    ASSERT_TRUE(create_state_tracked_object_record(deviceMemory, deviceMemory.get<VkMemoryAllocateInfo>(), expectedDeviceMemoryBindings));
    expectedDeviceMemoryBindings[gvk::get_state_tracked_object(deviceMemory)].mBindBufferMemoryInfo = bindBufferMemoryInfo;

    enumerator.records.clear();
    auto stateTrackedBuffer = gvk::get_state_tracked_object(buffer);
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedBuffer, &enumerateInfo);
    validate(gvk_file_line, expectedBufferBindings, enumerator.records);

    enumerator.records.clear();
    auto stateTrackedDeviceMemory = gvk::get_state_tracked_object(deviceMemory);
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedDeviceMemory, &enumerateInfo);
    validate(gvk_file_line, expectedDeviceMemoryBindings, enumerator.records);

    expectedInstanceObjects.erase(stateTrackedBuffer);
    expectedDeviceMemoryBindings.clear();
    buffer.reset();

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedBuffer, &enumerateInfo);
    validate(gvk_file_line, expectedDeviceMemoryBindings, enumerator.records);
}

TEST(DeviceMemoryBindingTracking, BindImageMemory)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent.width = 256;
    imageCreateInfo.extent.height = 256;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    ASSERT_EQ(gvk::Image::create(context.get_devices()[0], &imageCreateInfo, (const VkAllocationCallbacks*)nullptr, &image), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedInstanceObjects));

    VkMemoryRequirements memoryRequirements { };
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkGetImageMemoryRequirements);
    dispatchTable.gvkGetImageMemoryRequirements(image.get<gvk::Device>(), image, &memoryRequirements);
    auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    uint32_t memoryTypeCount = VK_MAX_MEMORY_TYPES;
    std::array<uint32_t, VK_MAX_MEMORY_TYPES> memoryTypeIndices;
    gvk::get_compatible_memory_type_indices(image.get<gvk::Device>().get<gvk::PhysicalDevice>(), memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, memoryTypeIndices.data());
    ASSERT_TRUE(1 <= memoryTypeCount);

    auto memoryAllocateInfo = gvk::get_default<VkMemoryAllocateInfo>();
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndices[0];
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    gvk::DeviceMemory deviceMemory;
    ASSERT_EQ(gvk::DeviceMemory::allocate(image.get<gvk::Device>(), &memoryAllocateInfo, nullptr, &deviceMemory), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(deviceMemory, deviceMemory.get<VkMemoryAllocateInfo>(), expectedInstanceObjects));
    assert(dispatchTable.gvkBindImageMemory);
    ASSERT_EQ(dispatchTable.gvkBindImageMemory(image.get<gvk::Device>(), image, deviceMemory, 0), VK_SUCCESS);

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    auto bindImageMemoryInfo = gvk::get_default<VkBindImageMemoryInfo>();
    bindImageMemoryInfo.image = image;
    bindImageMemoryInfo.memory = deviceMemory;

    std::map<GvkStateTrackedObject, ObjectRecord> expectedImageBindings;
    ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedImageBindings));
    expectedImageBindings[gvk::get_state_tracked_object(image)].mBindImageMemoryInfo = bindImageMemoryInfo;

    std::map<GvkStateTrackedObject, ObjectRecord> expectedDeviceMemoryBindings;
    ASSERT_TRUE(create_state_tracked_object_record(deviceMemory, deviceMemory.get<VkMemoryAllocateInfo>(), expectedDeviceMemoryBindings));
    expectedDeviceMemoryBindings[gvk::get_state_tracked_object(deviceMemory)].mBindImageMemoryInfo = bindImageMemoryInfo;

    enumerator.records.clear();
    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedImage, &enumerateInfo);
    validate(gvk_file_line, expectedImageBindings, enumerator.records);

    enumerator.records.clear();
    auto stateTrackedDeviceMemory = gvk::get_state_tracked_object(deviceMemory);
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedDeviceMemory, &enumerateInfo);
    validate(gvk_file_line, expectedDeviceMemoryBindings, enumerator.records);

    expectedInstanceObjects.erase(stateTrackedDeviceMemory);
    deviceMemory.reset();

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedImage, &enumerateInfo);
    validate(gvk_file_line, expectedImageBindings, enumerator.records);
}

TEST(DeviceMemoryBindingTracking, BindImageMemory2)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent.width = 256;
    imageCreateInfo.extent.height = 256;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    ASSERT_EQ(gvk::Image::create(context.get_devices()[0], &imageCreateInfo, (const VkAllocationCallbacks*)nullptr, &image), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedInstanceObjects));

    auto imageMemoryRequirementsInfo = gvk::get_default<VkImageMemoryRequirementsInfo2>();
    imageMemoryRequirementsInfo.image = image;
    auto memoryRequirements = gvk::get_default<VkMemoryRequirements2>();
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkGetImageMemoryRequirements2);
    dispatchTable.gvkGetImageMemoryRequirements2(image.get<gvk::Device>(), &imageMemoryRequirementsInfo, &memoryRequirements);
    auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    uint32_t memoryTypeCount = VK_MAX_MEMORY_TYPES;
    std::array<uint32_t, VK_MAX_MEMORY_TYPES> memoryTypeIndices;
    gvk::get_compatible_memory_type_indices(context.get_devices()[0].get<gvk::PhysicalDevice>(), memoryRequirements.memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, memoryTypeIndices.data());
    ASSERT_TRUE(1 <= memoryTypeCount);

    auto memoryAllocateInfo = gvk::get_default<VkMemoryAllocateInfo>();
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndices[0];
    memoryAllocateInfo.allocationSize = memoryRequirements.memoryRequirements.size;
    gvk::DeviceMemory deviceMemory;
    ASSERT_EQ(gvk::DeviceMemory::allocate(image.get<gvk::Device>(), &memoryAllocateInfo, nullptr, &deviceMemory), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(deviceMemory, deviceMemory.get<VkMemoryAllocateInfo>(), expectedInstanceObjects));

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    auto bindImageMemoryInfo = gvk::get_default<VkBindImageMemoryInfo>();
    bindImageMemoryInfo.image = image;
    bindImageMemoryInfo.memory = deviceMemory;
    assert(dispatchTable.gvkBindImageMemory2);
    ASSERT_EQ(dispatchTable.gvkBindImageMemory2(image.get<gvk::Device>(), 1, &bindImageMemoryInfo), VK_SUCCESS);

    std::map<GvkStateTrackedObject, ObjectRecord> expectedImageBindings;
    ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedImageBindings));
    expectedImageBindings[gvk::get_state_tracked_object(image)].mBindImageMemoryInfo = bindImageMemoryInfo;

    std::map<GvkStateTrackedObject, ObjectRecord> expectedDeviceMemoryBindings;
    ASSERT_TRUE(create_state_tracked_object_record(deviceMemory, deviceMemory.get<VkMemoryAllocateInfo>(), expectedDeviceMemoryBindings));
    expectedDeviceMemoryBindings[gvk::get_state_tracked_object(deviceMemory)].mBindImageMemoryInfo = bindImageMemoryInfo;

    enumerator.records.clear();
    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedImage, &enumerateInfo);
    validate(gvk_file_line, expectedImageBindings, enumerator.records);

    enumerator.records.clear();
    auto stateTrackedDeviceMemory = gvk::get_state_tracked_object(deviceMemory);
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedDeviceMemory, &enumerateInfo);
    validate(gvk_file_line, expectedDeviceMemoryBindings, enumerator.records);

    expectedInstanceObjects.erase(stateTrackedImage);
    expectedDeviceMemoryBindings.clear();
    image.reset();

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedImage, &enumerateInfo);
    validate(gvk_file_line, expectedDeviceMemoryBindings, enumerator.records);
}
