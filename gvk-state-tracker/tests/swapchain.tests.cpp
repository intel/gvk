
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

TEST(Swapchain, SwapchainResourceLifetime)
{
    StateTrackerValidationContext context;
    ASSERT_EQ(StateTrackerValidationContext::create(&context), VK_SUCCESS);
    load_gvk_state_tracker_entry_points();
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    auto systemSurfaceCreateInfo = gvk::get_default<gvk::system::Surface::CreateInfo>();
    systemSurfaceCreateInfo.pTitle = "VK_LAYER_INTEL_gvk_state_tracker - Tests - Swapchain - SwapchainResourceLifetime";
    gvk::system::Surface systemSurface;
    ASSERT_TRUE(gvk::system::Surface::create(&systemSurfaceCreateInfo, &systemSurface));

    auto wsiManagerCreateInfo = gvk::get_default<gvk::WsiManager::CreateInfo>();
#ifdef VK_USE_PLATFORM_XLIB_KHR
    auto xlibSurfaceCreateInfo = gvk::get_default<VkXlibSurfaceCreateInfoKHR>();
    xlibSurfaceCreateInfo.dpy = (Display*)systemSurface.get_display();
    xlibSurfaceCreateInfo.window = (Window)systemSurface.get_window();
    wsiManagerCreateInfo.pXlibSurfaceCreateInfoKHR = &xlibSurfaceCreateInfo;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    auto win32SurfaceCreateInfo = gvk::get_default<VkWin32SurfaceCreateInfoKHR>();
    win32SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
    win32SurfaceCreateInfo.hwnd = (HWND)systemSurface.get_hwnd();
    wsiManagerCreateInfo.pWin32SurfaceCreateInfoKHR = &win32SurfaceCreateInfo;
#endif
    wsiManagerCreateInfo.queueFamilyIndex = gvk::get_queue_family(context.get_devices()[0], 0).queues[0].get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
    gvk::WsiManager wsiManager;
    ASSERT_EQ(gvk::WsiManager::create(context.get_devices()[0], &wsiManagerCreateInfo, nullptr, &wsiManager), VK_SUCCESS);
#ifdef VK_USE_PLATFORM_XLIB_KHR
    ASSERT_TRUE(create_state_tracked_object_record(wsiManager.get_surface(), wsiManager.get_surface().get<VkXlibSurfaceCreateInfoKHR>(), expectedInstanceObjects));
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    ASSERT_TRUE(create_state_tracked_object_record(wsiManager.get_surface(), wsiManager.get_surface().get<VkWin32SurfaceCreateInfoKHR>(), expectedInstanceObjects));
#endif
    ASSERT_TRUE(create_state_tracked_object_record(wsiManager.get_swapchain(), wsiManager.get_swapchain().get<VkSwapchainCreateInfoKHR>(), expectedInstanceObjects));
    ASSERT_FALSE(wsiManager.get_swapchain().get<gvk::Images>().empty());
    for (const auto& renderTarget : wsiManager.get_render_targets()) {
        auto framebuffer = renderTarget.get_framebuffer();
        auto renderPass = framebuffer.get<gvk::RenderPass>();
        ASSERT_TRUE(create_state_tracked_object_record(framebuffer, framebuffer.get<VkFramebufferCreateInfo>(), expectedInstanceObjects));
        ASSERT_TRUE(create_state_tracked_object_record(renderPass, renderPass.get<VkRenderPassCreateInfo2>(), expectedInstanceObjects));
        for (const auto& imageView : framebuffer.get<gvk::ImageViews>()) {
            auto image = imageView.get<gvk::Image>();
            ASSERT_TRUE(create_state_tracked_object_record(imageView, imageView.get<VkImageViewCreateInfo>(), expectedInstanceObjects));
            ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedInstanceObjects));
        }
    }
    for (const auto& image : wsiManager.get_swapchain().get<gvk::Images>()) {
        ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedInstanceObjects));
    }
    for (const auto& commandBuffer : wsiManager.get_command_buffers()) {
        auto commandPool = commandBuffer.get<gvk::CommandPool>();
        ASSERT_TRUE(create_state_tracked_object_record(commandBuffer, commandBuffer.get<VkCommandBufferAllocateInfo>(), expectedInstanceObjects));
        ASSERT_TRUE(create_state_tracked_object_record(commandPool, commandPool.get<VkCommandPoolCreateInfo>(), expectedInstanceObjects));
    }
    for (const auto& fence : wsiManager.get_fences()) {
        ASSERT_TRUE(create_state_tracked_object_record(fence, fence.get<VkFenceCreateInfo>(), expectedInstanceObjects));
    }
    ASSERT_TRUE(create_state_tracked_object_record(wsiManager.get_image_acquired_semaphore(), wsiManager.get_image_acquired_semaphore().get<VkSemaphoreCreateInfo>(), expectedInstanceObjects));
    ASSERT_TRUE(create_state_tracked_object_record(wsiManager.get_image_rendered_semaphore(), wsiManager.get_image_rendered_semaphore().get<VkSemaphoreCreateInfo>(), expectedInstanceObjects));

    std::map<GvkStateTrackedObject, ObjectRecord> expectedImageDependencies;
    ASSERT_TRUE(create_state_tracked_object_record(wsiManager.get_swapchain(), wsiManager.get_swapchain().get<VkSwapchainCreateInfoKHR>(), expectedImageDependencies));
#ifdef VK_USE_PLATFORM_XLIB_KHR
    ASSERT_TRUE(create_state_tracked_object_record(wsiManager.get_surface(), wsiManager.get_surface().get<VkXlibSurfaceCreateInfoKHR>(), expectedImageDependencies));
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    ASSERT_TRUE(create_state_tracked_object_record(wsiManager.get_surface(), wsiManager.get_surface().get<VkWin32SurfaceCreateInfoKHR>(), expectedImageDependencies));
#endif
    ASSERT_TRUE(create_state_tracked_object_record(context.get_devices()[0], context.get_devices()[0].get<VkDeviceCreateInfo>(), expectedImageDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_physical_devices()[0], context.get_instance().get<VkInstanceCreateInfo>(), expectedImageDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get_instance(), context.get_instance().get<VkInstanceCreateInfo>(), expectedImageDependencies));

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get_instance());
    pfnGvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    for (const auto& image : wsiManager.get_swapchain().get<gvk::Images>()) {
        enumerator.records.clear();
        auto stateTrackedImage = gvk::get_state_tracked_object(image);
        pfnGvkEnumerateStateTrackedObjectDependencies(&stateTrackedImage, &enumerateInfo);
        validate(gvk_file_line, expectedImageDependencies, enumerator.records);
    }
}
