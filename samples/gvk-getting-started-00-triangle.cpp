
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

int main(int, const char*[])
{
    // We'll start by opening a gvk_result_scope...this will declare a VkResult in
    //  the current scope named 'gvkResult' initialized with the specified VkResult
    //  value.  Whenever gvk_result() is called it will update 'gvkResult' with the
    //  provided value.  When a VkResult besides VK_SUCCESS is encountered, control
    //  will break from the gvk_result_scope...all normal stack unwinding rules (ie.
    //  destructors and desctructor order) apply...in debug configurations, it will
    //  simply assert()...
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Now we create a GvkSampleContext.  gvk::Context handles initialization of
        //  gvk::Instance, gvk::Device(s), gvk::WsiManager (Window System Integration),
        //  and several other utility objects.  GvkSampleContext extends gvk::Context.
        //  GvkSampleContext's definition can be found in "gvk-sample-utilities.hpp"...
        GvkSampleContext context;
        gvk_result(GvkSampleContext::create("Intel(R) GPA Utilities for Vulkan* - Getting Started - 00 - Triangle", &context));

        // Create a gvk::system::Surface.  This is used to control a system window...
        gvk::system::Surface systemSurface;
        gvk_result(gvk_sample_create_sys_surface(context, &systemSurface));

        // Create a gvk::WsiManager.  This is used to manage a connection between your
        //  Vulkan context and the system window referenced by the gvk::system::Surface...
        gvk::WsiManager wsiManager;
        gvk_result(gvk_sample_create_wsi_manager(context, systemSurface, &wsiManager));

        // We'll prepare two very simple shaders...
        gvk::spirv::ShaderInfo vertexShaderInfo{ };
        vertexShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.lineOffset = __LINE__;
        vertexShaderInfo.source = R"(
            #version 450

            layout(location = 0) out vec4 fsColor;

            out gl_PerVertex
            {
                vec4 gl_Position;
            };

            vec2 positions[3] = vec2[](
                vec2( 0.0, -0.5),
                vec2( 0.5,  0.5),
                vec2(-0.5,  0.5)
            );

            vec4 colors[3] = vec4[](
                vec4(1, 0, 0, 1),
                vec4(0, 1, 0, 1),
                vec4(0, 0, 1, 1)
            );

            void main()
            {
                // Use gl_VertexIndex to index into our positions[3] vec2 array...
                gl_Position = vec4(positions[gl_VertexIndex], 0, 1);

                // Use gl_VertexIndex to index into our colors[3] vec4 array...
                fsColor = colors[gl_VertexIndex];
            }
        )";
        gvk::spirv::ShaderInfo fragmentShaderInfo{ };
        fragmentShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.lineOffset = __LINE__;
        fragmentShaderInfo.source = R"(
            #version 450

            layout(location = 0) in vec4 fsColor;
            layout(location = 0) out vec4 fragColor;

            void main()
            {
                // Simply output the interpolated color...
                fragColor = fsColor;
            }
        )";

        // With our GLSL shaders prepared, we'll create a gvk::Pipeline...
        gvk::Pipeline pipeline;
        gvk_result(gvk_sample_create_pipeline(
            wsiManager.get_render_pass(),
            VK_CULL_MODE_BACK_BIT,
            vertexShaderInfo,
            fragmentShaderInfo,
            &pipeline
        ));

        // Loop until the user presses [esc] or closes the app window...
        while (
            !(systemSurface.get_input().keyboard.down(gvk::system::Key::Escape)) &&
            !(systemSurface.get_status() & gvk::system::Surface::CloseRequested)) {

            // Call the static function gvk::system::Surface::update() to cause all
            //  gvk::system::Surface objects to process window/input events...
            gvk::system::Surface::update();

            // Call wsiManager.update().  This will cause the WsiManager to respond to system
            //  updates for the SurfaceKHR it's managing.  This call may cause resources to
            //  be created/destroyed.  If there's a valid SwapchainKHR, render and present.
            wsiManager.update();
            auto swapchain = wsiManager.get_swapchain();
            if (swapchain) {
                // Acquire the next image to render to.  The index will be used to access the
                //  acquired image as well as the command buffer and fence associated with that
                //  image.  Note that this method may return VK_SUBOPTIMAL_KHR...gvk::WsiManager
                //  will update itself when this occurs so there's no need to bail from the
                //  gvk_result_scope().
                uint32_t imageIndex = 0;
                auto vkResult = wsiManager.acquire_next_image(UINT64_MAX, VK_NULL_HANDLE, &imageIndex);
                gvk_result((vkResult == VK_SUCCESS || vkResult == VK_SUBOPTIMAL_KHR) ? VK_SUCCESS : vkResult);

                // Using the acquired image index we'll wait on the associated fence.  This
                //  ensures that the image isn't currently in use via vkQueueSubmit().  After
                //  waiting on the fence, we'll immediately reset it so that it's ready to be
                //  used in the next call to vkQueueSubmit().
                const auto& device = context.get_devices()[0];
                const auto& vkFences = wsiManager.get_vk_fences();
                gvk_result(vkWaitForFences(device, 1, &vkFences[imageIndex], VK_TRUE, UINT64_MAX));
                gvk_result(vkResetFences(device, 1, &vkFences[imageIndex]));

                // To render the triangle we'll start by beginning command buffer recording and
                //  and beginning a render pass.  We'll get a VkRenderPassBeginInfo from the
                //  gvk::RenderTarget (which is really just a gvk::Framebuffer with some added
                //  functionality) we're rendering into.  The VkRenderPassBeginInfo that we get
                //  from the gvk::RenderTarget is populated with default values and can be
                //  modified before calling vkCmdBeginRenderPass()...
                const auto& commandBuffer = wsiManager.get_command_buffers()[imageIndex];
                gvk_result(vkBeginCommandBuffer(commandBuffer, &gvk::get_default<VkCommandBufferBeginInfo>()));
                auto renderPassBeginInfo = wsiManager.get_render_targets()[imageIndex].get_render_pass_begin_info();
                vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                // Set our scissor and viewport to match our renderPassBeginInfo.renderArea...
                VkRect2D scissor { { }, renderPassBeginInfo.renderArea.extent };
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
                VkViewport viewport { 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                // Bind our pipeline and draw.  We're not binding a vertex buffer because our
                //  vertex positions and colors are hardcoded in arrays defined in our vertex
                //  shader.  We pass 3 for our vertexCount to vkCmdDraw(), in the vertex shader
                //  we get the vertex index using gl_VertexIndex...
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdDraw(commandBuffer, 3, 1, 0, 0);

                // After drawing our triangle we end the render pass and end the command buffer...
                vkCmdEndRenderPass(commandBuffer);
                gvk_result(vkEndCommandBuffer(commandBuffer));

                // Submit the command buffer associated with the acquired image.
                //  wsiManager.get_submit_info() prepares a VkSubmitInfo for the indexed
                //  command buffer.
                const auto& queue = gvk::get_queue_family(device, 0).queues[0];
                auto submitInfo = wsiManager.get_submit_info(imageIndex);
                gvk_result(vkQueueSubmit(queue, 1, &submitInfo, vkFences[imageIndex]));

                // Present the acquired image.  wsiManager.get_present_info() prepares a
                //  VkPresentInfoKHR for the acquired image.
                auto presentInfo = wsiManager.get_present_info(&imageIndex);
                vkResult = vkQueuePresentKHR(gvk::get_queue_family(context.get_devices()[0], 0).queues[0], &presentInfo);
                gvk_result((vkResult == VK_SUCCESS || vkResult == VK_SUBOPTIMAL_KHR) ? VK_SUCCESS : vkResult);
            }
        }

        // gvk::Device calls vkDeviceWaitIdle() in its dtor, but we need to make sure
        //  that we don't fall out of this scope and start running dtors for other
        //  objects (in this case our gvk::Pipeline) until we're sure they're done
        //  being used so we call vkDeviceWaitIdle() before everything is torn down.
        gvk_result(vkDeviceWaitIdle(context.get_devices()[0]));
    } gvk_result_scope_end;

    // Finally, if we encountered a Vulkan error, output the error to std::cerr...
    if (gvkResult) {
        std::cerr << gvk::to_string(gvkResult) << std::endl;
    }
    return (int)gvkResult;
}
