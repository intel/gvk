
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
    auto expectedInstanceObjects = get_expected_instance_objects(context);

    auto systemSurfaceCreateInfo = gvk::get_default<gvk::system::Surface::CreateInfo>();
    systemSurfaceCreateInfo.pTitle = "VK_LAYER_INTEL_gvk_state_tracker - Tests - Swapchain - SwapchainResourceLifetime";
    gvk::system::Surface systemSurface;
    ASSERT_EQ((VkResult)gvk::system::Surface::create(&systemSurfaceCreateInfo, &systemSurface), VK_SUCCESS);

    // Create gvk::SurfaceKHR
    const VkBaseInStructure* pSurfaceCreateInfo = nullptr;
#ifdef VK_USE_PLATFORM_XLIB_KHR
    auto xlibSurfaceCreateInfo = gvk::get_default<VkXlibSurfaceCreateInfoKHR>();
    xlibSurfaceCreateInfo.dpy = systemSurface.get<gvk::system::Surface::PlatformInfo>().x11Display;
    xlibSurfaceCreateInfo.window = systemSurface.get<gvk::system::Surface::PlatformInfo>().x11Window;
    pSurfaceCreateInfo = (VkBaseInStructure*)&xlibSurfaceCreateInfo;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    auto win32SurfaceCreateInfo = gvk::get_default<VkWin32SurfaceCreateInfoKHR>();
    win32SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
    win32SurfaceCreateInfo.hwnd = systemSurface.get<gvk::system::Surface::PlatformInfo>().hwnd;
    pSurfaceCreateInfo = (VkBaseInStructure*)&win32SurfaceCreateInfo;
#endif
    gvk::SurfaceKHR surface = VK_NULL_HANDLE;
    ASSERT_EQ(gvk::SurfaceKHR::create(context.get<gvk::Devices>()[0].get<gvk::Instance>(), pSurfaceCreateInfo, nullptr, &surface), VK_SUCCESS);

    // Create gvk::wsi::Context
    auto wsiContextCreateInfo = gvk::get_default<gvk::wsi::Context::CreateInfo>();
    wsiContextCreateInfo.queueFamilyIndex = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0].get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
    gvk::wsi::Context wsiContext = gvk::nullref;
    ASSERT_EQ(gvk::wsi::Context::create(context.get<gvk::Devices>()[0], surface, &wsiContextCreateInfo, nullptr, &wsiContext), VK_SUCCESS);

#ifdef VK_USE_PLATFORM_XLIB_KHR
    ASSERT_TRUE(create_state_tracked_object_record(wsiContext.get<gvk::SurfaceKHR>(), wsiContext.get<gvk::SurfaceKHR>().get<VkXlibSurfaceCreateInfoKHR>(), expectedInstanceObjects));
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    ASSERT_TRUE(create_state_tracked_object_record(wsiContext.get<gvk::SurfaceKHR>(), wsiContext.get<gvk::SurfaceKHR>().get<VkWin32SurfaceCreateInfoKHR>(), expectedInstanceObjects));
#endif
    ASSERT_TRUE(create_state_tracked_object_record(wsiContext.get<gvk::SwapchainKHR>(), wsiContext.get<gvk::SwapchainKHR>().get<VkSwapchainCreateInfoKHR>(), expectedInstanceObjects));
    ASSERT_FALSE(wsiContext.get<gvk::SwapchainKHR>().get<gvk::Images>().empty());
    for (const auto& renderTarget : wsiContext.get<gvk::RenderTargets>()) {
        const auto& framebuffer = renderTarget.get<gvk::Framebuffer>();
        const auto& renderPass = framebuffer.get<gvk::RenderPass>();
        ASSERT_TRUE(create_state_tracked_object_record(framebuffer, framebuffer.get<VkFramebufferCreateInfo>(), expectedInstanceObjects));
        ASSERT_TRUE(create_state_tracked_object_record(renderPass, renderPass.get<VkRenderPassCreateInfo2>(), expectedInstanceObjects));
        for (const auto& imageView : framebuffer.get<gvk::ImageViews>()) {
            const auto& image = imageView.get<gvk::Image>();
            ASSERT_TRUE(create_state_tracked_object_record(imageView, imageView.get<VkImageViewCreateInfo>(), expectedInstanceObjects));
            ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedInstanceObjects));
        }
    }
    for (const auto& image : wsiContext.get<gvk::SwapchainKHR>().get<gvk::Images>()) {
        ASSERT_TRUE(create_state_tracked_object_record(image, image.get<VkImageCreateInfo>(), expectedInstanceObjects));
    }
    const auto& wsiContextCommandResources = wsiContext.get<std::vector<gvk::wsi::Context::CommandResources>>();
    ASSERT_FALSE(wsiContextCommandResources.empty());
    for (const auto& commandResources : wsiContextCommandResources) {
        const auto& commandBuffer = commandResources.commandBuffer;
        const auto& commandPool = commandBuffer.get<gvk::CommandPool>();
        ASSERT_TRUE(create_state_tracked_object_record(commandBuffer, commandBuffer.get<VkCommandBufferAllocateInfo>(), expectedInstanceObjects));
        ASSERT_TRUE(create_state_tracked_object_record(commandPool, commandPool.get<VkCommandPoolCreateInfo>(), expectedInstanceObjects));
    }
    expectedInstanceObjects[gvk::get_state_tracked_object(wsiContextCommandResources[0].commandBuffer)].mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKED_OBJECT_STATUS_INVALID_BIT;
    for (const auto& commandResources : wsiContextCommandResources) {
        const auto& fence = commandResources.fence;
        const auto& imageAcquiredSemaphore = commandResources.imageAcquiredSemaphore;
        const auto& presentWaitSemaphore = commandResources.presentWaitSemaphore;
        ASSERT_TRUE(create_state_tracked_object_record(fence, fence.get<VkFenceCreateInfo>(), expectedInstanceObjects));
        ASSERT_TRUE(create_state_tracked_object_record(imageAcquiredSemaphore, imageAcquiredSemaphore.get<VkSemaphoreCreateInfo>(), expectedInstanceObjects));
        ASSERT_TRUE(create_state_tracked_object_record(presentWaitSemaphore, presentWaitSemaphore.get<VkSemaphoreCreateInfo>(), expectedInstanceObjects));
    }

    std::map<GvkStateTrackedObject, ObjectRecord> expectedImageDependencies;
    ASSERT_TRUE(create_state_tracked_object_record(wsiContext.get<gvk::SwapchainKHR>(), wsiContext.get<gvk::SwapchainKHR>().get<VkSwapchainCreateInfoKHR>(), expectedImageDependencies));
#ifdef VK_USE_PLATFORM_XLIB_KHR
    ASSERT_TRUE(create_state_tracked_object_record(wsiContext.get<gvk::SurfaceKHR>(), wsiContext.get<gvk::SurfaceKHR>().get<VkXlibSurfaceCreateInfoKHR>(), expectedImageDependencies));
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    ASSERT_TRUE(create_state_tracked_object_record(wsiContext.get<gvk::SurfaceKHR>(), wsiContext.get<gvk::SurfaceKHR>().get<VkWin32SurfaceCreateInfoKHR>(), expectedImageDependencies));
#endif
    ASSERT_TRUE(create_state_tracked_object_record(context.get<gvk::Devices>()[0], context.get<gvk::Devices>()[0].get<VkDeviceCreateInfo>(), expectedImageDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get<gvk::PhysicalDevices>()[0], VkApplicationInfo { }, expectedImageDependencies));
    ASSERT_TRUE(create_state_tracked_object_record(context.get<gvk::Instance>(), context.get<gvk::Instance>().get<VkInstanceCreateInfo>(), expectedImageDependencies));

    StateTrackerValidationEnumerator enumerator;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = StateTrackerValidationEnumerator::enumerate;
    enumerateInfo.pUserData = &enumerator;
    auto stateTrackedInstance = gvk::get_state_tracked_object(context.get<gvk::Instance>());
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);
    validate(gvk_file_line, expectedInstanceObjects, enumerator.records);

    for (const auto& image : wsiContext.get<gvk::SwapchainKHR>().get<gvk::Images>()) {
        enumerator.records.clear();
        auto stateTrackedImage = gvk::get_state_tracked_object(image);
        gvkEnumerateStateTrackedObjectDependencies(&stateTrackedImage, &enumerateInfo);
        validate(gvk_file_line, expectedImageDependencies, enumerator.records);
    }
}
