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

// Push constants for binning compute shader
struct BinningPushConstants
{
    uint32_t particleCount;
    uint32_t gridDimX;
    uint32_t gridDimY;
    uint32_t gridDimZ;
    float domainMinX;
    float domainMinY;
    float domainMinZ;
    float gridCellSize;
};

// Create compute pipeline for GPU particle binning
// This shader bins particles into spatial grid cells using atomic operations
inline VkResult create_binning_pipeline(const gvk::Device& device, gvk::spirv::Context& spirvContext, gvk::Pipeline* pPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        auto computeShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        computeShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        computeShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderInfo.lineOffset = __LINE__;
        computeShaderInfo.source = R"(
            #version 450

            layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

            struct Particle
            {
                vec4 position;    // xyz = position, w = radius
                vec4 velocity;    // xyz = velocity, w = mass
                vec4 color;       // rgba color
                vec4 force;       // xyz = accumulated force, w = padding
                vec4 homePosition; // xyz = original position, w = padding
                float density;
                float pressure;
                float scale;
                float wakeRadius;
                uint gridCell;
                uint sleepCounter;
                float padding1;
                float padding2;
            };

            struct GridCell
            {
                uint particleCount;       // Total particles in this cell
                uint particleStartIndex;  // Offset into sorted particle array (future use)
                uint awakeCount;          // Awake particles in cell
                uint _pad;                // Alignment padding
            };

            layout(binding = 0) buffer ParticleBuffer
            {
                Particle particles[];
            };

            layout(binding = 1) buffer GridCellBuffer
            {
                GridCell gridCells[];
            };

            layout(push_constant) uniform PushConstants
            {
                uint particleCount;
                uint gridDimX;
                uint gridDimY;
                uint gridDimZ;
                float domainMinX;
                float domainMinY;
                float domainMinZ;
                float gridCellSize;
            } pushConstants;

            const uint SLEEP_THRESHOLD = 20u;

            void main()
            {
                uint particleIndex = gl_GlobalInvocationID.x;

                if (particleIndex >= pushConstants.particleCount) {
                    return;
                }

                // Get particle position
                vec3 pos = particles[particleIndex].position.xyz;

                // Calculate grid cell coordinates
                int cellX = int((pos.x - pushConstants.domainMinX) / pushConstants.gridCellSize);
                int cellY = int((pos.y - pushConstants.domainMinY) / pushConstants.gridCellSize);
                int cellZ = int((pos.z - pushConstants.domainMinZ) / pushConstants.gridCellSize);

                // Clamp to valid grid range
                cellX = clamp(cellX, 0, int(pushConstants.gridDimX) - 1);
                cellY = clamp(cellY, 0, int(pushConstants.gridDimY) - 1);
                cellZ = clamp(cellZ, 0, int(pushConstants.gridDimZ) - 1);

                // Calculate flat cell index
                uint cellIndex = uint(cellX) + uint(cellY) * pushConstants.gridDimX + 
                                uint(cellZ) * pushConstants.gridDimX * pushConstants.gridDimY;

                // Store cell index in particle (for neighbor search)
                particles[particleIndex].gridCell = cellIndex;

                // Atomically increment particle count for this cell
                atomicAdd(gridCells[cellIndex].particleCount, 1u);

                // Atomically increment awake count if particle is awake
                if (particles[particleIndex].sleepCounter < SLEEP_THRESHOLD) {
                    atomicAdd(gridCells[cellIndex].awakeCount, 1u);
                }
            }
        )";

        gvk_result(gvk::spirv::Context::create_shader_module(spirvContext, device, &computeShaderInfo, nullptr));

        // Setup descriptor set layout
        std::array<VkDescriptorSetLayoutBinding, 2> bindings{ };
        // Binding 0: Particle buffer (read/write)
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        // Binding 1: Grid cell buffer (write)
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        auto descriptorSetLayoutCreateInfo = gvk::get_default<VkDescriptorSetLayoutCreateInfo>();
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();

        gvk::DescriptorSetLayout descriptorSetLayout;
        gvk_result(gvk::DescriptorSetLayout::create(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));

        // Setup push constant range
        auto pushConstantRange = gvk::get_default<VkPushConstantRange>();
        pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(BinningPushConstants);

        // Create pipeline layout
        auto pipelineLayoutCreateInfo = gvk::get_default<VkPipelineLayoutCreateInfo>();
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout.get<VkDescriptorSetLayout>();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::PipelineLayout::create(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

        // Create compute pipeline
        auto pipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineShaderStageCreateInfo.module = computeShaderInfo.get<gvk::ShaderModule>();
        pipelineShaderStageCreateInfo.pName = "main";

        auto computePipelineCreateInfo = gvk::get_default<VkComputePipelineCreateInfo>();
        computePipelineCreateInfo.stage = pipelineShaderStageCreateInfo;
        computePipelineCreateInfo.layout = pipelineLayout;

        gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, pPipeline));

    } gvk_result_scope_end;
    return gvkResult;
}

// Create compute pipeline to clear grid cell buffer
// Runs before binning to reset all cell counters to zero
inline VkResult create_grid_clear_pipeline(const gvk::Device& device, gvk::spirv::Context& spirvContext, gvk::Pipeline* pPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        auto computeShaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        computeShaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
        computeShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderInfo.lineOffset = __LINE__;
        computeShaderInfo.source = R"(
            #version 450

            layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

            struct GridCell
            {
                uint particleCount;
                uint particleStartIndex;
                uint awakeCount;
                uint _pad;
            };

            layout(binding = 0) buffer GridCellBuffer
            {
                GridCell gridCells[];
            };

            layout(push_constant) uniform PushConstants
            {
                uint gridCellCount;
            } pushConstants;

            void main()
            {
                uint cellIndex = gl_GlobalInvocationID.x;

                if (cellIndex >= pushConstants.gridCellCount) {
                    return;
                }

                // Clear all counters
                gridCells[cellIndex].particleCount = 0u;
                gridCells[cellIndex].particleStartIndex = 0u;
                gridCells[cellIndex].awakeCount = 0u;
                gridCells[cellIndex]._pad = 0u;
            }
        )";

        gvk_result(gvk::spirv::Context::create_shader_module(spirvContext, device, &computeShaderInfo, nullptr));

        // Setup descriptor set layout (single buffer binding)
        VkDescriptorSetLayoutBinding binding{ };
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        auto descriptorSetLayoutCreateInfo = gvk::get_default<VkDescriptorSetLayoutCreateInfo>();
        descriptorSetLayoutCreateInfo.bindingCount = 1;
        descriptorSetLayoutCreateInfo.pBindings = &binding;

        gvk::DescriptorSetLayout descriptorSetLayout;
        gvk_result(gvk::DescriptorSetLayout::create(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));

        // Setup push constant range (single uint for grid cell count)
        auto pushConstantRange = gvk::get_default<VkPushConstantRange>();
        pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(uint32_t);

        // Create pipeline layout
        auto pipelineLayoutCreateInfo = gvk::get_default<VkPipelineLayoutCreateInfo>();
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout.get<VkDescriptorSetLayout>();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::PipelineLayout::create(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

        // Create compute pipeline
        auto pipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineShaderStageCreateInfo.module = computeShaderInfo.get<gvk::ShaderModule>();
        pipelineShaderStageCreateInfo.pName = "main";

        auto computePipelineCreateInfo = gvk::get_default<VkComputePipelineCreateInfo>();
        computePipelineCreateInfo.stage = pipelineShaderStageCreateInfo;
        computePipelineCreateInfo.layout = pipelineLayout;

        gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, pPipeline));

    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace sph
} // namespace gvk
