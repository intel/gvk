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

#pragma once

#include "utilities.hpp"

namespace gvk {
namespace sph {

// Grid line vertex (position only, color in push constants)
struct GridLineVertex
{
    glm::vec3 position;
};

// Grid cell vertex with color per-vertex (for occupancy heatmap)
struct GridCellVertex
{
    glm::vec3 position;
    glm::vec4 color;  // RGBA color based on occupancy
};

// Push constants for grid visualization
struct GridVisualizationPushConstants
{
    glm::mat4 viewProjection;
    glm::vec4 gridColor;  // RGB + alpha (for wireframe mode)
};

inline VkResult create_grid_visualization_pipeline(
    const gvk::Device& device,
    const gvk::RenderPass& renderPass,
    gvk::spirv::Context& spirvContext,
    gvk::Pipeline* pPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Vertex shader
        auto vertexShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        vertexShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.lineOffset = __LINE__;
        vertexShaderInfo.source = R"(
            #version 450

            layout(location = 0) in vec3 inPosition;

            layout(push_constant) uniform PushConstants {
                mat4 viewProjection;
                vec4 gridColor;
            } pushConstants;

            void main() {
                gl_Position = pushConstants.viewProjection * vec4(inPosition, 1.0);
            }
        )";
        gvk_result(validate_shader_info(vertexShaderInfo));

        // Fragment shader
        auto fragmentShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        fragmentShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.lineOffset = __LINE__;
        fragmentShaderInfo.source = R"(
            #version 450

            layout(push_constant) uniform PushConstants {
                mat4 viewProjection;
                vec4 gridColor;
            } pushConstants;

            layout(location = 0) out vec4 outColor;

            void main() {
                outColor = pushConstants.gridColor;
            }
        )";
        gvk_result(validate_shader_info(fragmentShaderInfo));

        gvk_result(spirvContext.compile(&vertexShaderInfo));
        gvk_result(spirvContext.compile(&fragmentShaderInfo));

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

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
            vertexPipelineShaderStageCreateInfo,
            fragmentPipelineShaderStageCreateInfo,
        };

        // Vertex input state
        auto vertexBindingDescription = gvk::get_default<VkVertexInputBindingDescription>();
        vertexBindingDescription.binding = 0;
        vertexBindingDescription.stride = sizeof(GridLineVertex);
        vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        auto vertexAttributeDescription = gvk::get_default<VkVertexInputAttributeDescription>();
        vertexAttributeDescription.location = 0;
        vertexAttributeDescription.binding = 0;
        vertexAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexAttributeDescription.offset = offsetof(GridLineVertex, position);

        auto vertexInputStateCreateInfo = gvk::get_default<VkPipelineVertexInputStateCreateInfo>();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexAttributeDescription;

        // Input assembly - LINE_LIST topology for wireframe
        auto inputAssemblyStateCreateInfo = gvk::get_default<VkPipelineInputAssemblyStateCreateInfo>();
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        // Viewport/scissor (dynamic)
        auto viewportStateCreateInfo = gvk::get_default<VkPipelineViewportStateCreateInfo>();
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;

        // Rasterization - line drawing
        auto rasterizationStateCreateInfo = gvk::get_default<VkPipelineRasterizationStateCreateInfo>();
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateCreateInfo.lineWidth = 1.0f;

        // Multisample (match render pass - check both CreateInfo versions)
        auto multisampleStateCreateInfo = gvk::get_default<VkPipelineMultisampleStateCreateInfo>();
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        const auto& renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo>();
        if (renderPassCreateInfo.sType == gvk::get_stype<VkRenderPassCreateInfo>()) {
            for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
                multisampleStateCreateInfo.rasterizationSamples = std::max(
                    multisampleStateCreateInfo.rasterizationSamples,
                    renderPassCreateInfo.pAttachments[i].samples
                );
            }
        } else {
            const auto& renderPassCreateInfo2 = renderPass.get<VkRenderPassCreateInfo2>();
            for (uint32_t i = 0; i < renderPassCreateInfo2.attachmentCount; ++i) {
                multisampleStateCreateInfo.rasterizationSamples = std::max(
                    multisampleStateCreateInfo.rasterizationSamples,
                    renderPassCreateInfo2.pAttachments[i].samples
                );
            }
        }

        // Depth/stencil - read depth, no write (draw over particles)
        auto depthStencilStateCreateInfo = gvk::get_default<VkPipelineDepthStencilStateCreateInfo>();
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        // Color blend - alpha blending for semi-transparent grid
        auto colorBlendAttachmentState = gvk::get_default<VkPipelineColorBlendAttachmentState>();
        colorBlendAttachmentState.blendEnable = VK_TRUE;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        auto colorBlendStateCreateInfo = gvk::get_default<VkPipelineColorBlendStateCreateInfo>();
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

        // Dynamic state
        std::array<VkDynamicState, 2> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        auto dynamicStateCreateInfo = gvk::get_default<VkPipelineDynamicStateCreateInfo>();
        dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        // Push constant range
        auto pushConstantRange = gvk::get_default<VkPushConstantRange>();
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(GridVisualizationPushConstants);

        // Pipeline layout (no descriptor sets, only push constants)
        auto pipelineLayoutCreateInfo = gvk::get_default<VkPipelineLayoutCreateInfo>();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::PipelineLayout::create(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

        // Graphics pipeline
        auto pipelineCreateInfo = gvk::get_default<VkGraphicsPipelineCreateInfo>();
        pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;
        pipelineCreateInfo.renderPass = renderPass;
        pipelineCreateInfo.subpass = 0;

        gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, pPipeline));

    } gvk_result_scope_end;
    return gvkResult;
}

// Create pipeline for rendering colored grid cells (occupancy heatmap)
inline VkResult create_grid_cell_pipeline(
    const gvk::Device& device,
    const gvk::RenderPass& renderPass,
    gvk::spirv::Context& spirvContext,
    gvk::Pipeline* pPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Vertex shader - per-vertex color
        auto vertexShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        vertexShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.lineOffset = __LINE__;
        vertexShaderInfo.source = R"(
            #version 450

            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec4 inColor;

            layout(push_constant) uniform PushConstants {
                mat4 viewProjection;
                vec4 gridColor;  // Unused for colored cells
            } pushConstants;

            layout(location = 0) out vec4 outColor;

            void main() {
                gl_Position = pushConstants.viewProjection * vec4(inPosition, 1.0);
                outColor = inColor;
            }
        )";
        gvk_result(validate_shader_info(vertexShaderInfo));

        // Fragment shader - passthrough color
        auto fragmentShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        fragmentShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.lineOffset = __LINE__;
        fragmentShaderInfo.source = R"(
            #version 450

            layout(location = 0) in vec4 inColor;
            layout(location = 0) out vec4 outColor;

            void main() {
                outColor = inColor;
            }
        )";
        gvk_result(validate_shader_info(fragmentShaderInfo));

        gvk_result(spirvContext.compile(&vertexShaderInfo));
        gvk_result(spirvContext.compile(&fragmentShaderInfo));

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

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
            vertexPipelineShaderStageCreateInfo,
            fragmentPipelineShaderStageCreateInfo,
        };

        // Vertex input state - position + color
        std::array<VkVertexInputBindingDescription, 1> vertexBindingDescriptions{{
            { 0, sizeof(GridCellVertex), VK_VERTEX_INPUT_RATE_VERTEX }
        }};

        std::array<VkVertexInputAttributeDescription, 2> vertexAttributeDescriptions{{
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GridCellVertex, position) },
            { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GridCellVertex, color) }
        }};

        auto vertexInputStateCreateInfo = gvk::get_default<VkPipelineVertexInputStateCreateInfo>();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = (uint32_t)vertexBindingDescriptions.size();
        vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexBindingDescriptions.data();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexAttributeDescriptions.size();
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

        // Input assembly - TRIANGLE_LIST for filled quads
        auto inputAssemblyStateCreateInfo = gvk::get_default<VkPipelineInputAssemblyStateCreateInfo>();
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        // Viewport/scissor (dynamic)
        auto viewportStateCreateInfo = gvk::get_default<VkPipelineViewportStateCreateInfo>();
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;

        // Rasterization
        auto rasterizationStateCreateInfo = gvk::get_default<VkPipelineRasterizationStateCreateInfo>();
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateCreateInfo.lineWidth = 1.0f;

        // Multisample (match render pass)
        auto multisampleStateCreateInfo = gvk::get_default<VkPipelineMultisampleStateCreateInfo>();
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        const auto& renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo>();
        if (renderPassCreateInfo.sType == gvk::get_stype<VkRenderPassCreateInfo>()) {
            for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
                multisampleStateCreateInfo.rasterizationSamples = std::max(
                    multisampleStateCreateInfo.rasterizationSamples,
                    renderPassCreateInfo.pAttachments[i].samples
                );
            }
        } else {
            const auto& renderPassCreateInfo2 = renderPass.get<VkRenderPassCreateInfo2>();
            for (uint32_t i = 0; i < renderPassCreateInfo2.attachmentCount; ++i) {
                multisampleStateCreateInfo.rasterizationSamples = std::max(
                    multisampleStateCreateInfo.rasterizationSamples,
                    renderPassCreateInfo2.pAttachments[i].samples
                );
            }
        }

        // Depth/stencil - read depth, no write (draw behind particles)
        auto depthStencilStateCreateInfo = gvk::get_default<VkPipelineDepthStencilStateCreateInfo>();
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        // Color blend - alpha blending
        auto colorBlendAttachmentState = gvk::get_default<VkPipelineColorBlendAttachmentState>();
        colorBlendAttachmentState.blendEnable = VK_TRUE;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        auto colorBlendStateCreateInfo = gvk::get_default<VkPipelineColorBlendStateCreateInfo>();
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

        // Dynamic state
        std::array<VkDynamicState, 2> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        auto dynamicStateCreateInfo = gvk::get_default<VkPipelineDynamicStateCreateInfo>();
        dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        // Push constant range (same as wireframe pipeline)
        auto pushConstantRange = gvk::get_default<VkPushConstantRange>();
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(GridVisualizationPushConstants);

        // Pipeline layout
        auto pipelineLayoutCreateInfo = gvk::get_default<VkPipelineLayoutCreateInfo>();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::PipelineLayout::create(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

        // Graphics pipeline
        auto pipelineCreateInfo = gvk::get_default<VkGraphicsPipelineCreateInfo>();
        pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;
        pipelineCreateInfo.renderPass = renderPass;
        pipelineCreateInfo.subpass = 0;

        gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, pPipeline));

    } gvk_result_scope_end;
    return gvkResult;
}

// Generate grid line vertices for visualization
// Creates a wireframe box for each grid cell in a plane (Z=0 for 2D visualization)
inline std::vector<GridLineVertex> generate_grid_lines()
{
    std::vector<GridLineVertex> vertices;
    vertices.reserve(kGridDimensionX * kGridDimensionY * 8 * 2);  // Rough estimate

    // Generate grid lines in XY plane (Z = 0) for 2D visualization
    const int zSlice = static_cast<int>(kGridDimensionZ / 2);  // Middle slice
    const float z = kDomainMinZ + zSlice * kGridCellSize;

    // Vertical lines (X-aligned)
    for (uint32_t ix = 0; ix <= kGridDimensionX; ++ix) {
        float x = kDomainMinX + ix * kGridCellSize;
        vertices.push_back({ glm::vec3(x, kDomainMinY, z) });
        vertices.push_back({ glm::vec3(x, kDomainMaxY, z) });
    }

    // Horizontal lines (Y-aligned)
    for (uint32_t iy = 0; iy <= kGridDimensionY; ++iy) {
        float y = kDomainMinY + iy * kGridCellSize;
        vertices.push_back({ glm::vec3(kDomainMinX, y, z) });
        vertices.push_back({ glm::vec3(kDomainMaxX, y, z) });
    }

    return vertices;
}

// Helper: Get color for occupancy level (heatmap)
inline glm::vec4 get_occupancy_color(uint32_t particleCount)
{
    if (particleCount == 0) {
        return glm::vec4(0.2f, 0.2f, 0.2f, 0.1f);  // Gray, very transparent (empty)
    } else if (particleCount <= 10) {
        return glm::vec4(0.2f, 0.5f, 0.8f, 0.3f);  // Blue/Cyan (sparse)
    } else if (particleCount <= 50) {
        return glm::vec4(0.9f, 0.7f, 0.2f, 0.5f);  // Yellow/Orange (medium)
    } else {
        return glm::vec4(0.9f, 0.2f, 0.2f, 0.7f);  // Red (dense)
    }
}

// Generate colored cell quads showing particle occupancy
// Takes particle positions and generates filled quads for occupied cells
// NOTE: This is CPU-side and expensive! Should be moved to GPU (Milestone 2)
inline std::vector<GridCellVertex> generate_grid_cell_occupancy(
    const Particle* particles,
    uint32_t particleCount)
{
    std::vector<GridCellVertex> vertices;
    vertices.reserve(particleCount * 2);  // Conservative estimate (most cells will be empty)

    // Count particles per cell (static to avoid per-frame allocation)
    static std::vector<uint32_t> cellCounts;
    if (cellCounts.size() != kGridCellCount) {
        cellCounts.resize(kGridCellCount);
    }
    std::fill(cellCounts.begin(), cellCounts.end(), 0);

    for (uint32_t i = 0; i < particleCount; ++i) {
        int cellX, cellY, cellZ;
        world_to_grid_cell(
            particles[i].position.x,
            particles[i].position.y,
            particles[i].position.z,
            &cellX, &cellY, &cellZ
        );
        uint32_t cellIndex = grid_cell_index(cellX, cellY, cellZ);
        cellCounts[cellIndex]++;
    }

    // Generate quads for occupied cells (XY plane, Z = middle slice)
    const int zSlice = static_cast<int>(kGridDimensionZ / 2);
    const float z = kDomainMinZ + zSlice * kGridCellSize;

    for (uint32_t ix = 0; ix < kGridDimensionX; ++ix) {
        for (uint32_t iy = 0; iy < kGridDimensionY; ++iy) {
            uint32_t cellIndex = grid_cell_index(ix, iy, zSlice);
            uint32_t count = cellCounts[cellIndex];

            // Skip empty cells (or draw them very faint)
            if (count == 0) continue;

            glm::vec4 color = get_occupancy_color(count);

            float x0 = kDomainMinX + ix * kGridCellSize;
            float y0 = kDomainMinY + iy * kGridCellSize;
            float x1 = x0 + kGridCellSize;
            float y1 = y0 + kGridCellSize;

            // Two triangles forming a quad
            // Triangle 1
            vertices.push_back({ glm::vec3(x0, y0, z), color });
            vertices.push_back({ glm::vec3(x1, y0, z), color });
            vertices.push_back({ glm::vec3(x1, y1, z), color });

            // Triangle 2
            vertices.push_back({ glm::vec3(x0, y0, z), color });
            vertices.push_back({ glm::vec3(x1, y1, z), color });
            vertices.push_back({ glm::vec3(x0, y1, z), color });
        }
    }

    return vertices;
}

// GPU-accelerated version: Reads from GPU-computed GridCell buffer (Milestone 2)
// Much faster than CPU binning (100x+ speedup)
inline std::vector<GridCellVertex> generate_grid_cell_occupancy_from_gpu(const GridCell* gridCells)
{
    std::vector<GridCellVertex> vertices;
    vertices.reserve(kGridCellCount * 6);  // Max: 6 vertices per cell

    // Iterate over all grid cells and generate quads for occupied ones
    for (uint32_t cellIndex = 0; cellIndex < kGridCellCount; ++cellIndex) {
        uint32_t particleCount = gridCells[cellIndex].particleCount;

        if (particleCount == 0) {
            continue;  // Skip empty cells
        }

        // Get cell coordinates from flat index
        uint32_t x = cellIndex % kGridDimensionX;
        uint32_t y = (cellIndex / kGridDimensionX) % kGridDimensionY;
        uint32_t z = cellIndex / (kGridDimensionX * kGridDimensionY);

        // Convert grid coordinates to world position (cell corner)
        float x0 = kDomainMinX + x * kGridCellSize;
        float y0 = kDomainMinY + y * kGridCellSize;
        float z0 = kDomainMinZ + z * kGridCellSize;

        // Get color based on occupancy
        glm::vec4 color = get_occupancy_color(particleCount);

        // Z position at center for flat visualization (could use z0 for 3D)
        float z = z0 + kGridCellSize * 0.5f;

        // Calculate quad corners
        float x1 = x0 + kGridCellSize;
        float y1 = y0 + kGridCellSize;

        // Two triangles forming a quad
        // Triangle 1
        vertices.push_back({ glm::vec3(x0, y0, z), color });
        vertices.push_back({ glm::vec3(x1, y0, z), color });
        vertices.push_back({ glm::vec3(x1, y1, z), color });

        // Triangle 2
        vertices.push_back({ glm::vec3(x0, y0, z), color });
        vertices.push_back({ glm::vec3(x1, y1, z), color });
        vertices.push_back({ glm::vec3(x0, y1, z), color });
    }

    return vertices;
}

} // namespace sph
} // namespace gvk
