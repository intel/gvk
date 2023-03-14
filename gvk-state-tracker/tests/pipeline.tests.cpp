
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

// TODO : RayTracingPipelineResourceLifetime

TEST(Pipeline, ComputePipelineResourceLifetime)
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

    gvk::spirv::Context spirvContext;
    ASSERT_EQ(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext), VK_SUCCESS);
    ASSERT_EQ(spirvContext.compile(&shaderInfo), VK_SUCCESS);
    ASSERT_FALSE(shaderInfo.spirv.empty());

    auto shaderMoudleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
    shaderMoudleCreateInfo.codeSize = (uint32_t)shaderInfo.spirv.size() * sizeof(uint32_t);
    shaderMoudleCreateInfo.pCode = shaderInfo.spirv.data();
    gvk::ShaderModule shaderModule;
    ASSERT_EQ(gvk::ShaderModule::create(context.get_devices()[0], &shaderMoudleCreateInfo, nullptr, &shaderModule), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(shaderModule, shaderModule.get<VkShaderModuleCreateInfo>(), expectedInstanceObjects));

    auto bindingInfo = gvk::get_default<gvk::spirv::BindingInfo>();
    bindingInfo.add_shader(shaderInfo);
    gvk::PipelineLayout pipelineLayout;
    ASSERT_EQ(gvk::spirv::create_pipeline_layout(context.get_devices()[0], bindingInfo, nullptr, &pipelineLayout), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(pipelineLayout, pipelineLayout.get<VkPipelineLayoutCreateInfo>(), expectedInstanceObjects));
    auto descriptorSetLayouts = pipelineLayout.get<gvk::DescriptorSetLayouts>();
    ASSERT_EQ(descriptorSetLayouts.size(), 1);
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[0], descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>(), expectedInstanceObjects));

    auto computePipelineCreateInfo = gvk::get_default<VkComputePipelineCreateInfo>();
    computePipelineCreateInfo.stage.module = shaderModule;
    computePipelineCreateInfo.layout = pipelineLayout;
    VkPipeline vkPipeline = VK_NULL_HANDLE;
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkCreateComputePipelines);
    ASSERT_EQ(dispatchTable.gvkCreateComputePipelines(context.get_devices()[0], VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &vkPipeline), VK_SUCCESS);
    auto stateTrackedPipeline = gvk::get_default<GvkStateTrackedObject>();
    stateTrackedPipeline.type = VK_OBJECT_TYPE_PIPELINE;
    stateTrackedPipeline.handle = (uint64_t)vkPipeline;
    stateTrackedPipeline.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
    ASSERT_TRUE(expectedInstanceObjects.insert({ stateTrackedPipeline, ObjectRecord(stateTrackedPipeline, computePipelineCreateInfo, GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT) }).second);

    std::map<GvkStateTrackedObject, ObjectRecord> expectedPipelineDependencies;
    ASSERT_TRUE(create_state_tracked_object_record(shaderModule, shaderModule.get<VkShaderModuleCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(pipelineLayout, pipelineLayout.get<VkPipelineLayoutCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[0], descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_devices()[0], context.get_devices()[0].get<VkDeviceCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_physical_devices()[0], VkApplicationInfo { }, expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_instance(), context.get_instance().get<VkInstanceCreateInfo>(), expectedPipelineDependencies));

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjectDependencies(&stateTrackedPipeline, &enumerateInfo);
    validate(gvk_file_line, expectedPipelineDependencies, enumerator.records);

    expectedInstanceObjects.erase(gvk::get_state_tracked_object(shaderModule));
    expectedInstanceObjects.erase(gvk::get_state_tracked_object(pipelineLayout));
    expectedInstanceObjects.erase(gvk::get_state_tracked_object(descriptorSetLayouts[0]));
    expectedPipelineDependencies[gvk::get_state_tracked_object(shaderModule)].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    expectedPipelineDependencies[gvk::get_state_tracked_object(pipelineLayout)].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    expectedPipelineDependencies[gvk::get_state_tracked_object(descriptorSetLayouts[0])].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    shaderModule.reset();
    pipelineLayout.reset();
    descriptorSetLayouts.clear();

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjectDependencies(&stateTrackedPipeline, &enumerateInfo);
    validate(gvk_file_line, expectedPipelineDependencies, enumerator.records);

    assert(dispatchTable.gvkDestroyPipeline);
    dispatchTable.gvkDestroyPipeline(context.get_devices()[0], vkPipeline, nullptr);
}

TEST(Pipeline, GraphicsPipelineResourceLifetime)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    load_gvk_state_tracker_entry_points();
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    // Setup color VkAttachmentDescription2
    auto colorAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
    colorAttachmentDescription.format = get_render_pass_color_format(context);
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Setup color VkAttachmentReference2
    auto colorAttachmentReference = gvk::get_default<VkAttachmentReference2>();
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentReference.aspectMask = gvk::get_image_aspect_flags(colorAttachmentDescription.format);

    // Setup VkSubpassDescription2
    auto subpassDescription = gvk::get_default<VkSubpassDescription2>();
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    // Setup VkSubpassDependency2 for a gvk::RenderPass with a single, 1 sample, color attachment
    auto subpassDependency = gvk::get_default<VkSubpassDependency2>();
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create gvk::RenderPass
    auto renderPassCreateInfo = gvk::get_default<VkRenderPassCreateInfo2>();
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;
    gvk::RenderPass renderPass;
    ASSERT_EQ(gvk::RenderPass::create(context.get_devices()[0], &renderPassCreateInfo, nullptr, &renderPass), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(renderPass, renderPass.get<VkRenderPassCreateInfo2>(), expectedInstanceObjects));

    auto vertexShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
    vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderInfo.lineOffset = __LINE__;
    vertexShaderInfo.source = R"(
        #version 450

        layout(set = 0, binding = 0)
        uniform CameraUniformBuffer
        {
            mat4 view;
            mat4 projection;
        } camera;

        layout(set = 1, binding = 0)
        uniform ObjectUniformBuffer
        {
            mat4 world;
            vec4 color;
        } object;

        layout(location = 0) in vec3 vsPosition;

        out gl_PerVertex
        {
            vec4 gl_Position;
        };

        void main()
        {
            gl_Position = camera.projection * camera.view * object.world * vec4(vsPosition, 1);
        }
    )";

    auto fragmentShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
    fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderInfo.lineOffset = __LINE__;
    fragmentShaderInfo.source = R"(
        #version 450

        layout(set = 1, binding = 0)
        uniform ObjectUniformBuffer
        {
            mat4 world;
            vec4 color;
        } object;

        layout(location = 0) out vec4 fragColor;

        void main()
        {
            fragColor = object.color;
        }
    )";

    gvk::spirv::Context spirvContext;
    ASSERT_EQ(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext), VK_SUCCESS);
    ASSERT_EQ(spirvContext.compile(&vertexShaderInfo), VK_SUCCESS);
    ASSERT_FALSE(vertexShaderInfo.spirv.empty());
    ASSERT_EQ(spirvContext.compile(&fragmentShaderInfo), VK_SUCCESS);
    ASSERT_FALSE(fragmentShaderInfo.spirv.empty());

    auto vertexShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
    vertexShaderModuleCreateInfo.codeSize = (uint32_t)vertexShaderInfo.spirv.size() * sizeof(uint32_t);
    vertexShaderModuleCreateInfo.pCode = vertexShaderInfo.spirv.data();
    gvk::ShaderModule vertexShaderModule;
    ASSERT_EQ(gvk::ShaderModule::create(context.get_devices()[0], &vertexShaderModuleCreateInfo, nullptr, &vertexShaderModule), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(vertexShaderModule, vertexShaderModule.get<VkShaderModuleCreateInfo>(), expectedInstanceObjects));

    auto fragmentShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
    fragmentShaderModuleCreateInfo.codeSize = (uint32_t)fragmentShaderInfo.spirv.size() * sizeof(uint32_t);
    fragmentShaderModuleCreateInfo.pCode = fragmentShaderInfo.spirv.data();
    gvk::ShaderModule fragmentShaderModule;
    ASSERT_EQ(gvk::ShaderModule::create(context.get_devices()[0], &fragmentShaderModuleCreateInfo, nullptr, &fragmentShaderModule), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(fragmentShaderModule, fragmentShaderModule.get<VkShaderModuleCreateInfo>(), expectedInstanceObjects));

    auto bindingInfo = gvk::get_default<gvk::spirv::BindingInfo>();
    bindingInfo.add_shader(vertexShaderInfo);
    bindingInfo.add_shader(fragmentShaderInfo);
    gvk::PipelineLayout pipelineLayout;
    ASSERT_EQ(gvk::spirv::create_pipeline_layout(context.get_devices()[0], bindingInfo, nullptr, &pipelineLayout), VK_SUCCESS);
    ASSERT_TRUE(create_state_tracked_object_record(pipelineLayout, pipelineLayout.get<VkPipelineLayoutCreateInfo>(), expectedInstanceObjects));
    auto descriptorSetLayouts = pipelineLayout.get<gvk::DescriptorSetLayouts>();
    ASSERT_EQ(descriptorSetLayouts.size(), 2);
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[0], descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>(), expectedInstanceObjects));
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[1], descriptorSetLayouts[1].get<VkDescriptorSetLayoutCreateInfo>(), expectedInstanceObjects));

    std::array<VkPipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos { };
    pipelineShaderStageCreateInfos[0] = gvk::get_default<VkPipelineShaderStageCreateInfo>();
    pipelineShaderStageCreateInfos[0].stage = vertexShaderInfo.stage;
    pipelineShaderStageCreateInfos[0].module = vertexShaderModule;
    pipelineShaderStageCreateInfos[1] = gvk::get_default<VkPipelineShaderStageCreateInfo>();
    pipelineShaderStageCreateInfos[1].stage = fragmentShaderInfo.stage;
    pipelineShaderStageCreateInfos[1].module = fragmentShaderModule;

    auto vertexInputBindingDescription = gvk::get_default<VkVertexInputBindingDescription>();
    vertexInputBindingDescription.stride = sizeof(glm::vec3);
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    auto vertexInputAttributeDescriptions = gvk::get_vertex_description<glm::vec3>(0);
    auto pipelineVertexInputStateCreateInfo = gvk::get_default<VkPipelineVertexInputStateCreateInfo>();
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions.size();
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

    auto graphicsPipelineCreateInfo = gvk::get_default<VkGraphicsPipelineCreateInfo>();
    graphicsPipelineCreateInfo.stageCount = (uint32_t)pipelineShaderStageCreateInfos.size();
    graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();
    graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.layout = pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = renderPass;
    VkPipeline vkPipeline = VK_NULL_HANDLE;
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkCreateGraphicsPipelines);
    ASSERT_EQ(dispatchTable.gvkCreateGraphicsPipelines(context.get_devices()[0], VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &vkPipeline), VK_SUCCESS);
    auto stateTrackedPipeline = gvk::get_default<GvkStateTrackedObject>();
    stateTrackedPipeline.type = VK_OBJECT_TYPE_PIPELINE;
    stateTrackedPipeline.handle = (uint64_t)vkPipeline;
    stateTrackedPipeline.dispatchableHandle = (uint64_t)(VkDevice)context.get_devices()[0];
    ASSERT_TRUE(expectedInstanceObjects.insert({ stateTrackedPipeline, ObjectRecord(stateTrackedPipeline, graphicsPipelineCreateInfo, GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT) }).second);

    std::map<GvkStateTrackedObject, ObjectRecord> expectedPipelineDependencies;
    ASSERT_TRUE(create_state_tracked_object_record(vertexShaderModule, vertexShaderModule.get<VkShaderModuleCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(fragmentShaderModule, fragmentShaderModule.get<VkShaderModuleCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(renderPass, renderPass.get<VkRenderPassCreateInfo2>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(pipelineLayout, pipelineLayout.get<VkPipelineLayoutCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[0], descriptorSetLayouts[0].get<VkDescriptorSetLayoutCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(descriptorSetLayouts[1], descriptorSetLayouts[1].get<VkDescriptorSetLayoutCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_devices()[0], context.get_devices()[0].get<VkDeviceCreateInfo>(), expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_physical_devices()[0], VkApplicationInfo { }, expectedPipelineDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_instance(), context.get_instance().get<VkInstanceCreateInfo>(), expectedPipelineDependencies));

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjectDependencies(&stateTrackedPipeline, &enumerateInfo);
    validate(gvk_file_line, expectedPipelineDependencies, enumerator.records);

    expectedInstanceObjects.erase(gvk::get_state_tracked_object(vertexShaderModule));
    expectedInstanceObjects.erase(gvk::get_state_tracked_object(fragmentShaderModule));
    expectedInstanceObjects.erase(gvk::get_state_tracked_object(renderPass));
    expectedInstanceObjects.erase(gvk::get_state_tracked_object(pipelineLayout));
    expectedInstanceObjects.erase(gvk::get_state_tracked_object(descriptorSetLayouts[0]));
    expectedInstanceObjects.erase(gvk::get_state_tracked_object(descriptorSetLayouts[1]));
    expectedPipelineDependencies[gvk::get_state_tracked_object(vertexShaderModule)].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    expectedPipelineDependencies[gvk::get_state_tracked_object(fragmentShaderModule)].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    expectedPipelineDependencies[gvk::get_state_tracked_object(renderPass)].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    expectedPipelineDependencies[gvk::get_state_tracked_object(pipelineLayout)].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    expectedPipelineDependencies[gvk::get_state_tracked_object(descriptorSetLayouts[0])].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    expectedPipelineDependencies[gvk::get_state_tracked_object(descriptorSetLayouts[1])].mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    vertexShaderModule.reset();
    fragmentShaderModule.reset();
    renderPass.reset();
    pipelineLayout.reset();
    descriptorSetLayouts.clear();

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    enumerator.records.clear();
    pfnGvkEnumerateStateTrackedObjectDependencies(&stateTrackedPipeline, &enumerateInfo);
    validate(gvk_file_line, expectedPipelineDependencies, enumerator.records);

    assert(dispatchTable.gvkDestroyPipeline);
    dispatchTable.gvkDestroyPipeline(context.get_devices()[0], vkPipeline, nullptr);
}
