
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

/**
TODO : Documentation
*/
TEST(ImageLayout, SingleMipSingleArray)
{
    // TODO : Documentation
    gvk::Context context;
    create_state_tracker_validation_context(&context);
    load_gvk_state_tracker_entry_points();

    // TODO : Documentation
    auto imageCreateInfo = gvk::get_default<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gvk::Image image;
    gvk::DeviceMemory deviceMemory;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    // TODO : Documentation
    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    VkImageLayout imageLayout { };
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), &imageLayout);
    EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_UNDEFINED);

    // TODO : Documentation
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    // TODO : Documentation
    stateTrackedImage = gvk::get_state_tracked_object(image);
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), &imageLayout);
    EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_PREINITIALIZED);
}

/**
TODO : Documentation
*/
TEST(ImageLayout, MultiMipMultiArray)
{
    // TODO : Documentation
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    // TODO : Documentation
    gvk::Context context;
    create_state_tracker_validation_context(&context);
    load_gvk_state_tracker_entry_points();

    // TODO : Documentation
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

    // TODO : Documentation
    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
    for (const auto& imageLayout : imageLayouts) {
        EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_UNDEFINED);
    }

    // TODO : Documentation
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    create_memory_bound_image(context, imageCreateInfo, &image, &deviceMemory);

    // TODO : Documentation
    stateTrackedImage = gvk::get_state_tracked_object(image);
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
    for (const auto& imageLayout : imageLayouts) {
        EXPECT_EQ(imageLayout, VK_IMAGE_LAYOUT_PREINITIALIZED);
    }
}

/**
TODO : Documentation
*/
TEST(ImageLayout, PipelineBarrier)
{
    // TODO : Documentation
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    // TODO : Documentation
    gvk::Context context;
    create_state_tracker_validation_context(&context);
    load_gvk_state_tracker_entry_points();

    // TODO : Documentation
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

    // TODO : Documentation
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

    // TODO : Documentation
    gvk::execute_immediately(
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](const gvk::CommandBuffer& commandBuffer)
        {
            const auto& dispatchTable = gvk::DispatchTable::get_global_dispatch_table();
            assert(dispatchTable.gvkCmdPipelineBarrier);
            dispatchTable.gvkCmdPipelineBarrier(
                commandBuffer,
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

    // TODO : Documentation
    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
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

/**
TODO : Documentation
*/
TEST(ImageLayout, PipelineBarrier2)
{
    // TODO : Documentation
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    // TODO : Documentation
    StateTrackerValidationContext context;
    StateTrackerValidationContext::create(&context);
    load_gvk_state_tracker_entry_points();

    // TODO : Documentation
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

    // TODO : Documentation
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

    // TODO : Documentation
    auto dependencyInfo = gvk::get_default<VkDependencyInfo>();
    dependencyInfo.imageMemoryBarrierCount = (uint32_t)imageMemoryBarriers.size();
    dependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();

    // TODO : Documentation
    gvk::execute_immediately(
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](const gvk::CommandBuffer& commandBuffer)
        {
            const auto& dispatchTable = gvk::DispatchTable::get_global_dispatch_table();
            assert(dispatchTable.gvkCmdPipelineBarrier2);
            dispatchTable.gvkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
        }
    );

    // TODO : Documentation
    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
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

/**
TODO : Documentation
*/
TEST(ImageLayout, WaitEvents)
{
    // TODO : Documentation
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    // TODO : Documentation
    gvk::Context context;
    create_state_tracker_validation_context(&context);
    load_gvk_state_tracker_entry_points();

    // TODO : Documentation
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

    // TODO : Documentation
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

    // TODO : Documentation
    auto eventCreateInfo = gvk::get_default<VkEventCreateInfo>();
    gvk::Event event;
    ASSERT_EQ(gvk::Event::create(context.get_devices()[0], &eventCreateInfo, nullptr, &event), VK_SUCCESS);
    const auto& dispatchTable = gvk::DispatchTable::get_global_dispatch_table();
    assert(dispatchTable.gvkSetEvent);
    ASSERT_EQ(dispatchTable.gvkSetEvent(context.get_devices()[0], event), VK_SUCCESS);

    // TODO : Documentation
    gvk::execute_immediately(
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](const gvk::CommandBuffer& commandBuffer)
        {
            assert(dispatchTable.gvkCmdWaitEvents);
            dispatchTable.gvkCmdWaitEvents(
                commandBuffer,
                1,
                &event.get<const VkEvent&>(),
                VK_PIPELINE_STAGE_HOST_BIT,
                VK_PIPELINE_STAGE_HOST_BIT,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );
        }
    );

    // TODO : Documentation
    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
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

/**
TODO : Documentation
*/
TEST(ImageLayout, WaitEvents2)
{
    // TODO : Documentation
    const VkExtent3D Extent { 512, 512, 1 };
    const uint32_t ArrayLayers = 4;

    // TODO : Documentation
    StateTrackerValidationContext context;
    StateTrackerValidationContext::create(&context);
    load_gvk_state_tracker_entry_points();

    // TODO : Documentation
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

    // TODO : Documentation
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

    // TODO : Documentation
    auto dependencyInfo = gvk::get_default<VkDependencyInfo>();
    dependencyInfo.imageMemoryBarrierCount = (uint32_t)imageMemoryBarriers.size();
    dependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();

    // TODO : Documentation
    auto eventCreateInfo = gvk::get_default<VkEventCreateInfo>();
    gvk::Event event;
    ASSERT_EQ(gvk::Event::create(context.get_devices()[0], &eventCreateInfo, nullptr, &event), VK_SUCCESS);
    const auto& dispatchTable = gvk::DispatchTable::get_global_dispatch_table();
    assert(dispatchTable.gvkSetEvent);
    ASSERT_EQ(dispatchTable.gvkSetEvent(context.get_devices()[0], event), VK_SUCCESS);

    // TODO : Documentation
    gvk::execute_immediately(
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](const gvk::CommandBuffer& commandBuffer)
        {
            assert(dispatchTable.gvkCmdWaitEvents2);
            dispatchTable.gvkCmdWaitEvents2(commandBuffer, 1, &event.get<const VkEvent&>(), &dependencyInfo);
        }
    );

    // TODO : Documentation
    auto stateTrackedImage = gvk::get_state_tracked_object(image);
    std::vector<VkImageLayout> imageLayouts(imageCreateInfo.mipLevels * imageCreateInfo.arrayLayers);
    pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), imageLayouts.data());
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

/**
TODO : Documentation
*/
TEST(ImageLayout, RenderPass)
{
    // TODO : Documentation
    gvk::Context context;
    create_state_tracker_validation_context(&context);
    load_gvk_state_tracker_entry_points();

    // Get color VkFormat
    auto colorFormat = VK_FORMAT_UNDEFINED;
    gvk::enumerate_formats(
        context.get_devices()[0].get<gvk::PhysicalDevice>(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT,
        [&](VkFormat format)
        {
            const auto& formatInfo = gvk::get_format_info(format);
            if (formatInfo.components.size() == 4 &&
                formatInfo.bits_per_pixel() == 32 &&
                formatInfo.compressionType == gvk::CompressionType::CT_None &&
                formatInfo.numericFormat == gvk::NumericFormat::NF_UNORM &&
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
    auto requestedDepthBits = gvk::get_format_info(requestedDepthFormat).components[0].bits;
    gvk::enumerate_formats(
        context.get_devices()[0].get<gvk::PhysicalDevice>(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT,
        [&](VkFormat format)
        {
            if (format == requestedDepthFormat) {
                depthFormat = requestedDepthFormat;
            } else {
                auto actualDepthBits = depthFormat ? gvk::get_format_info(depthFormat).components[0].bits : 0;
                auto formatDepthBits = gvk::get_format_info(format).components[0].bits;
                if (actualDepthBits < formatDepthBits && formatDepthBits <= requestedDepthBits) {
                    depthFormat = format;
                }
            }
            return depthFormat != requestedDepthFormat;
        }
    );
    EXPECT_NE(depthFormat, VK_FORMAT_UNDEFINED);

    // Get VkSampleCountFlagBits
    const auto& physicalDevice = context.get_devices()[0].get<gvk::PhysicalDevice>();
    auto sampleCount = gvk::get_max_framebuffer_sample_count(physicalDevice, VK_TRUE, VK_TRUE, VK_FALSE);

    // Create gvk::RenderTarget
    auto renderTargetValidationCreateInfo = gvk::get_default<RenderTargetValidationCreateInfo>();
    renderTargetValidationCreateInfo.extent = { 1024, 1024 };
    renderTargetValidationCreateInfo.sampleCount = sampleCount;
    renderTargetValidationCreateInfo.colorFormat = colorFormat;
    renderTargetValidationCreateInfo.depthFormat = depthFormat;
    gvk::RenderTarget renderTarget;
    create_render_target(context, &renderTargetValidationCreateInfo, &renderTarget);

    // TODO : Documentation
    gvk::execute_immediately(
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](const gvk::CommandBuffer& commandBuffer)
        {
            auto renderPassBeginInfo = renderTarget.get_render_pass_begin_info();
            const auto& dispatchTable = gvk::DispatchTable::get_global_dispatch_table();
            assert(dispatchTable.gvkCmdBeginRenderPass);
            dispatchTable.gvkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            assert(dispatchTable.gvkCmdEndRenderPass);
            dispatchTable.gvkCmdEndRenderPass(commandBuffer);
        }
    );

    // TODO : Documentation
    auto renderPass = renderTarget.get_render_pass();
    ASSERT_TRUE(renderPass);
    auto renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo2>();
    for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
        auto image = renderTarget.get_image(i);
        ASSERT_TRUE(image);
        auto stateTrackedImage = gvk::get_state_tracked_object(image);
        VkImageLayout imageLayout { };
        pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), &imageLayout);
        EXPECT_EQ(imageLayout, renderPassCreateInfo.pAttachments[i].finalLayout);
    }
}

/**
TODO : Documentation
*/
TEST(ImageLayout, RenderPass2)
{
    // TODO : Documentation
    gvk::Context context;
    create_state_tracker_validation_context(&context);
    load_gvk_state_tracker_entry_points();

    // Get color VkFormat
    auto colorFormat = VK_FORMAT_UNDEFINED;
    gvk::enumerate_formats(
        context.get_devices()[0].get<gvk::PhysicalDevice>(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT,
        [&](VkFormat format)
        {
            const auto& formatInfo = gvk::get_format_info(format);
            if (formatInfo.components.size() == 4 &&
                formatInfo.bits_per_pixel() == 32 &&
                formatInfo.compressionType == gvk::CompressionType::CT_None &&
                formatInfo.numericFormat == gvk::NumericFormat::NF_UNORM &&
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
    auto requestedDepthBits = gvk::get_format_info(requestedDepthFormat).components[0].bits;
    gvk::enumerate_formats(
        context.get_devices()[0].get<gvk::PhysicalDevice>(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT,
        [&](VkFormat format)
        {
            if (format == requestedDepthFormat) {
                depthFormat = requestedDepthFormat;
            } else {
                auto actualDepthBits = depthFormat ? gvk::get_format_info(depthFormat).components[0].bits : 0;
                auto formatDepthBits = gvk::get_format_info(format).components[0].bits;
                if (actualDepthBits < formatDepthBits && formatDepthBits <= requestedDepthBits) {
                    depthFormat = format;
                }
            }
            return depthFormat != requestedDepthFormat;
        }
    );
    EXPECT_NE(depthFormat, VK_FORMAT_UNDEFINED);

    // Get VkSampleCountFlagBits
    const auto& physicalDevice = context.get_devices()[0].get<gvk::PhysicalDevice>();
    auto sampleCount = gvk::get_max_framebuffer_sample_count(physicalDevice, VK_TRUE, VK_TRUE, VK_FALSE);

    // Create gvk::RenderTarget
    auto renderTargetValidationCreateInfo = gvk::get_default<RenderTargetValidationCreateInfo>();
    renderTargetValidationCreateInfo.extent = { 1024, 1024 };
    renderTargetValidationCreateInfo.sampleCount = sampleCount;
    renderTargetValidationCreateInfo.colorFormat = colorFormat;
    renderTargetValidationCreateInfo.depthFormat = depthFormat;
    gvk::RenderTarget renderTarget;
    create_render_target(context, &renderTargetValidationCreateInfo, &renderTarget);

    // TODO : Documentation
    gvk::execute_immediately(
        gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
        context.get_command_buffers()[0],
        VK_NULL_HANDLE,
        [&](const gvk::CommandBuffer& commandBuffer)
        {
            auto renderPassBeginInfo = renderTarget.get_render_pass_begin_info();
            auto subpassBeginInfo = gvk::get_default<VkSubpassBeginInfo>();
            subpassBeginInfo.contents = VK_SUBPASS_CONTENTS_INLINE;
            const auto& dispatchTable = gvk::DispatchTable::get_global_dispatch_table();
            assert(dispatchTable.gvkCmdBeginRenderPass2);
            dispatchTable.gvkCmdBeginRenderPass2(commandBuffer, &renderPassBeginInfo, &subpassBeginInfo);
            assert(dispatchTable.gvkCmdEndRenderPass);
            dispatchTable.gvkCmdEndRenderPass(commandBuffer);
        }
    );

    // TODO : Documentation
    auto renderPass = renderTarget.get_render_pass();
    ASSERT_TRUE(renderPass);
    auto renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo2>();
    for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
        auto image = renderTarget.get_image(i);
        ASSERT_TRUE(image);
        auto stateTrackedImage = gvk::get_state_tracked_object(image);
        VkImageLayout imageLayout { };
        pfnGvkGetStateTrackedImageLayouts(&stateTrackedImage, &gvk::get_default<VkImageSubresourceRange>(), &imageLayout);
        EXPECT_EQ(imageLayout, renderPassCreateInfo.pAttachments[i].finalLayout);
    }
}
