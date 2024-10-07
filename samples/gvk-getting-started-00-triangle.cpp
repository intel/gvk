
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
        //  gvk::Instance, gvk::Device(s), and several other utility objects.
        //  GvkSampleContext found in "gvk-sample-utilities.hpp" extends gvk::Context...
        GvkSampleContext context = gvk::nullref;
        gvk_result(GvkSampleContext::create("Intel(R) GPA Utilities for Vulkan* - Getting Started - 00 - Triangle", &context));

        // Create a gvk::system::Surface.  This is used to control a system window...
        gvk::system::Surface systemSurface = gvk::nullref;
        gvk_result(gvk_sample_create_sys_surface(context, &systemSurface));

        // Create a gvk::wsi::Context.  This is used to manage a connection between the
        //  Vulkan and the system window referenced by the gvk::system::Surface...
        gvk::wsi::Context wsiContext = gvk::nullref;
        gvk_result(gvk_sample_create_wsi_context(context, systemSurface, &wsiContext));

        // We'll prepare two very simple shaders...
        auto vertexShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
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
        auto fragmentShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
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
        gvk::Pipeline pipeline = VK_NULL_HANDLE;
        gvk_result(gvk_sample_create_pipeline(
            wsiContext.get<gvk::RenderPass>(),
            VK_CULL_MODE_BACK_BIT,
            vertexShaderInfo,
            fragmentShaderInfo,
            &pipeline
        ));

        // Loop until the user presses [esc] or closes the app window...
        while (
            !(systemSurface.get<gvk::system::Input>().keyboard.down(gvk::system::Key::Escape)) &&
            !(systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::CloseRequested)) {

            // Call the static function gvk::system::Surface::update() to cause all
            //  gvk::system::Surface objects to process window/input events...
            gvk::system::Surface::update();

            // Call wsiContext.acquire_next_image(), causing the gvk::wsi::Context to
            //  respond to updates for the SurfaceKHR and SwapchainKHR.  This call may
            //  cause resources to be created/destroyed.  If an image is successfully
            //  acquired, the gvk::wsi::AcquiredImageInfo and optional gvk::RenderTarget
            //  arguments will be populated and can be used to render/present...
            // NOTE : VK_SUBOPTIMAL_KHR isn't an error, so we're not using gvk_result()
            //  because we don't want to bail when the SurfaceKHR and SwapchainKHR are
            //  no longer a perfect match.  We can still render to the SwapchainKHR in
            //  this case.  After presentation, the SwapchainKHR and assoicated resources
            //  will be recreated.
            gvk::wsi::AcquiredImageInfo acquiredImageInfo{ };
            gvk::RenderTarget acquiredImageRenderTarget{ };
            auto wsiStatus = wsiContext.acquire_next_image(UINT64_MAX, VK_NULL_HANDLE, &acquiredImageInfo, &acquiredImageRenderTarget);
            if (wsiStatus == VK_SUCCESS || wsiStatus == VK_SUBOPTIMAL_KHR) {

                // To render the triangle we'll start by beginning command buffer recording and
                //  beginning a render pass.  We'll get a VkRenderPassBeginInfo from the
                //  gvk::RenderTarget (which is really just a gvk::Framebuffer with some added
                //  functionality) we're rendering into.  The VkRenderPassBeginInfo that we get
                //  from the gvk::RenderTarget is populated with default values and can be
                //  modified before calling vkCmdBeginRenderPass()...
                gvk_result(vkBeginCommandBuffer(acquiredImageInfo.commandBuffer, &gvk::get_default<VkCommandBufferBeginInfo>()));
                const auto& renderPassBeginInfo = acquiredImageRenderTarget.get<VkRenderPassBeginInfo>();
                vkCmdBeginRenderPass(acquiredImageInfo.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                // Set our scissor and viewport to match our renderPassBeginInfo.renderArea...
                VkRect2D scissor { { }, renderPassBeginInfo.renderArea.extent };
                vkCmdSetScissor(acquiredImageInfo.commandBuffer, 0, 1, &scissor);
                VkViewport viewport { 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                vkCmdSetViewport(acquiredImageInfo.commandBuffer, 0, 1, &viewport);

                // Bind our pipeline and draw.  We're not binding a vertex buffer because our
                //  vertex positions and colors are hardcoded in arrays defined in our vertex
                //  shader.  We pass 3 for our vertexCount to vkCmdDraw(), in the vertex shader
                //  we get the vertex index using gl_VertexIndex...
                vkCmdBindPipeline(acquiredImageInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdDraw(acquiredImageInfo.commandBuffer, 3, 1, 0, 0);

                // After drawing our triangle we end the render pass and end the command buffer...
                vkCmdEndRenderPass(acquiredImageInfo.commandBuffer);
                gvk_result(vkEndCommandBuffer(acquiredImageInfo.commandBuffer));

                // Submit the command buffer associated with the acquired image.
                //  wsiContext.get<VkSubmitInfo>() prepares a VkSubmitInfo for the given
                //  gvk::wsi::AcquiredImageInfo...
                const auto& queue = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0];
                gvk_result(vkQueueSubmit(queue, 1, &wsiContext.get<VkSubmitInfo>(acquiredImageInfo), acquiredImageInfo.fence));

                // Present the acquired image...
                wsiStatus = wsiContext.queue_present(queue, &acquiredImageInfo);
            }

            // Validate wsiStatus...
            // NOTE : If wsiStatus is VK_SUBOPTIMAL_KHR, VK_ERROR_OUT_OF_DATE_KHR, or if
            //  there's a mismatch between SurfaceKHR and SwapchainKHR extents, then the
            //  SwapchainKHR and associated resources will be recreated after presentation.
            //  If there are any application resources dependent on these resources, they
            //  should be recreated here as well.
            gvk_result((wsiStatus == VK_SUBOPTIMAL_KHR || wsiStatus == VK_ERROR_OUT_OF_DATE_KHR) ? VK_SUCCESS : wsiStatus);
        }

        // gvk::Device calls vkDeviceWaitIdle() in its dtor, but we need to make sure
        //  that we don't fall out of this scope and start running dtors for other
        //  objects until they're done being used so we call vkDeviceWwaitidle() before
        //  everything is torn down...
        gvk_result(vkDeviceWaitIdle(context.get<gvk::Devices>()[0]));
    } gvk_result_scope_end;
    if (gvkResult) {
        // Finally, if we encountered a Vulkan error, output the error to std::cerr...
        std::cerr << gvk::to_string(gvkResult) << std::endl;
    }
    return (int)gvkResult;
}
