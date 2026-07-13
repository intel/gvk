
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

#include "utilities.hpp"

namespace gvk {
namespace sph {

// Quad vertex for instanced rendering
struct QuadVertex
{
    glm::vec2 position;
};

// Push constants for rendering
struct RenderPushConstants
{
    glm::mat4 viewProjection;
    glm::vec3 cameraRight;
    float particleScale;
    glm::vec3 cameraUp;
    uint32_t showCollisionCircles;
};

inline VkResult create_graphics_pipeline(const gvk::Device& device, const gvk::RenderPass& renderPass, gvk::spirv::Context& spirvContext, gvk::Pipeline* pPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Vertex shader: Instanced quad rendering with billboarding
        auto vertexShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        vertexShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.lineOffset = __LINE__;
        vertexShaderInfo.source = R"(
            #version 450

            struct Particle
            {
                vec4 position;    // xyz = position, w = radius
                vec4 velocity;    // xyz = velocity, w = mass
                vec4 color;       // rgba color
                vec4 force;       // xyz = accumulated force, w = padding
                vec4 homePosition; // xyz = original position, w = padding
                float density;
                float pressure;
                float scale;      // Per-particle visual scale
                float wakeRadius;  // Wake distance
                uint gridCell;
                uint sleepCounter; // Frames stationary
                float padding1;
                float padding2;
            };

            layout(binding = 0) readonly buffer ParticleBuffer
            {
                Particle particles[];
            };

            layout(push_constant) uniform PushConstants
            {
                mat4 viewProjection;           // 64 bytes (0-63)
                vec3 cameraRight;              // 12 bytes (64-75) + 4 bytes padding
                float particleScale;           // 4 bytes  (76-79)
                vec3 cameraUp;                 // 12 bytes (80-91) + 4 bytes padding
                uint showCollisionCircles;     // 4 bytes  (92-95)
                uint selectedParticleIndex;    // 4 bytes  (96-99)
                float selectedParticleScale;   // 4 bytes  (100-103)
                uint hardEdges;                // 4 bytes  (104-107)
                uint showVelocityColors;       // 4 bytes  (108-111)
            } pushConstants;

            layout(location = 0) in vec2 quadVertex;

            layout(location = 0) out vec2 fsTexCoord;
            layout(location = 1) out vec4 fsColor;
            layout(location = 2) flat out uint fsIsSelected;
            layout(location = 3) out float fsVelocityMag;

            out gl_PerVertex
            {
                vec4 gl_Position;
            };

            void main()
            {
                Particle p = particles[gl_InstanceIndex];

                // Use per-particle scale
                float scale = p.scale;

                // Billboard the quad to face camera
                vec3 worldPos = p.position.xyz;
                worldPos += pushConstants.cameraRight * quadVertex.x * p.position.w * scale;
                worldPos += pushConstants.cameraUp * quadVertex.y * p.position.w * scale;

                gl_Position = pushConstants.viewProjection * vec4(worldPos, 1.0);

                fsTexCoord = quadVertex * 0.5 + 0.5;
                fsColor = p.color;
                fsIsSelected = (gl_InstanceIndex == pushConstants.selectedParticleIndex) ? 1u : 0u;
                fsVelocityMag = length(p.velocity.xyz);
            }
        )";

        // Fragment shader: Draw a circle with soft edges
        auto fragmentShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        fragmentShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.lineOffset = __LINE__;
        fragmentShaderInfo.source = R"(
            #version 450

            layout(location = 0) in vec2 fsTexCoord;
            layout(location = 1) in vec4 fsColor;
            layout(location = 2) flat in uint fsIsSelected;
            layout(location = 3) in float fsVelocityMag;
            layout(location = 0) out vec4 fragColor;

            layout(push_constant) uniform PushConstants
            {
                mat4 viewProjection;           // 64 bytes (0-63)
                vec3 cameraRight;              // 12 bytes (64-75) + 4 bytes padding
                float particleScale;           // 4 bytes  (76-79)
                vec3 cameraUp;                 // 12 bytes (80-91) + 4 bytes padding
                uint showCollisionCircles;     // 4 bytes  (92-95)
                uint selectedParticleIndex;    // 4 bytes  (96-99)
                float selectedParticleScale;   // 4 bytes  (100-103)
                uint hardEdges;                // 4 bytes  (104-107)
                uint showVelocityColors;       // 4 bytes  (108-111)
            } pushConstants;

            vec3 velocityToColor(float velocity)
            {
                // Map velocity to color: Blue (slow) -> Green (medium) -> Red (fast)
                // Typical velocity range for SPH: 0.0 to ~10.0
                float t = clamp(velocity / 10.0, 0.0, 1.0);

                if (t < 0.5) {
                    // Blue to Green
                    float local_t = t * 2.0;
                    return mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), local_t);
                } else {
                    // Green to Red
                    float local_t = (t - 0.5) * 2.0;
                    return mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), local_t);
                }
            }

            void main()
            {
                vec2 coord = fsTexCoord * 2.0 - 1.0;
                float dist = length(coord);

                if (dist > 1.0) {
                    discard;
                }

                // Highlight selected particle with bright yellow/orange color
                if (fsIsSelected != 0u) {
                    float alpha = (pushConstants.hardEdges != 0u) ? 1.0 : (1.0 - smoothstep(0.7, 1.0, dist));
                    fragColor = vec4(1.0, 0.8, 0.0, alpha);  // Bright yellow/orange
                }
                // Draw collision circle outline if enabled
                else if (pushConstants.showCollisionCircles != 0 && dist > 0.9) {
                    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
                } else {
                    // Hard or soft edges based on setting
                    float alpha;
                    if (pushConstants.hardEdges != 0u) {
                        alpha = 1.0;  // Hard edges - fully opaque
                    } else {
                        alpha = 1.0 - smoothstep(0.7, 1.0, dist);  // Soft edges - gradient
                    }

                    // Color by velocity or use original color
                    vec3 color;
                    if (pushConstants.showVelocityColors != 0u) {
                        color = velocityToColor(fsVelocityMag);
                    } else {
                        color = fsColor.rgb;
                    }

                    fragColor = vec4(color, alpha);
                }
            }
        )";

        gvk_result(spirvContext.compile(&vertexShaderInfo));
        gvk_result(spirvContext.compile(&fragmentShaderInfo));
        gvk_result(validate_shader_info(vertexShaderInfo));
        gvk_result(validate_shader_info(fragmentShaderInfo));

        auto vertexShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        vertexShaderModuleCreateInfo.codeSize = vertexShaderInfo.bytecode.size() * sizeof(uint32_t);
        vertexShaderModuleCreateInfo.pCode = !vertexShaderInfo.bytecode.empty() ? vertexShaderInfo.bytecode.data() : nullptr;
        gvk::ShaderModule vertexShaderModule;
        gvk_result(gvk::ShaderModule::create(device, &vertexShaderModuleCreateInfo, nullptr, &vertexShaderModule));

        auto fragmentShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        fragmentShaderModuleCreateInfo.codeSize = fragmentShaderInfo.bytecode.size() * sizeof(uint32_t);
        fragmentShaderModuleCreateInfo.pCode = !fragmentShaderInfo.bytecode.empty() ? fragmentShaderInfo.bytecode.data() : nullptr;
        gvk::ShaderModule fragmentShaderModule;
        gvk_result(gvk::ShaderModule::create(device, &fragmentShaderModuleCreateInfo, nullptr, &fragmentShaderModule));

        auto vertexPipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        vertexPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexPipelineShaderStageCreateInfo.module = vertexShaderModule;

        auto fragmentPipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        fragmentPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentPipelineShaderStageCreateInfo.module = fragmentShaderModule;

        std::array<VkPipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos{
            vertexPipelineShaderStageCreateInfo,
            fragmentPipelineShaderStageCreateInfo,
        };

        VkVertexInputBindingDescription vertexInputBindingDescription{ 0, sizeof(QuadVertex), VK_VERTEX_INPUT_RATE_VERTEX };
        VkVertexInputAttributeDescription vertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 };

        auto pipelineVertexInputStateCreateInfo = gvk::get_default<VkPipelineVertexInputStateCreateInfo>();
        pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
        pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescription;

        auto pipelineRasterizationStateCreateInfo = gvk::get_default<VkPipelineRasterizationStateCreateInfo>();
        pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;

        auto pipelineColorBlendAttachmentState = gvk::get_default<VkPipelineColorBlendAttachmentState>();
        pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
        pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        auto pipelineColorBlendStateCreateInfo = gvk::get_default<VkPipelineColorBlendStateCreateInfo>();
        pipelineColorBlendStateCreateInfo.attachmentCount = 1;
        pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

        // Enable depth testing for proper layering
        auto pipelineDepthStencilStateCreateInfo = gvk::get_default<VkPipelineDepthStencilStateCreateInfo>();
        pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;

        // Set multisample state to match render pass (handle both VkRenderPassCreateInfo and VkRenderPassCreateInfo2)
        auto pipelineMultisampleStateCreateInfo = gvk::get_default<VkPipelineMultisampleStateCreateInfo>();
        const auto& renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo>();
        const auto& renderPassCreateInfo2 = renderPass.get<VkRenderPassCreateInfo2>();
        if (renderPassCreateInfo.sType == gvk::get_stype<VkRenderPassCreateInfo>()) {
            // Vulkan 1.1 path
            for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
                pipelineMultisampleStateCreateInfo.rasterizationSamples = std::max(
                    pipelineMultisampleStateCreateInfo.rasterizationSamples,
                    renderPassCreateInfo.pAttachments[i].samples
                );
            }
        } else if (renderPassCreateInfo2.sType == gvk::get_stype<VkRenderPassCreateInfo2>()) {
            // Vulkan 1.2+ path
            for (uint32_t i = 0; i < renderPassCreateInfo2.attachmentCount; ++i) {
                pipelineMultisampleStateCreateInfo.rasterizationSamples = std::max(
                    pipelineMultisampleStateCreateInfo.rasterizationSamples,
                    renderPassCreateInfo2.pAttachments[i].samples
                );
            }
        }

        gvk::spirv::BindingInfo spirvBindingInfo;
        spirvBindingInfo.add_shader(vertexShaderInfo);
        spirvBindingInfo.add_shader(fragmentShaderInfo);
        // Manually set push constant range to cover the entire struct for both stages
        // The struct is 112 bytes with std140 alignment:
        // mat4(64) + vec3(12)+pad(4) + float(4) + vec3(12)+pad(4) + 5*uint/float(20) = 112 bytes
        spirvBindingInfo.pushConstantRanges.clear();
        spirvBindingInfo.pushConstantRanges.push_back({ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 112 });
        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::spirv::create_pipeline_layout(device, spirvBindingInfo, nullptr, &pipelineLayout));

        auto graphicsPipelineCreateInfo = gvk::get_default<VkGraphicsPipelineCreateInfo>();
        graphicsPipelineCreateInfo.stageCount = (uint32_t)pipelineShaderStageCreateInfos.size();
        graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();
        graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
        graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
        graphicsPipelineCreateInfo.layout = pipelineLayout;
        graphicsPipelineCreateInfo.renderPass = renderPass;
        gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, pPipeline));

    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace sph
} // namespace gvk
