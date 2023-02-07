
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

// TODO : Need to setup tests that exercise complex vkUpdateDescriptorSets() logic
// TODO : Immutable samplers
// TODO : vkUpdateDescriptorSetWithTemplate

static void create_descriptor_set_layouts(const gvk::Device& device, gvk::spirv::ShaderInfo shaderInfo, std::vector<gvk::DescriptorSetLayout>& descriptorSetLayouts)
{
    gvk::spirv::Context spirvContext;
    ASSERT_EQ(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext), VK_SUCCESS);
    ASSERT_EQ(spirvContext.compile(&shaderInfo), VK_SUCCESS);
    ASSERT_FALSE(shaderInfo.spirv.empty());

    auto shaderMoudleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
    shaderMoudleCreateInfo.codeSize = (uint32_t)shaderInfo.spirv.size() * sizeof(uint32_t);
    shaderMoudleCreateInfo.pCode = shaderInfo.spirv.data();
    gvk::ShaderModule shaderModule;
    ASSERT_EQ(gvk::ShaderModule::create(device, &shaderMoudleCreateInfo, nullptr, &shaderModule), VK_SUCCESS);

    gvk::spirv::BindingInfo bindingInfo;
    bindingInfo.add_shader(shaderInfo);
    uint32_t descriptorSetLayoutCount = 0;
    ASSERT_EQ(gvk::spirv::create_descriptor_set_layouts(device, bindingInfo, nullptr, &descriptorSetLayoutCount, nullptr), VK_SUCCESS);
    descriptorSetLayouts.resize(descriptorSetLayoutCount);
    ASSERT_EQ(gvk::spirv::create_descriptor_set_layouts(device, bindingInfo, nullptr, &descriptorSetLayoutCount, descriptorSetLayouts.data()), VK_SUCCESS);
}

static void create_descriptor_pool(const gvk::Device& device, const std::vector<gvk::DescriptorSetLayout>& descriptorSetLayouts, gvk::DescriptorPool* pDescriptorPool)
{
    std::array<VkDescriptorPoolSize, 17> descriptorPoolSizes { };
    descriptorPoolSizes[ 0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorPoolSizes[ 1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[ 2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorPoolSizes[ 3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorPoolSizes[ 4].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    descriptorPoolSizes[ 5].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    descriptorPoolSizes[ 6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSizes[ 7].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSizes[ 8].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptorPoolSizes[ 9].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    descriptorPoolSizes[10].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptorPoolSizes[11].type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    descriptorPoolSizes[12].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    descriptorPoolSizes[13].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    descriptorPoolSizes[14].type = VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM;
    descriptorPoolSizes[15].type = VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM;
    descriptorPoolSizes[16].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    for (const auto& descriptorSetLayout : descriptorSetLayouts) {
        auto descriptorSetLayoutCreateInfo = descriptorSetLayout.get<VkDescriptorSetLayoutCreateInfo>();
        ASSERT_TRUE(descriptorSetLayoutCreateInfo.bindingCount && descriptorSetLayoutCreateInfo.pBindings);
        for (uint32_t i = 0; i < descriptorSetLayoutCreateInfo.bindingCount; ++i) {
            const auto& binding = descriptorSetLayoutCreateInfo.pBindings[i];
            ASSERT_TRUE(binding.descriptorCount);
            switch (binding.descriptorType) {
            case VK_DESCRIPTOR_TYPE_SAMPLER:                   { descriptorPoolSizes[ 0].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:    { descriptorPoolSizes[ 1].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:             { descriptorPoolSizes[ 2].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:             { descriptorPoolSizes[ 3].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:      { descriptorPoolSizes[ 4].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:      { descriptorPoolSizes[ 5].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:            { descriptorPoolSizes[ 6].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:            { descriptorPoolSizes[ 7].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:    { descriptorPoolSizes[ 8].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:    { descriptorPoolSizes[ 9].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:          { descriptorPoolSizes[10].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:      { descriptorPoolSizes[11].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:{ descriptorPoolSizes[12].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: { descriptorPoolSizes[13].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:  { descriptorPoolSizes[14].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:    { descriptorPoolSizes[15].descriptorCount += binding.descriptorCount; } break;
            case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:               { descriptorPoolSizes[16].descriptorCount += binding.descriptorCount; } break;
            default:
            {
                FAIL();
            } break;
            }
        }
    }
    size_t descriptorPoolSize_i = 0;
    uint32_t descriptorPoolSizeCount = 0;
    for (size_t i = 0; i < descriptorPoolSizes.size(); ++i) {
        if (descriptorPoolSizes[i].descriptorCount) {
            if (descriptorPoolSize_i != i) {
                ASSERT_TRUE(descriptorPoolSize_i < i);
                ASSERT_FALSE(descriptorPoolSizes[descriptorPoolSize_i].descriptorCount);
                std::swap(descriptorPoolSizes[descriptorPoolSize_i], descriptorPoolSizes[i]);
            }
            ++descriptorPoolSizeCount;
            ++descriptorPoolSize_i;
        }
    }
    ASSERT_TRUE(descriptorPoolSizeCount);
    auto descriptorPoolCreateInfo = gvk::get_default<VkDescriptorPoolCreateInfo>();
    descriptorPoolCreateInfo.maxSets = (uint32_t)descriptorSetLayouts.size();
    descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizeCount;
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    ASSERT_EQ(gvk::DescriptorPool::create(device, &descriptorPoolCreateInfo, nullptr, pDescriptorPool), VK_SUCCESS);
}

TEST(DescriptorSet, BasicDescriptorBinding)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    load_gvk_state_tracker_entry_points();
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    auto shaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
    shaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderInfo.lineOffset = __LINE__;
    shaderInfo.source = R"(
        #version 450

        layout(local_size_x = 1, local_size_y = 1) in;
        layout(set = 0, binding = 0, rgba32f) uniform writeonly image2D image;

        void main()
        {
        }
    )";
    std::vector<gvk::DescriptorSetLayout> descriptorSetLayouts;
    create_descriptor_set_layouts(context.get_devices()[0], shaderInfo, descriptorSetLayouts);
    ASSERT_EQ(descriptorSetLayouts.size(), 1);
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[0], descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>(), expectedInstanceObjects));

    gvk::DescriptorPool descriptorPool;
    create_descriptor_pool(context.get_devices()[0], descriptorSetLayouts, &descriptorPool);
    ASSERT_TRUE(create_state_tracked_object_record(descriptorPool, descriptorPool.get<VkDescriptorPoolCreateInfo>(), expectedInstanceObjects));

    auto descriptorSetAllocateInfo = gvk::get_default<VkDescriptorSetAllocateInfo>();
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayouts[0].get<const VkDescriptorSetLayout&>();
    gvk::DescriptorSet descriptorSet;
    ASSERT_EQ(gvk::DescriptorSet::allocate(context.get_devices()[0], &descriptorSetAllocateInfo, &descriptorSet), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSet, descriptorSet.get<VkDescriptorSetAllocateInfo>(), expectedInstanceObjects));

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent.width = 256;
    imageCreateInfo.extent.height = 256;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    auto allocationCreateInfo = gvk::get_default<VmaAllocationCreateInfo>();
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    gvk::Image image;
    ASSERT_EQ(gvk::Image::create(context.get_devices()[0], &imageCreateInfo, &allocationCreateInfo, &image), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedInstanceObjects));

    VmaAllocationInfo allocationInfo { };
    auto vmaAllocator = context.get_devices()[0].get<VmaAllocator>();
    vmaGetAllocationInfo(vmaAllocator, image.get<VmaAllocation>(), &allocationInfo);

    GvkStateTrackedObject stateTrackedDeviceMemory { };
    stateTrackedDeviceMemory.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
    stateTrackedDeviceMemory.handle = (uint64_t)allocationInfo.deviceMemory;
    stateTrackedDeviceMemory.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
    VkStructureType allocateInfoType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    pfnGvkGetStateTrackedObjectAllocateInfo(&stateTrackedDeviceMemory, &allocateInfoType, nullptr);
    ASSERT_EQ(allocateInfoType, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    auto memoryAllocateInfo = gvk::get_default<VkMemoryAllocateInfo>();
    pfnGvkGetStateTrackedObjectAllocateInfo(&stateTrackedDeviceMemory, &allocateInfoType, (VkBaseOutStructure*)&memoryAllocateInfo);
    auto& stateTrackedDeviceMemoryRecord = expectedInstanceObjects[stateTrackedDeviceMemory];
    stateTrackedDeviceMemoryRecord.mStateTrackedObject = stateTrackedDeviceMemory;
    stateTrackedDeviceMemoryRecord.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
    stateTrackedDeviceMemoryRecord.mMemoryAllocateInfo = memoryAllocateInfo;

    auto imageViewCreateInfo = gvk::get_default<VkImageViewCreateInfo>();
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = imageCreateInfo.format;
    gvk::ImageView imageView;
    ASSERT_EQ(gvk::ImageView::create(context.get_devices()[0], &imageViewCreateInfo, nullptr, &imageView), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(imageView, imageView.get<VkImageViewCreateInfo>(), expectedInstanceObjects));

    auto descriptorImageInfo = gvk::get_default<VkDescriptorImageInfo>();
    descriptorImageInfo.imageView = imageView;
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto descriptorWrite = gvk::get_default<VkWriteDescriptorSet>();
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.descriptorCount = 1;
    auto descriptorSetLayoutCreateInfo = descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>();
    ASSERT_EQ(descriptorSetLayoutCreateInfo.bindingCount, (uint32_t)1);
    ASSERT_TRUE(descriptorSetLayoutCreateInfo.pBindings);
    descriptorWrite.descriptorType = descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>().pBindings[0].descriptorType;
    descriptorWrite.pImageInfo = &descriptorImageInfo;
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkUpdateDescriptorSets);
    dispatchTable.gvkUpdateDescriptorSets(context.get_devices()[0], 1, &descriptorWrite, 0, nullptr);

    std::map<GvkStateTrackedObject, ObjectRecord> expectedDescriptorSetDependencies;
    ASSERT_TRUE(create_state_tracked_object_record(descriptorPool, descriptorPool.get<VkDescriptorPoolCreateInfo>(), expectedDescriptorSetDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[0], descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>(), expectedDescriptorSetDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_devices()[0], context.get_devices()[0].get<VkDeviceCreateInfo>(), expectedDescriptorSetDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_physical_devices()[0], context.get_instance().get<VkInstanceCreateInfo>(), expectedDescriptorSetDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_instance(), context.get_instance().get<VkInstanceCreateInfo>(), expectedDescriptorSetDependencies));

    std::map<GvkStateTrackedObject, ObjectRecord> expectedDescriptorSetBindings;
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSet, descriptorSet.get<VkDescriptorSetAllocateInfo>(), expectedDescriptorSetBindings));
    expectedDescriptorSetBindings[gvk::get_state_tracked_object(descriptorSet)].mDescriptorSetBindingInfos.push_back(descriptorWrite);

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    auto stateTrackedDescriptorSet = gvk::get_state_tracked_object(descriptorSet);
    pfnGvkEnumerateStateTrackedObjectDependencies(&stateTrackedDescriptorSet, &enumerateInfo);
    validate(gvk_file_line, expectedDescriptorSetDependencies, enumerator.records);

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjectBindings(&stateTrackedDescriptorSet, &enumerateInfo);
    validate(gvk_file_line, expectedDescriptorSetBindings, enumerator.records);
}

TEST(DescriptorSet, DescriptorSetResourceLifetime)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    load_gvk_state_tracker_entry_points();
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    auto shaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
    shaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderInfo.lineOffset = __LINE__;
    shaderInfo.source = R"(
        #version 450

        layout(local_size_x = 1, local_size_y = 1) in;
        layout(set = 0, binding = 0, rgba32f) uniform writeonly image2D image;

        void main()
        {
        }
    )";
    std::vector<gvk::DescriptorSetLayout> descriptorSetLayouts;
    create_descriptor_set_layouts(context.get_devices()[0], shaderInfo, descriptorSetLayouts);
    ASSERT_EQ(descriptorSetLayouts.size(), 1);
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[0], descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>(), expectedInstanceObjects));
    descriptorSetLayouts.resize(8);
    for (auto& descriptorSetLayout : descriptorSetLayouts) {
        descriptorSetLayout = descriptorSetLayouts[0];
    }

    gvk::DescriptorPool descriptorPool;
    create_descriptor_pool(context.get_devices()[0], descriptorSetLayouts, &descriptorPool);
    ASSERT_TRUE(create_state_tracked_object_record(descriptorPool, descriptorPool.get<VkDescriptorPoolCreateInfo>(), expectedInstanceObjects));

    auto vkDescriptorSetLayouts = gvk::get_vk_handles(descriptorSetLayouts);
    auto descriptorSetAllocateInfo = gvk::get_default<VkDescriptorSetAllocateInfo>();
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)vkDescriptorSetLayouts.size();
    descriptorSetAllocateInfo.pSetLayouts = vkDescriptorSetLayouts.data();
    std::vector<VkDescriptorSet> vkDescriptorSets(descriptorSetAllocateInfo.descriptorSetCount);
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkAllocateDescriptorSets);
    ASSERT_EQ(dispatchTable.gvkAllocateDescriptorSets(context.get_devices()[0], &descriptorSetAllocateInfo, vkDescriptorSets.data()), VK_SUCCESS);
    for (auto vkDescriptorSet : vkDescriptorSets) {
        GvkStateTrackedObject stateTrackedDescriptorSet { };
        stateTrackedDescriptorSet.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
        stateTrackedDescriptorSet.handle = (uint64_t)vkDescriptorSet;
        stateTrackedDescriptorSet.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
        ASSERT_TRUE(create_state_tracked_object_record(stateTrackedDescriptorSet, descriptorSetAllocateInfo, expectedInstanceObjects));
    }

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    std::vector<VkDescriptorSet> vkDescriptorSetsToFree(3);
    ASSERT_LT(vkDescriptorSetsToFree.size(), vkDescriptorSets.size());
    for (auto& vkDescriptorSetToFree : vkDescriptorSetsToFree) {
        GvkStateTrackedObject stateTrackedDescriptorSet { };
        stateTrackedDescriptorSet.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
        stateTrackedDescriptorSet.handle = (uint64_t)vkDescriptorSets.back();
        stateTrackedDescriptorSet.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
        expectedInstanceObjects.erase(stateTrackedDescriptorSet);
        vkDescriptorSetToFree = vkDescriptorSets.back();
        vkDescriptorSets.pop_back();
    }
    assert(dispatchTable.gvkFreeDescriptorSets);
    ASSERT_EQ(dispatchTable.gvkFreeDescriptorSets(context.get_devices()[0], descriptorPool, (uint32_t)vkDescriptorSetsToFree.size(), vkDescriptorSetsToFree.data()), VK_SUCCESS);

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    for (auto& vkDescriptorSet : vkDescriptorSets) {
        GvkStateTrackedObject stateTrackedDescriptorSet { };
        stateTrackedDescriptorSet.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
        stateTrackedDescriptorSet.handle = (uint64_t)vkDescriptorSet;
        stateTrackedDescriptorSet.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
        expectedInstanceObjects.erase(stateTrackedDescriptorSet);
    }
    assert(dispatchTable.gvkResetDescriptorPool);
    ASSERT_EQ(dispatchTable.gvkResetDescriptorPool(context.get_devices()[0], descriptorPool, 0), VK_SUCCESS);

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    vkDescriptorSets.resize(descriptorSetAllocateInfo.descriptorSetCount);
    assert(dispatchTable.gvkAllocateDescriptorSets);
    ASSERT_EQ(dispatchTable.gvkAllocateDescriptorSets(context.get_devices()[0], &descriptorSetAllocateInfo, vkDescriptorSets.data()), VK_SUCCESS);
    for (auto vkDescriptorSet : vkDescriptorSets) {
        GvkStateTrackedObject stateTrackedDescriptorSet { };
        stateTrackedDescriptorSet.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
        stateTrackedDescriptorSet.handle = (uint64_t)vkDescriptorSet;
        stateTrackedDescriptorSet.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
        ASSERT_TRUE(create_state_tracked_object_record(stateTrackedDescriptorSet, descriptorSetAllocateInfo, expectedInstanceObjects));
    }

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    for (auto& vkDescriptorSet : vkDescriptorSets) {
        GvkStateTrackedObject stateTrackedDescriptorSet { };
        stateTrackedDescriptorSet.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
        stateTrackedDescriptorSet.handle = (uint64_t)vkDescriptorSet;
        stateTrackedDescriptorSet.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
        expectedInstanceObjects.erase(stateTrackedDescriptorSet);
    }
    expectedInstanceObjects.erase(gvk::get_state_tracked_object(descriptorPool));
    descriptorPool.reset();

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);
}
