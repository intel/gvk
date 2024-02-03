
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

TEST(ImageLayout, SingleMipSingleArray)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    gvk::DeviceMemory deviceMemory;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    VkImageLayout imageLayout { };
    gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), &imageLayout);
    EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_UNDEFINED);

    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    stateTrackedImage = gvk::get_state_tracked_object(image);
    gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), &imageLayout);
    EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_PREINITIALIZED);
}

TEST(ImageLayout, MultiMipMultiArray)
{
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent = Extent;
    imageCreateInfo.mipLevels = gvk::get_mip_level_count(imageCreateInfo.extent);
    imageCreateInfo.arrayLayers = ArrayLayers;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    gvk::DeviceMemory deviceMemory;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
    for (const auto& imageLayout : imageLayouts) {
        EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_UNDEFINED);
    }

    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    stateTrackedImage = gvk::get_state_tracked_object(image);
    gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
    for (const auto& imageLayout : imageLayouts) {
        EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_PREINITIALIZED);
    }
}

TEST(ImageLayout, PipelineBarrier)
{
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent = Extent;
    imageCreateInfo.mipLevels = gvk::get_mip_level_count(imageCreateInfo.extent);
    imageCreateInfo.arrayLayers = ArrayLayers;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    gvk::DeviceMemory deviceMemory;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers { };
    for (auto& imageMemoryBarrier : imageMemoryBarriers) {
        imageMemoryBarrier = gvk::get_default<VkImageMemoryBarrier>();
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.layerCount = imageCreateInfo.arrayLayers / 2;
        imageMemoryBarrier.subresourceRange.levelCount = imageCreateInfo.mipLevels / 2;
    }
    imageMemoryBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageMemoryBarriers[0].subresourceRange.baseArrayLayer = 0;
    imageMemoryBarriers[0].subresourceRange.baseMipLevel = 0;
    imageMemoryBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarriers[1].subresourceRange.baseArrayLayer = imageCreateInfo.arrayLayers / 2;
    imageMemoryBarriers[1].subresourceRange.baseMipLevel = imageCreateInfo.mipLevels / 2;

    gvk::execute_immediately(
        context.get_devices()[0],
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](auto)
        {
            const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
            assert(dispatchTable.gvkCmdPipelineBarrier);
            dispatchTable.gvkCmdPipelineBarrier(
                context.get_command_buffers()[0],
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                0,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );
        }
    );

    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
    for (uint32_t arrayLayer = 0; arrayLayer < imageCreateInfo.arrayLayers; ++arrayLayer) {
        for (uint32_t mipLevel = 0; mipLevel < imageCreateInfo.mipLevels; ++mipLevel) {
            auto imageLayout = imageLayouts[arrayLayer * imageCreateInfo.mipLevels + mipLevel];
            if (arrayLayer < imageCreateInfo.arrayLayers / 2 && mipLevel < imageCreateInfo.mipLevels / 2) {
                EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            } else if (imageCreateInfo.arrayLayers / 2 <= arrayLayer && imageCreateInfo.mipLevels / 2 <= mipLevel) {
                EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            }
        }
    }
}

TEST(ImageLayout, PipelineBarrier2)
{
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent = Extent;
    imageCreateInfo.mipLevels = gvk::get_mip_level_count(imageCreateInfo.extent);
    imageCreateInfo.arrayLayers = ArrayLayers;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    gvk::DeviceMemory deviceMemory;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    std::array<VkImageMemoryBarrier2, 2> imageMemoryBarriers { };
    for (auto& imageMemoryBarrier : imageMemoryBarriers) {
        imageMemoryBarrier = gvk::get_default<VkImageMemoryBarrier2>();
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.layerCount = imageCreateInfo.arrayLayers / 2;
        imageMemoryBarrier.subresourceRange.levelCount = imageCreateInfo.mipLevels / 2;
    }
    imageMemoryBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageMemoryBarriers[0].subresourceRange.baseArrayLayer = 0;
    imageMemoryBarriers[0].subresourceRange.baseMipLevel = 0;
    imageMemoryBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarriers[1].subresourceRange.baseArrayLayer = imageCreateInfo.arrayLayers / 2;
    imageMemoryBarriers[1].subresourceRange.baseMipLevel = imageCreateInfo.mipLevels / 2;

    auto dependencyInfo = gvk::get_default<VkDependencyInfo>();
    dependencyInfo.imageMemoryBarrierCount = (uint32_t)imageMemoryBarriers.size();
    dependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();

    gvk::execute_immediately(
        context.get_devices()[0],
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](auto)
        {
            const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
            assert(dispatchTable.gvkCmdPipelineBarrier2);
            dispatchTable.gvkCmdPipelineBarrier2(context.get_command_buffers()[0], &dependencyInfo);
        }
    );

    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
    for (uint32_t arrayLayer = 0; arrayLayer < imageCreateInfo.arrayLayers; ++arrayLayer) {
        for (uint32_t mipLevel = 0; mipLevel < imageCreateInfo.mipLevels; ++mipLevel) {
            auto imageLayout = imageLayouts[arrayLayer * imageCreateInfo.mipLevels + mipLevel];
            if (arrayLayer < imageCreateInfo.arrayLayers / 2 && mipLevel < imageCreateInfo.mipLevels / 2) {
                EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            } else if (imageCreateInfo.arrayLayers / 2 <= arrayLayer && imageCreateInfo.mipLevels / 2 <= mipLevel) {
                EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            }
        }
    }
}

TEST(ImageLayout, WaitEvents)
{
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent = Extent;
    imageCreateInfo.mipLevels = gvk::get_mip_level_count(imageCreateInfo.extent);
    imageCreateInfo.arrayLayers = ArrayLayers;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    gvk::DeviceMemory deviceMemory;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers { };
    for (auto& imageMemoryBarrier : imageMemoryBarriers) {
        imageMemoryBarrier = gvk::get_default<VkImageMemoryBarrier>();
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.layerCount = imageCreateInfo.arrayLayers / 2;
        imageMemoryBarrier.subresourceRange.levelCount = imageCreateInfo.mipLevels / 2;
    }
    imageMemoryBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageMemoryBarriers[0].subresourceRange.baseArrayLayer = 0;
    imageMemoryBarriers[0].subresourceRange.baseMipLevel = 0;
    imageMemoryBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarriers[1].subresourceRange.baseArrayLayer = imageCreateInfo.arrayLayers / 2;
    imageMemoryBarriers[1].subresourceRange.baseMipLevel = imageCreateInfo.mipLevels / 2;

    auto eventCreateInfo = gvk::get_default<VkEventCreateInfo>();
    gvk::Event event;
    ASSERT_EQ(gvk::Event::create(context.get_devices()[0], &eventCreateInfo, nullptr, &event), VK_SUCCESS);
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkSetEvent);
    ASSERT_EQ(dispatchTable.gvkSetEvent(context.get_devices()[0], event), VK_SUCCESS);

    gvk::execute_immediately(
        context.get_devices()[0],
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](auto)
        {
            assert(dispatchTable.gvkCmdWaitEvents);
            dispatchTable.gvkCmdWaitEvents(
                context.get_command_buffers()[0],
                1,
                &event.get<VkEvent>(),
                VK_PIPELINE_STAGE_HOST_BIT,
                VK_PIPELINE_STAGE_HOST_BIT,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );
        }
    );

    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
    for (uint32_t arrayLayer = 0; arrayLayer < imageCreateInfo.arrayLayers; ++arrayLayer) {
        for (uint32_t mipLevel = 0; mipLevel < imageCreateInfo.mipLevels; ++mipLevel) {
            auto imageLayout = imageLayouts[arrayLayer * imageCreateInfo.mipLevels + mipLevel];
            if (arrayLayer < imageCreateInfo.arrayLayers / 2 && mipLevel < imageCreateInfo.mipLevels / 2) {
                EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            } else if (imageCreateInfo.arrayLayers / 2 <= arrayLayer && imageCreateInfo.mipLevels / 2 <= mipLevel) {
                EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            }
        }
    }
}

TEST(ImageLayout, WaitEvents2)
{
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);

    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent = Extent;
    imageCreateInfo.mipLevels = gvk::get_mip_level_count(imageCreateInfo.extent);
    imageCreateInfo.arrayLayers = ArrayLayers;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    gvk::DeviceMemory deviceMemory;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    std::array<VkImageMemoryBarrier2, 2> imageMemoryBarriers { };
    for (auto& imageMemoryBarrier : imageMemoryBarriers) {
        imageMemoryBarrier = gvk::get_default<VkImageMemoryBarrier2>();
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.layerCount = imageCreateInfo.arrayLayers / 2;
        imageMemoryBarrier.subresourceRange.levelCount = imageCreateInfo.mipLevels / 2;
    }
    imageMemoryBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageMemoryBarriers[0].subresourceRange.baseArrayLayer = 0;
    imageMemoryBarriers[0].subresourceRange.baseMipLevel = 0;
    imageMemoryBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarriers[1].subresourceRange.baseArrayLayer = imageCreateInfo.arrayLayers / 2;
    imageMemoryBarriers[1].subresourceRange.baseMipLevel = imageCreateInfo.mipLevels / 2;

    auto dependencyInfo = gvk::get_default<VkDependencyInfo>();
    dependencyInfo.imageMemoryBarrierCount = (uint32_t)imageMemoryBarriers.size();
    dependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();

    auto eventCreateInfo = gvk::get_default<VkEventCreateInfo>();
    gvk::Event event;
    ASSERT_EQ(gvk::Event::create(context.get_devices()[0], &eventCreateInfo, nullptr, &event), VK_SUCCESS);
    const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkSetEvent);
    ASSERT_EQ(dispatchTable.gvkSetEvent(context.get_devices()[0], event), VK_SUCCESS);

    gvk::execute_immediately(
        context.get_devices()[0],
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](auto)
        {
            assert(dispatchTable.gvkCmdWaitEvents2);
            dispatchTable.gvkCmdWaitEvents2(context.get_command_buffers()[0], 1, &event.get<VkEvent>(), &dependencyInfo);
        }
    );

    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
    for (uint32_t arrayLayer = 0; arrayLayer < imageCreateInfo.arrayLayers; ++arrayLayer) {
        for (uint32_t mipLevel = 0; mipLevel < imageCreateInfo.mipLevels; ++mipLevel) {
            auto imageLayout = imageLayouts[arrayLayer * imageCreateInfo.mipLevels + mipLevel];
            if (arrayLayer < imageCreateInfo.arrayLayers / 2 && mipLevel < imageCreateInfo.mipLevels / 2) {
                EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            } else if (imageCreateInfo.arrayLayers / 2 <= arrayLayer && imageCreateInfo.mipLevels / 2 <= mipLevel) {
                EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            }
        }
    }
}

TEST(ImageLayout, RenderPass)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);

    auto colorFormat = VK_FORMAT_UNDEFINED;
    const auto& physicalDevice = context.get_devices()[0].get<gvk::PhysicalDevice>();
    gvk::enumerate_formats(
        physicalDevice.get<gvk::DispatchTable>().gvkGetPhysicalDeviceFormatProperties2,
        physicalDevice,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT,
        [&](VkFormat format)
        {
            GvkFormatInfo formatInfo { };
            gvk::get_format_info(format, &formatInfo);
            if (gvk::get_bits_per_texel(format) == 32 &&
                formatInfo.componentCount == 4 &&
                formatInfo.compressionType == GVK_FORMAT_COMPRESSION_TYPE_NONE &&
                formatInfo.numericFormat == GVK_NUMERIC_FORMAT_UNORM &&
                !formatInfo.packed &&
                !formatInfo.chroma
            ) {
                colorFormat = format;
            }
            return colorFormat == VK_FORMAT_UNDEFINED;
        }
    );
    EXPECT_EQ(colorFormat, VK_FORMAT_R8G8B8A8_UNORM);

    // Get depth VkFormat
    auto depthFormat = VK_FORMAT_UNDEFINED;
    auto requestedDepthFormat = VK_FORMAT_D32_SFLOAT;
    GvkFormatInfo depthFormatInfo { };
    gvk::get_format_info(requestedDepthFormat, &depthFormatInfo);
    assert(depthFormatInfo.componentCount);
    assert(depthFormatInfo.pComponents);
    auto requestedDepthBits = depthFormatInfo.pComponents[0].bits;
    gvk::enumerate_formats(
        physicalDevice.get<gvk::DispatchTable>().gvkGetPhysicalDeviceFormatProperties2,
        physicalDevice,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT,
        [&](VkFormat format)
        {
            // TODO : Abstract this logic into a utility function
            if (format == requestedDepthFormat) {
                depthFormat = requestedDepthFormat;
            } else {
                uint32_t actualDepthBits = 0;
                if (depthFormat) {
                    gvk::get_format_info(depthFormat, &depthFormatInfo);
                    assert(depthFormatInfo.componentCount);
                    assert(depthFormatInfo.pComponents);
                    actualDepthBits = depthFormatInfo.pComponents[0].bits;
                }
                GvkFormatInfo formatInfo { };
                gvk::get_format_info(format, &formatInfo);
                assert(formatInfo.componentCount);
                assert(formatInfo.pComponents);
                auto formatDepthBits = formatInfo.pComponents[0].bits;
                if (actualDepthBits < formatDepthBits && formatDepthBits <= requestedDepthBits) {
                    depthFormat = format;
                }
            }
            return depthFormat != requestedDepthFormat;
        }
    );
    EXPECT_NE(depthFormat, VK_FORMAT_UNDEFINED);

    // Get VkSampleCountFlagBits
    auto sampleCount = gvk::get_max_framebuffer_sample_count(physicalDevice, VK_TRUE, VK_TRUE, VK_FALSE);

    // Create gvk::RenderTarget
    auto renderTargetValidationCreateInfo = gvk::get_default<RenderTargetValidationCreateInfo>();
    renderTargetValidationCreateInfo.extent = { 1024, 1024 };
    renderTargetValidationCreateInfo.sampleCount = sampleCount;
    renderTargetValidationCreateInfo.colorFormat = colorFormat;
    renderTargetValidationCreateInfo.depthFormat = depthFormat;
    gvk::RenderTarget renderTarget;
    create_render_target(context, &renderTargetValidationCreateInfo, &renderTarget);

    gvk::execute_immediately(
        context.get_devices()[0],
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](auto)
        {
            auto renderPassBeginInfo = renderTarget.get_render_pass_begin_info();
            const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
            assert(dispatchTable.gvkCmdBeginRenderPass);
            dispatchTable.gvkCmdBeginRenderPass(context.get_command_buffers()[0], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            assert(dispatchTable.gvkCmdEndRenderPass);
            dispatchTable.gvkCmdEndRenderPass(context.get_command_buffers()[0]);
        }
    );

    auto renderPass = renderTarget.get_render_pass();
    ASSERT_TRUE(renderPass);
    const auto& renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo2>();
    for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
        auto image = renderTarget.get_image(i);
        ASSERT_TRUE(image);
        auto stateTrackedImage = gvk::get_state_tracked_object(image);
        VkImageLayout imageLayout { };
        gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), &imageLayout);
        EXPECT_EQ(imageLayout, renderPassCreateInfo.pAttachments[i].finalLayout);
    }
}

TEST(ImageLayout, RenderPass2)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);

    // Get color VkFormat
    auto colorFormat = VK_FORMAT_UNDEFINED;
    const auto& physicalDevice = context.get_devices()[0].get<gvk::PhysicalDevice>();
    gvk::enumerate_formats(
        physicalDevice.get<gvk::DispatchTable>().gvkGetPhysicalDeviceFormatProperties2,
        physicalDevice,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT,
        [&](VkFormat format)
        {
            GvkFormatInfo formatInfo { };
            gvk::get_format_info(format, &formatInfo);
            if (gvk::get_bits_per_texel(format) == 32 &&
                formatInfo.componentCount == 4 &&
                formatInfo.compressionType == GVK_FORMAT_COMPRESSION_TYPE_NONE &&
                formatInfo.numericFormat == GVK_NUMERIC_FORMAT_UNORM &&
                !formatInfo.packed &&
                !formatInfo.chroma
            ) {
                colorFormat = format;
            }
            return colorFormat == VK_FORMAT_UNDEFINED;
        }
    );
    EXPECT_EQ(colorFormat, VK_FORMAT_R8G8B8A8_UNORM);

    // Get depth VkFormat
    auto depthFormat = VK_FORMAT_UNDEFINED;
    auto requestedDepthFormat = VK_FORMAT_D32_SFLOAT;
    GvkFormatInfo depthFormatInfo { };
    gvk::get_format_info(requestedDepthFormat, &depthFormatInfo);
    assert(depthFormatInfo.componentCount);
    assert(depthFormatInfo.pComponents);
    auto requestedDepthBits = depthFormatInfo.pComponents[0].bits;
    gvk::enumerate_formats(
        physicalDevice.get<gvk::DispatchTable>().gvkGetPhysicalDeviceFormatProperties2,
        physicalDevice,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT,
        [&](VkFormat format)
        {
            // TODO : Abstract this logic into a utility function
            if (format == requestedDepthFormat) {
                depthFormat = requestedDepthFormat;
            } else {
                uint32_t actualDepthBits = 0;
                if (depthFormat) {
                    gvk::get_format_info(depthFormat, &depthFormatInfo);
                    assert(depthFormatInfo.componentCount);
                    assert(depthFormatInfo.pComponents);
                    actualDepthBits = depthFormatInfo.pComponents[0].bits;
                }
                GvkFormatInfo formatInfo { };
                gvk::get_format_info(format, &formatInfo);
                assert(formatInfo.componentCount);
                assert(formatInfo.pComponents);
                auto formatDepthBits = formatInfo.pComponents[0].bits;
                if (actualDepthBits < formatDepthBits && formatDepthBits <= requestedDepthBits) {
                    depthFormat = format;
                }
            }
            return depthFormat != requestedDepthFormat;
        }
    );
    EXPECT_NE(depthFormat, VK_FORMAT_UNDEFINED);

    // Get VkSampleCountFlagBits
    auto sampleCount = gvk::get_max_framebuffer_sample_count(physicalDevice, VK_TRUE, VK_TRUE, VK_FALSE);

    // Create gvk::RenderTarget
    auto renderTargetValidationCreateInfo = gvk::get_default<RenderTargetValidationCreateInfo>();
    renderTargetValidationCreateInfo.extent = { 1024, 1024 };
    renderTargetValidationCreateInfo.sampleCount = sampleCount;
    renderTargetValidationCreateInfo.colorFormat = colorFormat;
    renderTargetValidationCreateInfo.depthFormat = depthFormat;
    gvk::RenderTarget renderTarget;
    create_render_target(context, &renderTargetValidationCreateInfo, &renderTarget);

    gvk::execute_immediately(
        context.get_devices()[0],
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](auto)
        {
            auto renderPassBeginInfo = renderTarget.get_render_pass_begin_info();
            auto subpassBeginInfo = gvk::get_default<VkSubpassBeginInfo>();
            subpassBeginInfo.contents = VK_SUBPASS_CONTENTS_INLINE;
            const auto& dispatchTable = context.get_devices()[0].get<gvk::DispatchTable>();
            assert(dispatchTable.gvkCmdBeginRenderPass2);
            dispatchTable.gvkCmdBeginRenderPass2(context.get_command_buffers()[0], &renderPassBeginInfo, &subpassBeginInfo);
            assert(dispatchTable.gvkCmdEndRenderPass);
            dispatchTable.gvkCmdEndRenderPass(context.get_command_buffers()[0]);
        }
    );

    auto renderPass = renderTarget.get_render_pass();
    ASSERT_TRUE(renderPass);
    const auto& renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo2>();
    for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
        auto image = renderTarget.get_image(i);
        ASSERT_TRUE(image);
        auto stateTrackedImage = gvk::get_state_tracked_object(image);
        VkImageLayout imageLayout { };
        gvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), &imageLayout);
        EXPECT_EQ(imageLayout, renderPassCreateInfo.pAttachments[i].finalLayout);
    }
}
