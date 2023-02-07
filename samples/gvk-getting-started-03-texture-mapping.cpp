
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

#include "stb/stb_image.h"

#include <array>
#include <cassert>
#include <iostream>
#include <vector>

VkResult create_image_and_view(const gvk::Context& context, gvk::ImageView* pImageView)
{
    assert(pImageView);
    // We're declaring pImageData outside of our gvk_result_scope because we're
    //  responsible for cleaning it up and we need to make sure that we handle
    //  cases where an error has caused us to break out of the gvk_result_scope...
    stbi_uc* pImageData = nullptr;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // We're using stb (https://github.com/nothings/stb) to load image data.  Stb
        //  will give us the pixel data, width, height, and channel count of the loaded
        //  image.  We're passing 4 in for the number of channels we want stb to give
        //  us...if we passed 0, stb would provide the number of channels provided by
        //  the loaded image.
        int width = 0;
        int height = 0;
        int channels = 0;
#if 1
        pImageData = stbi_load_from_memory(gvk_sample_get_png().data(), (int)gvk_sample_get_png().size(), &width, &height, &channels, 4);
#else
        pImageData = stbi_load("path/to/image.png", &width, &height, &channels, 4);
#endif
        gvk_result(pImageData ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Create a gvk::Image with the width and height returned from stb.  We're
        //  setting VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        //  because we're going to use the gvk::Image as a trasfer destination when we
        //  copy data to it and we're going to sample from it during rendering...
        auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VmaAllocationCreateInfo allocationCreateInfo{ };
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        gvk::Image image;
        gvk_result(gvk::Image::create(context.get_devices()[0], &imageCreateInfo, &allocationCreateInfo, &image));

        // Get the gvk::Device object's VmaAllocator, then get the gvk::Image
        //  object's VmaAllocationInfo...we'll use this to get the required
        //  size to create a gvk::Buffer to use as a staging resource for
        //  uploading the image data...
        VmaAllocationInfo allocationInfo{ };
        auto vmaAllocator = context.get_devices()[0].get<VmaAllocator>();
        vmaGetAllocationInfo(vmaAllocator, image.get<VmaAllocation>(), &allocationInfo);

        // Create the staging gvk::Buffer.
        //  Because we're using VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        //  we should only perform sequential writes like memcpy().  See VMA's
        //  documentation for more info...
        //  https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/choosing_memory_type.html
        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = allocationInfo.size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        gvk::Buffer stagingBuffer;
        gvk_result(gvk::Buffer::create(context.get_devices()[0], &bufferCreateInfo, &allocationCreateInfo, &stagingBuffer));

        // Copy the image data that we got from stb into the gvk::Buffer...
        uint8_t* pStagingData = nullptr;
        gvk_result(vmaMapMemory(vmaAllocator, stagingBuffer.get<VmaAllocation>(), (void**)&pStagingData));
        memcpy(pStagingData, pImageData, width * height * 4 * sizeof(uint8_t));
        vmaUnmapMemory(vmaAllocator, stagingBuffer.get<VmaAllocation>());

        // We need to record cmds to execute the transfer.  gvk::execute_immediately()
        //  will prepare a given VkCommandBuffer (gvk handles implicitly convert to and
        //  from their underlying Vulkan type) for one time submission, execute the
        //  given callback, then submit the VkCommandBuffer on the given VkQueue.  If a
        //  VkFence isn't provided, gvk::execute_immediately() will block the calling
        //  thread until execution of the given VkCommandBuffer has completed...
        gvk_result(gvk::execute_immediately(
            context.get_devices()[0],
            gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
            context.get_command_buffers()[0],
            VK_NULL_HANDLE,
            [&](auto)
            {
                // We start by recording a VkImageMemoryBarrier that will transition the
                //  gvk::Image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and make it available at
                //  VK_PIPELINE_STAGE_TRANSFER_BIT...
                auto imageMemoryBarrier = gvk::get_default<VkImageMemoryBarrier>();
                imageMemoryBarrier.srcAccessMask = 0;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.image = image;
                vkCmdPipelineBarrier(
                    context.get_command_buffers()[0],
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier
                );

                // Copy from the staging resource to the gvk::Image...
                auto bufferImageCopy = gvk::get_default<VkBufferImageCopy>();
                bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                bufferImageCopy.imageSubresource.layerCount = 1;
                bufferImageCopy.imageExtent = imageCreateInfo.extent;
                vkCmdCopyBufferToImage(context.get_command_buffers()[0], stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

                // Then transition the gvk::Image to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                //  after VK_PIPELINE_STAGE_TRANSFER_BIT and make it available at
                //  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT since it will be sampled from during
                //  fragment shading...
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                vkCmdPipelineBarrier(
                    context.get_command_buffers()[0],
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier
                );
            }
        ));

        // Create gvk::ImageView...
        auto imageViewCreateInfo = gvk::get_default<VkImageViewCreateInfo>();
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = imageCreateInfo.format;
        gvk_result(gvk::ImageView::create(context.get_devices()[0], &imageViewCreateInfo, nullptr, pImageView));
    } gvk_result_scope_end;

    // If we got image data from stb, we need to free it...
    if (pImageData) {
        stbi_image_free(pImageData);
    }
    return gvkResult;
}

VkResult create_mesh(const gvk::Context& context, const glm::vec2& extent, gvk::Mesh* pMesh)
{
    assert(pMesh);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        float a = 1.0f / std::max(extent[0], extent[1]);
        auto w = extent[0] * a * 0.5f;
        auto h = extent[1] * a * 0.5f;
        std::array<VertexPositionTexcoord, 4> vertices {
            VertexPositionTexcoord {{ -w, 0, -h }, { 0, 0 }},
            VertexPositionTexcoord {{  w, 0, -h }, { 1, 0 }},
            VertexPositionTexcoord {{  w, 0,  h }, { 1, 1 }},
            VertexPositionTexcoord {{ -w, 0,  h }, { 0, 1 }},
        };
        std::array<uint16_t, 6> indices {
            0, 1, 2,
            2, 3, 0,
        };
        gvk_result(pMesh->write(
            context.get_devices()[0],
            gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
            context.get_command_buffers()[0],
            VK_NULL_HANDLE,
            (uint32_t)vertices.size(),
            vertices.data(),
            (uint32_t)indices.size(),
            indices.data()
        ));
    } gvk_result_scope_end;
    return gvkResult;
}

int main(int, const char*[])
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        GvkSampleContext context;
        gvk_result(GvkSampleContext::create("Intel(R) GPA Utilities for Vulkan* - Getting Started - 03 - Texture Mapping", &context));

        gvk::system::Surface systemSurface;
        gvk_result(gvk_sample_create_sys_surface(context, &systemSurface));

        gvk::WsiManager wsiManager;
        gvk_result(gvk_sample_create_wsi_manager(context, systemSurface, &wsiManager));

        gvk::spirv::ShaderInfo vertexShaderInfo{ };
        vertexShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.lineOffset = __LINE__;
        vertexShaderInfo.source = R"(
            #version 450

            layout(set = 0, binding = 0)
            uniform UniformBuffer
            {
                mat4 world;
                mat4 view;
                mat4 projection;
            } ubo;

            layout(location = 0) in vec3 vsPosition;
            layout(location = 1) in vec2 vsTexcoord;
            layout(location = 0) out vec2 fsTexcoord;

            out gl_PerVertex
            {
                vec4 gl_Position;
            };

            void main()
            {
                gl_Position = ubo.projection * ubo.view * ubo.world * vec4(vsPosition, 1);
                fsTexcoord = vsTexcoord;
            }
        )";
        gvk::spirv::ShaderInfo fragmentShaderInfo{ };
        fragmentShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.lineOffset = __LINE__;
        fragmentShaderInfo.source = R"(
            #version 450

            layout(set = 0, binding = 1) uniform sampler2D image;
            layout(location = 0) in vec2 fsTexcoord;
            layout(location = 0) out vec4 fragColor;

            void main()
            {
                fragColor = texture(image, fsTexcoord);
            }
        )";
        gvk::Pipeline pipeline;
        gvk_result(gvk_sample_create_pipeline<VertexPositionTexcoord>(
            wsiManager.get_render_pass(),
            VK_CULL_MODE_BACK_BIT,
            vertexShaderInfo,
            fragmentShaderInfo,
            &pipeline
        ));

        // Create gvk::ImageView...
        gvk::ImageView imageView;
        gvk_result(create_image_and_view(context, &imageView));
        auto imageViewExtent = imageView.get<gvk::Image>().get<VkImageCreateInfo>().extent;

        // Create gvk::Sampler...
        gvk::Sampler sampler;
        gvk_result(gvk::Sampler::create(context.get_devices()[0], &gvk::get_default<VkSamplerCreateInfo>(), nullptr, &sampler));

        gvk::Mesh mesh;
        gvk_result(create_mesh(context, { (float)imageViewExtent.width, (float)imageViewExtent.height }, &mesh));

        gvk::Buffer uniformBuffer;
        gvk_result(gvk_sample_create_uniform_buffer<Uniforms>(context, &uniformBuffer));
        VmaAllocationInfo uniformBufferAllocationInfo{ };
        vmaGetAllocationInfo(context.get_devices()[0].get<VmaAllocator>(), uniformBuffer.get<VmaAllocation>(), &uniformBufferAllocationInfo);

        std::vector<gvk::DescriptorSet> descriptorSets;
        gvk_result(gvk_sample_allocate_descriptor_sets(pipeline, descriptorSets));
        assert(descriptorSets.size() == 1);
        auto descriptorSet = descriptorSets[0];

        auto descriptorBufferInfo = gvk::get_default<VkDescriptorBufferInfo>();
        descriptorBufferInfo.buffer = uniformBuffer;
        descriptorBufferInfo.range = VK_WHOLE_SIZE;

        // Prepare a VkDescriptorImageInfo with our gvk::Sampler and gvk::ImageView...
        auto descriptorImageInfo = gvk::get_default<VkDescriptorImageInfo>();
        descriptorImageInfo.sampler = sampler;
        descriptorImageInfo.imageView = imageView;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Update the gvk::DescriptorSet with gvk::Buffer and gvk::Image descriptors...
        std::array<VkWriteDescriptorSet, 2> writeDescriptorSets {
            VkWriteDescriptorSet {
                /* .sType            = */ gvk::get_stype<VkWriteDescriptorSet>(),
                /* .pNext            = */ nullptr,
                /* .dstSet           = */ descriptorSet,
                /* .dstBinding       = */ 0,
                /* .dstArrayElement  = */ 0,
                /* .descriptorCount  = */ 1,
                /* .descriptorType   = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                /* .pImageInfo       = */ nullptr,
                /* .pBufferInfo      = */ &descriptorBufferInfo,
                /* .pTexelBufferView = */ nullptr,
            },
            VkWriteDescriptorSet {
                /* .sType            = */ gvk::get_stype<VkWriteDescriptorSet>(),
                /* .pNext            = */ nullptr,
                /* .dstSet           = */ descriptorSet,
                /* .dstBinding       = */ 1,
                /* .dstArrayElement  = */ 0,
                /* .descriptorCount  = */ 1,
                /* .descriptorType   = */ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                /* .pImageInfo       = */ &descriptorImageInfo,
                /* .pBufferInfo      = */ nullptr,
                /* .pTexelBufferView = */ nullptr,
            }
        };
        vkUpdateDescriptorSets(context.get_devices()[0], (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

        gvk::system::Clock clock;
        gvk::math::Camera camera;
        camera.transform.translation = { 0, 1.25f, 1.25f };
        gvk::math::Transform quadTransform;

        while (
            !(systemSurface.get_input().keyboard.down(gvk::system::Key::Escape)) &&
            !(systemSurface.get_status() & gvk::system::Surface::CloseRequested)) {
            gvk::system::Surface::update();

            clock.update();
            auto rotation = 90.0f * clock.elapsed<gvk::system::Seconds<float>>();
            quadTransform.rotation *= glm::angleAxis(glm::radians(rotation), glm::vec3 { 0, 1, 0 });

            Uniforms uniforms{ };
            uniforms.object.world = quadTransform.world_from_local();
            uniforms.camera.view = camera.view(quadTransform.translation);
            uniforms.camera.projection = camera.projection();
            memcpy(uniformBufferAllocationInfo.pMappedData, &uniforms, sizeof(Uniforms));

            wsiManager.update();
            auto swapchain = wsiManager.get_swapchain();
            if (swapchain) {
                auto extent = wsiManager.get_swapchain().get<VkSwapchainCreateInfoKHR>().imageExtent;
                camera.set_aspect_ratio(extent.width, extent.height);

                uint32_t imageIndex = 0;
                auto vkResult = wsiManager.acquire_next_image(UINT64_MAX, VK_NULL_HANDLE, &imageIndex);
                gvk_result((vkResult == VK_SUCCESS || vkResult == VK_SUBOPTIMAL_KHR) ? VK_SUCCESS : vkResult);

                const auto& device = context.get_devices()[0];
                const auto& vkFences = wsiManager.get_vk_fences();
                gvk_result(vkWaitForFences(device, 1, &vkFences[imageIndex], VK_TRUE, UINT64_MAX));
                gvk_result(vkResetFences(device, 1, &vkFences[imageIndex]));

                const auto& commandBuffer = wsiManager.get_command_buffers()[imageIndex];
                gvk_result(vkBeginCommandBuffer(commandBuffer, &gvk::get_default<VkCommandBufferBeginInfo>()));
                auto renderPassBeginInfo = wsiManager.get_render_targets()[imageIndex].get_render_pass_begin_info();
                vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                VkRect2D scissor { { }, renderPassBeginInfo.renderArea.extent };
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
                VkViewport viewport { 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get<gvk::PipelineLayout>(), 0, 1, &(const VkDescriptorSet&)descriptorSet, 0, nullptr);
                mesh.record_cmds(commandBuffer);

                vkCmdEndRenderPass(commandBuffer);
                gvk_result(vkEndCommandBuffer(commandBuffer));

                const auto& queue = gvk::get_queue_family(device, 0).queues[0];
                auto submitInfo = wsiManager.get_submit_info(imageIndex);
                gvk_result(vkQueueSubmit(queue, 1, &submitInfo, vkFences[imageIndex]));

                auto presentInfo = wsiManager.get_present_info(&imageIndex);
                vkResult = vkQueuePresentKHR(gvk::get_queue_family(context.get_devices()[0], 0).queues[0], &presentInfo);
                gvk_result((vkResult == VK_SUCCESS || vkResult == VK_SUBOPTIMAL_KHR) ? VK_SUCCESS : vkResult);
            }
        }
        gvk_result(vkDeviceWaitIdle(context.get_devices()[0]));
    } gvk_result_scope_end;
    if (gvkResult) {
        std::cerr << gvk::to_string(gvkResult) << std::endl;
    }
    return (int)gvkResult;
}
