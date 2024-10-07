
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

VkResult create_mesh(const gvk::Context& context, gvk::Mesh* pMesh)
{
    assert(pMesh);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // gvk::Mesh provides high level control over gvk::Buffer objects and
        //  gvk::CommandBuffer recording for drawing with vertex/index data.  Here
        //  we'll create a gvk::Mesh with VertexPositionColor vertex data.  This
        //  is a simple struct that has a glm::vec3 member for position and a glm::vec4
        //  member for color.  We'll prepare 4 vertices to create a quad and feed that
        //  into our gvk::Mesh object's write() method.
        std::array<VertexPositionColor, 4> vertices {
            VertexPositionColor {{ -0.5f, -0.5f, 0.0f }, { gvk::math::Color::OrangeRed }},
            VertexPositionColor {{  0.5f, -0.5f, 0.0f }, { gvk::math::Color::BlueViolet }},
            VertexPositionColor {{  0.5f,  0.5f, 0.0f }, { gvk::math::Color::DodgerBlue }},
            VertexPositionColor {{ -0.5f,  0.5f, 0.0f }, { gvk::math::Color::Goldenrod }},
        };
        std::array<uint16_t, 6> indices {
            0, 1, 2,
            2, 3, 0,
        };
        gvk_result(pMesh->write(
            context.get<gvk::Devices>()[0],
            gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0],
            context.get<gvk::CommandBuffers>()[0],
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

        GvkSampleContext context = gvk::nullref;
        gvk_result(GvkSampleContext::create("Intel(R) GPA Utilities for Vulkan* - Getting Started - 01 - Mesh", &context));

        gvk::system::Surface systemSurface = gvk::nullref;
        gvk_result(gvk_sample_create_sys_surface(context, &systemSurface));

        gvk::wsi::Context wsiContext = gvk::nullref;
        gvk_result(gvk_sample_create_wsi_context(context, systemSurface, &wsiContext));

        auto vertexShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        vertexShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.lineOffset = __LINE__;
        vertexShaderInfo.source = R"(
            #version 450

            /**
            These declarations describe vertex input.  In the CPU side code we're using
                VertexPositionColor, which has vec3 and vec4 members that will be mapped to
                these two input layout locations.  Note that we're passing the vertex type
                to the following call to gvk_sample_create_pipeline<>().  Vertex info for
                VertexPositionColor is accessed via gvk::get_vertex_description<>() and
                gvk::get_vertex_input_attribute_format<>()...these functions should be
                specialized for any custom vertex types.
            */
            layout(location = 0) in vec3 vsPosition;
            layout(location = 1) in vec4 vsColor;

            layout(location = 0) out vec4 fsColor;

            out gl_PerVertex
            {
                vec4 gl_Position;
            };

            void main()
            {
                gl_Position = vec4(vsPosition, 1);
                fsColor = vsColor;
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
                fragColor = fsColor;
            }
        )";
        gvk::Pipeline pipeline = VK_NULL_HANDLE;
        gvk_result(gvk_sample_create_pipeline<VertexPositionColor>(
            wsiContext.get<gvk::RenderPass>(),
            VK_CULL_MODE_BACK_BIT,
            vertexShaderInfo,
            fragmentShaderInfo,
            &pipeline
        ));

        // Create our gvk::Mesh...
        gvk::Mesh mesh;
        gvk_result(create_mesh(context, &mesh));

        while (
            !(systemSurface.get<gvk::system::Input>().keyboard.down(gvk::system::Key::Escape)) &&
            !(systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::CloseRequested)) {

            gvk::system::Surface::update();
            gvk::wsi::AcquiredImageInfo acquiredImageInfo{ };
            gvk::RenderTarget acquiredImageRenderTarget = gvk::nullref;
            auto wsiStatus = wsiContext.acquire_next_image(UINT64_MAX, VK_NULL_HANDLE, &acquiredImageInfo, &acquiredImageRenderTarget);
            if (wsiStatus == VK_SUCCESS || wsiStatus == VK_SUBOPTIMAL_KHR) {

                gvk_result(vkBeginCommandBuffer(acquiredImageInfo.commandBuffer, &gvk::get_default<VkCommandBufferBeginInfo>()));
                const auto& renderPassBeginInfo = acquiredImageRenderTarget.get<VkRenderPassBeginInfo>();
                vkCmdBeginRenderPass(acquiredImageInfo.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                VkRect2D scissor { { }, renderPassBeginInfo.renderArea.extent };
                vkCmdSetScissor(acquiredImageInfo.commandBuffer, 0, 1, &scissor);
                VkViewport viewport { 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                vkCmdSetViewport(acquiredImageInfo.commandBuffer, 0, 1, &viewport);

                vkCmdBindPipeline(acquiredImageInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                // Record gvk::Mesh draw cmds...
                mesh.record_cmds(acquiredImageInfo.commandBuffer);

                vkCmdEndRenderPass(acquiredImageInfo.commandBuffer);
                gvk_result(vkEndCommandBuffer(acquiredImageInfo.commandBuffer));

                const auto& queue = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0];
                gvk_result(vkQueueSubmit(queue, 1, &wsiContext.get<VkSubmitInfo>(acquiredImageInfo), acquiredImageInfo.fence));

                wsiStatus = wsiContext.queue_present(queue, &acquiredImageInfo);
                gvk_result((wsiStatus == VK_SUBOPTIMAL_KHR || wsiStatus == VK_ERROR_OUT_OF_DATE_KHR) ? VK_SUCCESS : wsiStatus);
            }
        }
        gvk_result(vkDeviceWaitIdle(context.get<gvk::Devices>()[0]));
    } gvk_result_scope_end;
    if (gvkResult) {
        std::cerr << gvk::to_string(gvkResult) << std::endl;
    }
    return (int)gvkResult;
}
