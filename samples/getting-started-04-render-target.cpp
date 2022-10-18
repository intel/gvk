
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

#include "gvk-sample-utilities.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <vector>

VkResult create_mesh(
    const gvk::Context& context,
    const glm::vec3& dimensions,
    const glm::vec4& topColor,
    const glm::vec4& bottomColor,
    gvk::Mesh* pMesh
)
{
    float w = dimensions[0] * 0.5f;
    float h = dimensions[1] * 0.5f;
    float d = dimensions[2] * 0.5f;
    std::array<VertexPositionTexcoordColor, 24> vertices {
        // Top
        VertexPositionTexcoordColor {{ -w,  h, -d }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{  w,  h, -d }, { 1, 0 }, { topColor }},
        VertexPositionTexcoordColor {{  w,  h,  d }, { 1, 1 }, { topColor }},
        VertexPositionTexcoordColor {{ -w,  h,  d }, { 0, 1 }, { topColor }},
        // Left
        VertexPositionTexcoordColor {{ -w,  h, -d }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{ -w,  h,  d }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{ -w, -h,  d }, { 0, 0 }, { bottomColor }},
        VertexPositionTexcoordColor {{ -w, -h, -d }, { 0, 0 }, { bottomColor }},
        // Front
        VertexPositionTexcoordColor {{ -w,  h,  w }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{  w,  h,  w }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{  w, -h,  w }, { 0, 0 }, { bottomColor }},
        VertexPositionTexcoordColor {{ -w, -h,  w }, { 0, 0 }, { bottomColor }},
        // Right
        VertexPositionTexcoordColor {{  w,  h,  d }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{  w,  h, -d }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{  w, -h, -d }, { 0, 0 }, { bottomColor}},
        VertexPositionTexcoordColor {{  w, -h,  d }, { 0, 0 }, { bottomColor}},
        // Back
        VertexPositionTexcoordColor {{  w,  h, -d }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{ -w,  h, -d }, { 0, 0 }, { topColor }},
        VertexPositionTexcoordColor {{ -w, -h, -d }, { 0, 0 }, { bottomColor }},
        VertexPositionTexcoordColor {{  w, -h, -d }, { 0, 0 }, { bottomColor }},
        // Bottom
        VertexPositionTexcoordColor {{ -w, -h,  d }, { 0, 0 }, { bottomColor }},
        VertexPositionTexcoordColor {{  w, -h,  d }, { 0, 0 }, { bottomColor }},
        VertexPositionTexcoordColor {{  w, -h, -d }, { 0, 0 }, { bottomColor }},
        VertexPositionTexcoordColor {{ -w, -h, -d }, { 0, 0 }, { bottomColor }},
    };
    size_t index_i = 0;
    size_t vertex_i = 0;
    constexpr size_t FaceCount = 6;
    constexpr size_t IndicesPerFace = 6;
    std::array<uint16_t, IndicesPerFace * FaceCount> indices;
    for (size_t face_i = 0; face_i < FaceCount; ++face_i) {
        indices[index_i++] = (uint16_t)(vertex_i + 0);
        indices[index_i++] = (uint16_t)(vertex_i + 1);
        indices[index_i++] = (uint16_t)(vertex_i + 2);
        indices[index_i++] = (uint16_t)(vertex_i + 2);
        indices[index_i++] = (uint16_t)(vertex_i + 3);
        indices[index_i++] = (uint16_t)(vertex_i + 0);
        vertex_i += 4;
    }
    return pMesh->write(
        context.get_devices()[0],
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        (uint32_t)vertices.size(),
        vertices.data(),
        (uint32_t)indices.size(),
        indices.data()
    );
}

int main(int, const char*[])
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        GvkSampleContext context;
        gvk_result(GvkSampleContext::create("Intel GVK - Getting Started - 04 - Render Target", &context));

        // Create a gvk::RenderTarget.  We're going to want to be able to render to
        //  this gvk::RenderTarget and the gvk::WsiManager gvk::RenderTarget objects
        //  using the same gvk::Pipeline objects so the gvk::RenderPass objects need to
        //  be compatible...
        GvkSampleRenderTargetCreateInfo renderTargetCreateInfo{ };
        renderTargetCreateInfo.extent = { 1024, 1024 };
        renderTargetCreateInfo.sampleCount = context.get_wsi_manager().get_sample_count();
        renderTargetCreateInfo.colorFormat = context.get_wsi_manager().get_color_format();
        renderTargetCreateInfo.depthFormat = context.get_wsi_manager().get_depth_format();
        gvk::RenderTarget renderTarget;
        gvk_result(gvk_sample_create_render_target(context, renderTargetCreateInfo, &renderTarget));

        // Create the gvk::Sampler that we'll use when we bind the gvk::RenderTarget
        //  color attachment as a shader resource...
        gvk::Sampler sampler;
        gvk_result(gvk::Sampler::create(context.get_devices()[0], &gvk::get_default<VkSamplerCreateInfo>(), nullptr, &sampler));

        // Create resources for our cube object...this includes a gvk::Mesh, a uniform
        //  gvk::Buffer, a gvk::math::Transform, and a gvk::Pipeline...
        gvk::Mesh cubeMesh;
        gvk_result(create_mesh(context, { 1, 1, 1 }, gvk::math::Color::Black, gvk::math::Color::White, &cubeMesh));
        gvk::Buffer cubeUniformBuffer;
        gvk_result(gvk_sample_create_uniform_buffer<ObjectUniforms>(context, &cubeUniformBuffer));
        gvk::math::Transform cubeTransform;
        gvk::spirv::ShaderInfo vertexShaderInfo{ };
        vertexShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
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
            } object;

            layout(location = 0) in vec3 vsPosition;
            layout(location = 1) in vec2 vsTexCoord;
            layout(location = 2) in vec4 vsColor;
            layout(location = 0) out vec2 fsTexCoord;
            layout(location = 1) out vec4 fsColor;

            out gl_PerVertex
            {
                vec4 gl_Position;
            };

            void main()
            {
                gl_Position = camera.projection * camera.view * object.world * vec4(vsPosition, 1);
                fsTexCoord = vsTexCoord;
                fsColor = vsColor;
            }
        )";
        gvk::spirv::ShaderInfo fragmentShaderInfo{ };
        fragmentShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.lineOffset = __LINE__;
        fragmentShaderInfo.source = R"(
            #version 450

            layout(location = 0) in vec2 fsTexCoord;
            layout(location = 1) in vec4 fsColor;
            layout(location = 0) out vec4 fragColor;

            void main()
            {
                fragColor = fsColor;
            }
        )";
        gvk::Pipeline cubePipeline;
        gvk_result(gvk_sample_create_pipeline<VertexPositionTexcoordColor>(
            renderTarget.get_render_pass(),
            VK_CULL_MODE_NONE,
            vertexShaderInfo,
            fragmentShaderInfo,
            &cubePipeline
        ));

        // Create resources for our floor object...this includes a gvk::Mesh, a uniform
        //  gvk::Buffer, a gvk::math::Transform, and a gvk::Pipeline...
        gvk::Mesh floorMesh;
        gvk_result(create_mesh(context, { 6, 0, 6 }, gvk::math::Color::White, gvk::math::Color::SlateGray, &floorMesh));
        gvk::Buffer floorUniformBuffer;
        gvk_result(gvk_sample_create_uniform_buffer<ObjectUniforms>(context, &floorUniformBuffer));
        gvk::math::Transform floorTransform;
        vertexShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
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
            } object;

            layout(location = 0) in vec3 vsPosition;
            layout(location = 1) in vec2 vsTexcoord;
            layout(location = 2) in vec4 vsColor;
            layout(location = 0) out vec4 fsPosition;
            layout(location = 1) out vec2 fsTexcoord;
            layout(location = 2) out vec4 fsColor;

            out gl_PerVertex
            {
                vec4 gl_Position;
            };
            
            void main()
            {
                fsPosition = camera.projection * camera.view * object.world * vec4(vsPosition, 1);
                gl_Position = fsPosition;
                fsTexcoord = vsTexcoord;
                fsColor = vsColor;
            }
        )";
        fragmentShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.lineOffset = __LINE__;
        fragmentShaderInfo.source = R"(
            #version 450

            layout(set = 1, binding = 1) uniform sampler2D reflectionImage;
            layout(location = 0) in vec4 fsPosition;
            layout(location = 1) in vec2 fsTexcoord;
            layout(location = 2) in vec4 fsColor;
            layout(location = 0) out vec4 fragColor;

            void main()
            {
                // Calculate surface color
                vec4 surfaceColor;
                surfaceColor.rb = fsTexcoord;
                surfaceColor.g = dot(surfaceColor.rb, vec2(0.5));

                // Calculate reflection texcoord
                vec2 reflectionTexcoord = fsPosition.xy * (1.0 / fsPosition.w);
                reflectionTexcoord += vec2(1, 1);
                reflectionTexcoord *= 0.5;

                // Calculate reflection color
                vec4 reflectionColor = texture(reflectionImage, reflectionTexcoord);
                reflectionColor.a *= 0.34;
                reflectionColor.rgb *= reflectionColor.a;
                fragColor.rgb = surfaceColor.rgb + reflectionColor.rgb;
                fragColor.a = 1;
            }
        )";
        gvk::Pipeline floorPipeline;
        gvk_result(gvk_sample_create_pipeline<VertexPositionTexcoordColor>(
            renderTarget.get_render_pass(),
            VK_CULL_MODE_BACK_BIT,
            vertexShaderInfo,
            fragmentShaderInfo,
            &floorPipeline
        ));

        // Create camera uniform gvk::Buffer objects...
        gvk::Buffer cameraUniformBuffer;
        gvk::Buffer reflectionCameraUniformBuffer;
        gvk_result(gvk_sample_create_uniform_buffer<CameraUniforms>(context, &cameraUniformBuffer));
        gvk_result(gvk_sample_create_uniform_buffer<CameraUniforms>(context, &reflectionCameraUniformBuffer));

        // Allocate gvk::DescriptorSet objects...because both gvk::Pipeline objects
        //  have individual camera and object uniforms we expect 2 gvk::DescriptorSet
        //  objects for each, with the camera gvk::DescriptorSet at index 0 and the
        //  object gvk::DescriptorSet at index 1 as defined in the vertex shader GLSL.
        //  The gvk::Pipeline that will be used to draw the floor object uses the
        //  gvk::RenderTarget as a texture to create the reflection of the cube object.
        //  The gvk::ImageView for the gvk::RenderTarget color attachment will be at
        //  gvk::DescriptorSet 1 and binding index 1...
        std::vector<gvk::DescriptorSet> descriptorSets;
        gvk_result(gvk_sample_allocate_descriptor_sets(cubePipeline, &descriptorSets));
        assert(descriptorSets.size() == 2);
        auto reflectionCameraDescriptorSet = descriptorSets[0];
        auto cubeDescriptorSet = descriptorSets[1];
        gvk_result(gvk_sample_allocate_descriptor_sets(floorPipeline, &descriptorSets));
        assert(descriptorSets.size() == 2);
        auto cameraDescriptorSet = descriptorSets[0];
        auto floorDescriptorSet = descriptorSets[1];

        // Prepare descriptors...
        auto cubeUniformBufferDescriptorInfo = gvk::get_default<VkDescriptorBufferInfo>();
        cubeUniformBufferDescriptorInfo.buffer = cubeUniformBuffer;
        auto floorUniformBufferDescriptorInfo = gvk::get_default<VkDescriptorBufferInfo>();
        floorUniformBufferDescriptorInfo.buffer = floorUniformBuffer;
        auto cameraUniformBufferDescriptorInfo = gvk::get_default<VkDescriptorBufferInfo>();
        cameraUniformBufferDescriptorInfo.buffer = cameraUniformBuffer;
        auto reflectionCameraUniformBufferDescriptorInfo = gvk::get_default<VkDescriptorBufferInfo>();
        reflectionCameraUniformBufferDescriptorInfo.buffer = reflectionCameraUniformBuffer;

        // For the VkDescriptorImageInfo we'll use the gvk::RenderTarget object's
        //  color attachment.  If the gvk::RenderTarget has multisample anti aliasing
        //  enabled, the MSAA attachment will be at index 0 and the resolve attachment
        //  will be the gvk::ImageView at index 1...
        auto colorAttachmentIndex = VK_SAMPLE_COUNT_1_BIT < renderTargetCreateInfo.sampleCount ? 1 : 0;
        assert(!renderTarget.get_framebuffer().get<gvk::ImageViews>().empty());
        auto renderTargetColorAttachmentDescriptorInfo = gvk::get_default<VkDescriptorImageInfo>();
        renderTargetColorAttachmentDescriptorInfo.sampler = sampler;
        renderTargetColorAttachmentDescriptorInfo.imageView = renderTarget.get_framebuffer().get<gvk::ImageViews>()[colorAttachmentIndex];
        renderTargetColorAttachmentDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Write the descriptors...
        std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{
            VkWriteDescriptorSet {
                /* .sType            = */ gvk::get_stype<VkWriteDescriptorSet>(),
                /* .pNext            = */ nullptr,
                /* .dstSet           = */ cubeDescriptorSet,
                /* .dstBinding       = */ 0,
                /* .dstArrayElement  = */ 0,
                /* .descriptorCount  = */ 1,
                /* .descriptorType   = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                /* .pImageInfo       = */ nullptr,
                /* .pBufferInfo      = */ &cubeUniformBufferDescriptorInfo,
                /* .pTexelBufferView = */ nullptr,
            },
            VkWriteDescriptorSet {
                /* .sType            = */ gvk::get_stype<VkWriteDescriptorSet>(),
                /* .pNext            = */ nullptr,
                /* .dstSet           = */ floorDescriptorSet,
                /* .dstBinding       = */ 0,
                /* .dstArrayElement  = */ 0,
                /* .descriptorCount  = */ 1,
                /* .descriptorType   = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                /* .pImageInfo       = */ nullptr,
                /* .pBufferInfo      = */ &floorUniformBufferDescriptorInfo,
                /* .pTexelBufferView = */ nullptr,
            },
            VkWriteDescriptorSet {
                /* .sType            = */ gvk::get_stype<VkWriteDescriptorSet>(),
                /* .pNext            = */ nullptr,
                /* .dstSet           = */ floorDescriptorSet,
                /* .dstBinding       = */ 1,
                /* .dstArrayElement  = */ 0,
                /* .descriptorCount  = */ 1,
                /* .descriptorType   = */ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                /* .pImageInfo       = */ &renderTargetColorAttachmentDescriptorInfo,
                /* .pBufferInfo      = */ nullptr,
                /* .pTexelBufferView = */ nullptr,
            },
            VkWriteDescriptorSet {
                /* .sType            = */ gvk::get_stype<VkWriteDescriptorSet>(),
                /* .pNext            = */ nullptr,
                /* .dstSet           = */ cameraDescriptorSet,
                /* .dstBinding       = */ 0,
                /* .dstArrayElement  = */ 0,
                /* .descriptorCount  = */ 1,
                /* .descriptorType   = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                /* .pImageInfo       = */ nullptr,
                /* .pBufferInfo      = */ &cameraUniformBufferDescriptorInfo,
                /* .pTexelBufferView = */ nullptr,
            },
            VkWriteDescriptorSet {
                /* .sType            = */ gvk::get_stype<VkWriteDescriptorSet>(),
                /* .pNext            = */ nullptr,
                /* .dstSet           = */ reflectionCameraDescriptorSet,
                /* .dstBinding       = */ 0,
                /* .dstArrayElement  = */ 0,
                /* .descriptorCount  = */ 1,
                /* .descriptorType   = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                /* .pImageInfo       = */ nullptr,
                /* .pBufferInfo      = */ &reflectionCameraUniformBufferDescriptorInfo,
                /* .pTexelBufferView = */ nullptr,
            },
        };
        vkUpdateDescriptorSets(context.get_devices()[0], (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

        gvk::math::Camera camera;
        camera.transform.translation = { 0, 2, -7 };
        gvk::math::FreeCameraController cameraController;
        cameraController.set_camera(&camera);

        gvk::sys::Clock clock;
        while (
            !(context.get_sys_surface().get_input().keyboard.down(gvk::sys::Key::Escape)) &&
            !(context.get_sys_surface().get_status() & gvk::sys::Surface::CloseRequested)) {
            gvk::sys::Surface::update();
            clock.update();

            // Update the gvk::math::FreeCameraController...
            auto deltaTime = clock.elapsed<gvk::sys::Seconds<float>>();
            const auto& input = context.get_sys_surface().get_input();
            gvk::math::FreeCameraController::UpdateInfo cameraControllerUpdateInfo {
                /* .deltaTime           = */ deltaTime,
                /* .moveUp              = */ input.keyboard.down(gvk::sys::Key::Q),
                /* .moveDown            = */ input.keyboard.down(gvk::sys::Key::E),
                /* .moveLeft            = */ input.keyboard.down(gvk::sys::Key::A),
                /* .moveRight           = */ input.keyboard.down(gvk::sys::Key::D),
                /* .moveForward         = */ input.keyboard.down(gvk::sys::Key::W),
                /* .moveBackward        = */ input.keyboard.down(gvk::sys::Key::S),
                /* .moveSpeedMultiplier = */ input.keyboard.down(gvk::sys::Key::LeftShift) ? 2.0f : 1.0f,
                /* .lookDelta           = */ { input.mouse.position.delta()[0], input.mouse.position.delta()[1] },
                /* .fieldOfViewDelta    = */ input.mouse.scroll.delta()[1],
            };
            cameraController.lookEnabled = input.mouse.buttons.down(gvk::sys::Mouse::Button::Left);
            if (input.mouse.buttons.pressed(gvk::sys::Mouse::Button::Right)) {
                camera.fieldOfView = 60.0f;
            }
            cameraController.update(cameraControllerUpdateInfo);

            // Update the floating cube object's gvk::math::Transform...
            float anchor = 1.5f;
            float amplitude = 0.5f;
            float frequency = 3;
            cubeTransform.translation.y = anchor + amplitude * glm::sin(frequency * clock.total<gvk::sys::Seconds<float>>());
            auto cubeRotationY = glm::angleAxis(glm::radians(90.0f * deltaTime), glm::vec3{ 0, 1, 0 });
            auto cubeRotationZ = glm::angleAxis(glm::radians(45.0f * deltaTime), glm::vec3{ 0, 0, 1 });
            cubeTransform.rotation = glm::normalize(cubeRotationY * cubeTransform.rotation * cubeRotationZ);

            // Uddate the gvk::math::Camera uniform data...
            CameraUniforms cameraUbo{ };
            cameraUbo.view = camera.view();
            cameraUbo.projection = camera.projection();
            VmaAllocationInfo allocationInfo{ };
            vmaGetAllocationInfo(context.get_devices()[0].get<VmaAllocator>(), cameraUniformBuffer.get<VmaAllocation>(), &allocationInfo);
            assert(allocationInfo.pMappedData);
            memcpy(allocationInfo.pMappedData, &cameraUbo, sizeof(CameraUniforms));

            // Setup the reflection vk::math::Camera uniforms by scaling the view by -1 on
            //  the y axis then update the reflection gvk::math::Camera uniform data...
            cameraUbo.view = cameraUbo.view * glm::scale(glm::vec3{ 1, -1, 1 });
            vmaGetAllocationInfo(context.get_devices()[0].get<VmaAllocator>(), reflectionCameraUniformBuffer.get<VmaAllocation>(), &allocationInfo);
            assert(allocationInfo.pMappedData);
            memcpy(allocationInfo.pMappedData, &cameraUbo, sizeof(CameraUniforms));

            // Update the cube uniform data...
            ObjectUniforms cubeUbo{ };
            cubeUbo.world = cubeTransform.world_from_local();
            vmaGetAllocationInfo(context.get_devices()[0].get<VmaAllocator>(), cubeUniformBuffer.get<VmaAllocation>(), &allocationInfo);
            assert(allocationInfo.pMappedData);
            memcpy(allocationInfo.pMappedData, &cubeUbo, sizeof(ObjectUniforms));

            // Update the floor uniform data...
            ObjectUniforms floorUbo{ };
            floorUbo.world = floorTransform.world_from_local();
            vmaGetAllocationInfo(context.get_devices()[0].get<VmaAllocator>(), floorUniformBuffer.get<VmaAllocation>(), &allocationInfo);
            assert(allocationInfo.pMappedData);
            memcpy(allocationInfo.pMappedData, &floorUbo, sizeof(ObjectUniforms));

            auto& wsiManager = context.get_wsi_manager();
            if (wsiManager.update()) {
                auto extent = wsiManager.get_swapchain().get<VkSwapchainCreateInfoKHR>().imageExtent;
                camera.set_aspect_ratio(extent.width, extent.height);
                for (size_t i = 0; i < wsiManager.get_command_buffers().size(); ++i) {
                    const auto& commandBuffer = wsiManager.get_command_buffers()[i];
                    gvk_result(vkBeginCommandBuffer(commandBuffer, &gvk::get_default<VkCommandBufferBeginInfo>()));

                    // Begin a gvk::RenderPass with our gvk::RenderTarget...
                    auto renderPassBeginInfo = renderTarget.get_render_pass_begin_info();
                    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                    {
                        VkRect2D scissor{ { }, renderPassBeginInfo.renderArea.extent };
                        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
                        VkViewport viewport{ 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                        // Bind cube gvk::Pipeline, reflection gvk::math::Camera uniform gvk::Buffer,
                        //  and the cube uniform gvk::Buffer, then render the cube gvk::Mesh.  This
                        //  will draw the cube into the gvk::RenderTarget with the reflection view
                        //  matrix...in the next gvk::RenderPass, we'll use this gvk::RenderTarget as
                        //  the texture for the floor to create the illusion of a reflection on the
                        //  floor...
                        auto pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        vkCmdBindPipeline(commandBuffer, pipelineBindPoint, cubePipeline);
                        vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, cubePipeline.get<gvk::PipelineLayout>(), 0, 1, &(const VkDescriptorSet&)reflectionCameraDescriptorSet, 0, nullptr);
                        vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, cubePipeline.get<gvk::PipelineLayout>(), 1, 1, &(const VkDescriptorSet&)cubeDescriptorSet, 0, nullptr);
                        cubeMesh.record_cmds(commandBuffer);
                    }
                    vkCmdEndRenderPass(commandBuffer);

                    // Begin the gvk::RenderPass that renders into the gvk::WsiManager...
                    renderPassBeginInfo = wsiManager.get_render_targets()[i].get_render_pass_begin_info();
                    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                    {
                        VkRect2D scissor{ { }, renderPassBeginInfo.renderArea.extent };
                        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
                        VkViewport viewport{ 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                        // Bind the gvk::math::Camera uniform gvk::Buffer and the floor resources then
                        //  issue a draw call for the floor.  Then bind the floating cube resources...
                        //  we can leave the gvk::math::Camera uniform gvk::Buffer bound and update the
                        //  gvk::Pipeline and gvk::DescriptorSet at index 1 without distrubing the
                        //  gvk::DescriptorSet at index 0...then issue a draw call for the floating
                        //  cube...
                        auto pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        vkCmdBindPipeline(commandBuffer, pipelineBindPoint, floorPipeline);
                        vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, floorPipeline.get<gvk::PipelineLayout>(), 0, 1, &(const VkDescriptorSet&)cameraDescriptorSet, 0, nullptr);
                        vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, floorPipeline.get<gvk::PipelineLayout>(), 1, 1, &(const VkDescriptorSet&)floorDescriptorSet, 0, nullptr);
                        floorMesh.record_cmds(commandBuffer);
                        vkCmdBindPipeline(commandBuffer, pipelineBindPoint, cubePipeline);
                        vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, cubePipeline.get<gvk::PipelineLayout>(), 1, 1, &(const VkDescriptorSet&)cubeDescriptorSet, 0, nullptr);
                        cubeMesh.record_cmds(commandBuffer);
                    }
                    vkCmdEndRenderPass(commandBuffer);

                    // Ensure the gvk::RenderTarget attachments are transitioned back to the
                    //  VkImageLayout expected when the gvk::RenderPass is next executed...the
                    //  VkImageMemoryBarrier objects provided by gvk::RenderTarget do not
                    //  account for layout transitions that occur outside of the associated
                    //  gvk::RenderPass, those must be handled by your application...
                    auto attachmentCount = renderTarget.get_render_pass().get<VkRenderPassCreateInfo2>().attachmentCount;
                    for (size_t attachment_i = 0; attachment_i < attachmentCount; ++attachment_i) {
                        auto imageMemoryBarrier = renderTarget.get_image_memory_barrier((uint32_t)attachment_i);
                        if (imageMemoryBarrier.oldLayout != imageMemoryBarrier.newLayout) {
                            vkCmdPipelineBarrier(
                                commandBuffer,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                0,
                                0, nullptr,
                                0, nullptr,
                                1, &imageMemoryBarrier
                            );
                        }
                    }

                    gvk_result(vkEndCommandBuffer(commandBuffer));
                }
            }
            gvk_result(gvk_sample_acquire_submit_present(context));
        }
        gvk_result(vkDeviceWaitIdle(context.get_devices()[0]));
    } gvk_result_scope_end;
    if (gvkResult) {
        std::cerr << gvk::to_string(gvkResult) << std::endl;
    }
    return (int)gvkResult;
}
