
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
        gvk_result(GvkSampleContext::create("Intel GVK - Getting Started - 01 - Mesh", &context));

        gvk::spirv::ShaderInfo vertexShaderInfo{
            .language = gvk::spirv::ShadingLanguage::Glsl,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .lineOffset = __LINE__,
            .source = R"(
                #version 450

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
            )"
        };
        gvk::spirv::ShaderInfo fragmentShaderInfo{
            .language = gvk::spirv::ShadingLanguage::Glsl,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .lineOffset = __LINE__,
            .source = R"(
                #version 450

                layout(location = 0) in vec4 fsColor;
                layout(location = 0) out vec4 fragColor;

                void main()
                {
                    fragColor = fsColor;
                }
            )"
        };
        gvk::Pipeline pipeline;
        gvk_result(gvk_sample_create_pipeline<VertexPositionColor>(
            context.get_wsi_manager().get_render_pass(),
            VK_CULL_MODE_BACK_BIT,
            vertexShaderInfo,
            fragmentShaderInfo,
            &pipeline
        ));

        // Create our gvk::Mesh...
        gvk::Mesh mesh;
        gvk_result(create_mesh(context, &mesh));

        while (
            !(context.get_sys_surface().get_input().keyboard.down(gvk::sys::Key::Escape)) &&
            !(context.get_sys_surface().get_status() & gvk::sys::Surface::CloseRequested)) {
            gvk::sys::Surface::update();
            auto& wsiManager = context.get_wsi_manager();
            if (wsiManager.update()) {
                auto extent = wsiManager.get_swapchain().get<VkSwapchainCreateInfoKHR>().imageExtent;
                for (size_t i = 0; i < wsiManager.get_command_buffers().size(); ++i) {
                    const auto& commandBuffer = wsiManager.get_command_buffers()[i];
                    gvk_result(vkBeginCommandBuffer(commandBuffer, &gvk::get_default<VkCommandBufferBeginInfo>()));
                    auto renderPassBeginInfo = wsiManager.get_render_targets()[i].get_render_pass_begin_info();
                    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                    VkRect2D scissor{ .extent = extent };
                    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
                    VkViewport viewport{ .width = (float)extent.width, .height = (float)extent.height, .minDepth = 0, .maxDepth = 1 };
                    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                    // Record gvk::Mesh draw cmds...
                    mesh.record_cmds(commandBuffer);

                    vkCmdEndRenderPass(commandBuffer);
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
