
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
#include "gvk-gui.hpp"

#include <array>
#include <iostream>

namespace gvk {
namespace sph {

// Quad vertex for instanced rendering
struct QuadVertex
{
    glm::vec2 position;
};

// Create compute pipeline for particle simulation
VkResult create_compute_pipeline(
    const gvk::Device& device,
    gvk::spirv::Context& spirvContext,
    gvk::Pipeline* pPipeline
)
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
                float density;
                float pressure;
                uint gridCell;
                float scale;      // Per-particle visual scale
            };

            layout(binding = 0) buffer ParticleBuffer
            {
                Particle particles[];
            };

            layout(push_constant) uniform PushConstants
            {
                uint particleCount;
                float deltaTime;
                float smoothingRadius;
                float particleMass;
                float gasConstant;
                float restDensity;
                float viscosity;
                float gravityStrength;
                float gravityRadius;
                float domainMinX;
                float domainMaxX;
                float domainMinY;
                float domainMaxY;
                float domainMinZ;
                float domainMaxZ;
                uint gridDimX;
                uint gridDimY;
                uint gridDimZ;
            } pushConstants;

            // Spatial hash grid functions (optimized for 2D)
            ivec3 getGridCell(vec3 position)
            {
                vec3 domainMin = vec3(pushConstants.domainMinX, pushConstants.domainMinY, pushConstants.domainMinZ);
                vec3 domainMax = vec3(pushConstants.domainMaxX, pushConstants.domainMaxY, pushConstants.domainMaxZ);
                vec3 domainSize = domainMax - domainMin;
                vec3 gridSize = vec3(pushConstants.gridDimX, pushConstants.gridDimY, pushConstants.gridDimZ);

                vec3 normalizedPos = (position - domainMin) / domainSize;
                ivec3 cell = ivec3(floor(normalizedPos * gridSize));

                // Clamp to grid bounds
                cell = clamp(cell, ivec3(0), ivec3(gridSize) - ivec3(1));
                return cell;
            }

            uint getCellHash(ivec3 cell)
            {
                // Simple 2D hash (Z is always 0)
                return uint(cell.y) * pushConstants.gridDimX + uint(cell.x);
            }

            // Optimized 2D neighbor check
            bool areNeighboringCells(ivec3 cell1, ivec3 cell2)
            {
                // For 2D, only check X and Y (Z is always 0)
                ivec2 diff = abs(cell1.xy - cell2.xy);
                return diff.x <= 1 && diff.y <= 1;
            }

            // SPH Poly6 kernel for density calculation
            float poly6Kernel(float r, float h)
            {
                if (r >= 0.0 && r <= h) {
                    float x = (h * h - r * r);
                    return 315.0 / (64.0 * 3.14159265 * pow(h, 9.0)) * x * x * x;
                }
                return 0.0;
            }

            // SPH Spiky kernel gradient for pressure force
            vec3 spikyKernelGradient(vec3 r, float h)
            {
                float rLen = length(r);
                if (rLen > 0.0 && rLen <= h) {
                    float x = (h - rLen);
                    return -45.0 / (3.14159265 * pow(h, 6.0)) * x * x * (r / rLen);
                }
                return vec3(0.0);
            }

            // SPH Viscosity kernel Laplacian
            float viscosityKernelLaplacian(float r, float h)
            {
                if (r >= 0.0 && r <= h) {
                    return 45.0 / (3.14159265 * pow(h, 6.0)) * (h - r);
                }
                return 0.0;
            }

            void main()
            {
                uint index = gl_GlobalInvocationID.x;
                if (index >= pushConstants.particleCount) {
                    return;
                }

                Particle p = particles[index];

                // Calculate and store grid cell for this particle
                ivec3 myCell = getGridCell(p.position.xyz);
                p.gridCell = getCellHash(myCell);

                // Effective radius considering scale
                float effectiveRadius = p.position.w * p.scale;

                // Reset force and density
                p.force.xyz = vec3(0.0);
                p.density = 0.0;

                // Calculate density and pressure using spatial grid acceleration
                for (uint j = 0; j < pushConstants.particleCount; ++j) {
                    Particle pj = particles[j];

                    // Quick distance check before grid lookup
                    vec3 diff = p.position.xyz - pj.position.xyz;
                    float distSq = dot(diff, diff);
                    float maxDistSq = pushConstants.smoothingRadius * pushConstants.smoothingRadius;

                    // Skip if too far away
                    if (distSq > maxDistSq) {
                        continue;
                    }

                    float dist = sqrt(distSq);

                    // Use smoothing radius for density calculation
                    if (dist < pushConstants.smoothingRadius) {
                        p.density += pushConstants.particleMass * poly6Kernel(dist, pushConstants.smoothingRadius);
                    }
                }

                // Ensure minimum density to avoid division by zero
                p.density = max(p.density, 0.01);

                // Calculate pressure using ideal gas law
                p.pressure = pushConstants.gasConstant * max(p.density - pushConstants.restDensity, 0.0);

                // Calculate forces
                vec3 pressureForce = vec3(0.0);
                vec3 viscosityForce = vec3(0.0);
                vec3 collisionForce = vec3(0.0);

                // Pre-calculate this particle's effective radius
                float pEffectiveRadius = p.position.w * p.scale;

                for (uint j = 0; j < pushConstants.particleCount; ++j) {
                    if (j == index) continue;

                    Particle pj = particles[j];
                    vec3 diff = p.position.xyz - pj.position.xyz;

                    // Quick distance check using squared distance
                    float distSq = dot(diff, diff);

                    // Calculate max interaction distance (larger of smoothing radius and collision radius)
                    float pjEffectiveRadius = pj.position.w * pj.scale;
                    float maxInteractionDist = max(pushConstants.smoothingRadius, pEffectiveRadius + pjEffectiveRadius);
                    float maxDistSq = maxInteractionDist * maxInteractionDist;

                    // Skip if particles are too far to interact
                    if (distSq > maxDistSq) {
                        continue;
                    }

                    float dist = sqrt(distSq);

                    // Ensure minimum distance to avoid singularities
                    if (dist < 0.001) {
                        // Particles are too close, apply emergency separation
                        diff = vec3(0.001, 0.0, 0.0);
                        dist = 0.001;
                    }

                    vec3 normal = normalize(diff);

                    // Collision detection with scaled radii (should match visual size)
                    float minDist = pEffectiveRadius + pjEffectiveRadius;

                    // Hard sphere collision - very strong repulsion to prevent overlap
                    if (dist < minDist) {
                        float overlap = minDist - dist;

                        // Very strong collision force to prevent overlap
                        // Force increases quadratically with overlap
                        float collisionStrength = 2000.0;
                        float overlapFactor = overlap / minDist; // Normalized overlap
                        collisionForce += normal * overlap * collisionStrength * (1.0 + overlapFactor * 5.0);

                        // Strong velocity damping on collision
                        vec3 relativeVel = p.velocity.xyz - pj.velocity.xyz;
                        float velAlongNormal = dot(relativeVel, normal);
                        if (velAlongNormal < 0.0) {
                            // Strong damping to prevent particles from pushing through each other
                            collisionForce -= normal * velAlongNormal * 100.0;
                        }
                    }

                    // SPH forces only if within smoothing radius and NOT in hard collision
                    if (dist < pushConstants.smoothingRadius && dist >= minDist * 0.95) {
                        vec3 gradW = spikyKernelGradient(diff, pushConstants.smoothingRadius);
                        float pjDensity = max(particles[j].density, 0.01);

                        // Symmetric pressure force
                        float pressureTerm = (p.pressure / (p.density * p.density)) + (pj.pressure / (pjDensity * pjDensity));
                        pressureForce -= pushConstants.particleMass * pressureTerm * gradW;

                        // Viscosity force
                        float lapW = viscosityKernelLaplacian(dist, pushConstants.smoothingRadius);
                        viscosityForce += pushConstants.viscosity * pushConstants.particleMass * 
                                         (pj.velocity.xyz - p.velocity.xyz) / pjDensity * lapW;
                    }
                }

                // Combine all forces
                p.force.xyz = pressureForce + viscosityForce + collisionForce;

                // Apply gravity (attractive force toward center)
                vec3 toCenter = -p.position.xyz;
                float distToCenter = length(toCenter);
                if (distToCenter > pushConstants.gravityRadius) {
                    vec3 gravityDir = normalize(toCenter);
                    float gravityMag = pushConstants.gravityStrength * (distToCenter - pushConstants.gravityRadius);
                    p.force.xyz += gravityDir * gravityMag;
                }

                // Update velocity (F = ma, a = F/m)
                vec3 acceleration = p.force.xyz / pushConstants.particleMass;

                // Lock Z-axis - keep everything in 2D plane
                acceleration.z = 0.0;

                // Limit acceleration to prevent extreme instability
                float maxAccel = 500.0;  // Increased from 100 to allow strong collision response
                float accelMag = length(acceleration);
                if (accelMag > maxAccel) {
                    acceleration = (acceleration / accelMag) * maxAccel;
                }

                p.velocity.xyz += acceleration * pushConstants.deltaTime;

                // Lock Z velocity
                p.velocity.z = 0.0;

                // Light velocity damping for stability
                p.velocity.xyz *= 0.995;  // Reduced damping from 0.98 to allow faster separation

                // Limit maximum velocity to prevent tunneling
                float maxVel = 30.0;  // Increased from 10 to allow fast collision response
                float velMag = length(p.velocity.xyz);
                if (velMag > maxVel) {
                    p.velocity.xyz = (p.velocity.xyz / velMag) * maxVel;
                }

                // Update position
                p.position.xyz += p.velocity.xyz * pushConstants.deltaTime;

                // Lock Z position to 0
                p.position.z = 0.0;

                // Boundary conditions: bounce off walls with damping (2D only, X and Y)
                float damping = 0.5;
                if (p.position.x < pushConstants.domainMinX) {
                    p.position.x = pushConstants.domainMinX;
                    p.velocity.x = abs(p.velocity.x) * damping;
                }
                if (p.position.x > pushConstants.domainMaxX) {
                    p.position.x = pushConstants.domainMaxX;
                    p.velocity.x = -abs(p.velocity.x) * damping;
                }
                if (p.position.y < pushConstants.domainMinY) {
                    p.position.y = pushConstants.domainMinY;
                    p.velocity.y = abs(p.velocity.y) * damping;
                }
                if (p.position.y > pushConstants.domainMaxY) {
                    p.position.y = pushConstants.domainMaxY;
                    p.velocity.y = -abs(p.velocity.y) * damping;
                }

                // Color based on density (blue = low, red = high)
                // Normalize against rest density to show variation
                float densityRatio = p.density / pushConstants.restDensity;
                float densityNormalized = clamp((densityRatio - 0.5) / 1.5, 0.0, 1.0);
                p.color = vec4(densityNormalized, 0.3 + densityNormalized * 0.3, 1.0 - densityNormalized, 1.0);

                particles[index] = p;
            }
        )";

        gvk_result(spirvContext.compile(&computeShaderInfo));
        gvk_result(validate_shader_info(computeShaderInfo));

        auto shaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        shaderModuleCreateInfo.codeSize = computeShaderInfo.bytecode.size() * sizeof(uint32_t);
        shaderModuleCreateInfo.pCode = !computeShaderInfo.bytecode.empty() ? computeShaderInfo.bytecode.data() : nullptr;
        gvk::ShaderModule shaderModule;
        gvk_result(gvk::ShaderModule::create(device, &shaderModuleCreateInfo, nullptr, &shaderModule));

        gvk::spirv::BindingInfo spirvBindingInfo;
        spirvBindingInfo.add_shader(computeShaderInfo);
        // Manually set push constant range to match the full struct size in the shader
        spirvBindingInfo.pushConstantRanges.clear();
        spirvBindingInfo.pushConstantRanges.push_back({ VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants) });
        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::spirv::create_pipeline_layout(device, spirvBindingInfo, nullptr, &pipelineLayout));

        auto pipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineShaderStageCreateInfo.module = shaderModule;

        auto computePipelineCreateInfo = gvk::get_default<VkComputePipelineCreateInfo>();
        computePipelineCreateInfo.stage = pipelineShaderStageCreateInfo;
        computePipelineCreateInfo.layout = pipelineLayout;
        gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, pPipeline));

    } gvk_result_scope_end;
    return gvkResult;
}

// Create graphics pipeline for particle rendering
VkResult create_graphics_pipeline(
    const gvk::Device& device,
    const gvk::RenderPass& renderPass,
    gvk::spirv::Context& spirvContext,
    gvk::Pipeline* pPipeline
)
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
                float density;
                float pressure;
                uint gridCell;
                float scale;      // Per-particle visual scale
            };

            layout(binding = 0) readonly buffer ParticleBuffer
            {
                Particle particles[];
            };

            layout(push_constant) uniform PushConstants
            {
                mat4 viewProjection;
                vec3 cameraRight;
                float particleScale;
                vec3 cameraUp;
                uint showCollisionCircles;
                uint selectedParticleIndex;
                float selectedParticleScale;
            } pushConstants;

            layout(location = 0) in vec2 quadVertex;

            layout(location = 0) out vec2 fsTexCoord;
            layout(location = 1) out vec4 fsColor;
            layout(location = 2) flat out uint fsIsSelected;

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
            layout(location = 0) out vec4 fragColor;

            layout(push_constant) uniform PushConstants
            {
                mat4 viewProjection;
                vec3 cameraRight;
                float particleScale;
                vec3 cameraUp;
                uint showCollisionCircles;
                uint selectedParticleIndex;
                float selectedParticleScale;
            } pushConstants;

            void main()
            {
                vec2 coord = fsTexCoord * 2.0 - 1.0;
                float dist = length(coord);

                if (dist > 1.0) {
                    discard;
                }

                // Highlight selected particle with bright yellow/orange color
                if (fsIsSelected != 0u) {
                    float alpha = 1.0 - smoothstep(0.7, 1.0, dist);
                    fragColor = vec4(1.0, 0.8, 0.0, alpha);  // Bright yellow/orange
                }
                // Draw collision circle outline if enabled
                else if (pushConstants.showCollisionCircles != 0 && dist > 0.9) {
                    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
                } else {
                    // Soft circle gradient
                    float alpha = 1.0 - smoothstep(0.7, 1.0, dist);
                    fragColor = vec4(fsColor.rgb, alpha);
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

        gvk::spirv::BindingInfo spirvBindingInfo;
        spirvBindingInfo.add_shader(vertexShaderInfo);
        spirvBindingInfo.add_shader(fragmentShaderInfo);
        // Manually set push constant range to cover the entire struct for both stages
        // The struct is 104 bytes: mat4(64) + vec3(12) + float(4) + vec3(12) + uint(4) + uint(4) + float(4)
        spirvBindingInfo.pushConstantRanges.clear();
        spirvBindingInfo.pushConstantRanges.push_back({ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 104 });
        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::spirv::create_pipeline_layout(device, spirvBindingInfo, nullptr, &pipelineLayout));

        auto graphicsPipelineCreateInfo = gvk::get_default<VkGraphicsPipelineCreateInfo>();
        graphicsPipelineCreateInfo.stageCount = (uint32_t)pipelineShaderStageCreateInfos.size();
        graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();
        graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
        graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
        graphicsPipelineCreateInfo.layout = pipelineLayout;
        graphicsPipelineCreateInfo.renderPass = renderPass;
        gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, pPipeline));

    } gvk_result_scope_end;
    return gvkResult;
}

// TODO: Stub functions will go here

} // namespace sph
} // namespace gvk

int main(int, const char*[])
{
    using namespace gvk::sph;

    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        SphContext context;
        gvk_result(SphContext::create("Intel(R) GPA Utilities for Vulkan* - N-Body SPH Simulation", &context));
        const auto& gvkDevice = context.get<gvk::Devices>()[0];
        const auto& gvkQueue = gvk::get_queue_family(gvkDevice, 0).queues[0];

        gvk::system::Surface systemSurface;
        gvk_result(create_system_surface(context, &systemSurface));

        gvk::wsi::Context wsiContext;
        gvk_result(create_wsi_context(context, systemSurface, &wsiContext));

        gvk::gui::Renderer guiRenderer;
        gvk_result(gvk::gui::Renderer::create(
            context.get<gvk::Devices>()[0],
            gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0],
            context.get<gvk::CommandBuffers>()[0],
            wsiContext.get<gvk::RenderPass>(),
            nullptr,
            &guiRenderer
        ));

        // Create SPIRV context for shader compilation
        gvk::spirv::Context spirvContext;
        gvk_result(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext));

        // Initialize simulation parameters
        SimulationParameters simParams;

        // Initialize particles with initial count
        std::vector<Particle> particles;
        initialize_particles(particles, simParams.particleCount, simParams.initMode);

        // Create particle buffer with MAXIMUM size (to allow runtime particle count changes)
        // Allocate for max particles, but only initialize the initial count
        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = kMaxParticleCount * sizeof(Particle);  // Allocate max size
        bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VmaAllocationCreateInfo vmaAllocationCreateInfo{ };
        vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        gvk::Buffer particleBuffer;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &vmaAllocationCreateInfo, &particleBuffer));

        // Upload initial particle data
        VmaAllocationInfo particleBufferAllocationInfo{ };
        vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), particleBuffer.get<VmaAllocation>(), &particleBufferAllocationInfo);
        memcpy(particleBufferAllocationInfo.pMappedData, particles.data(), particles.size() * sizeof(Particle));

        // Create quad vertex buffer for instanced rendering (6 vertices forming 2 triangles)
        std::array<QuadVertex, 6> quadVertices {{
            {{ -1.0f, -1.0f }},  // Bottom-left
            {{  1.0f, -1.0f }},  // Bottom-right
            {{  1.0f,  1.0f }},  // Top-right
            {{ -1.0f, -1.0f }},  // Bottom-left (second triangle)
            {{  1.0f,  1.0f }},  // Top-right
            {{ -1.0f,  1.0f }}   // Top-left
        }};

        bufferCreateInfo.size = quadVertices.size() * sizeof(QuadVertex);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        gvk::Buffer quadVertexBuffer;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &vmaAllocationCreateInfo, &quadVertexBuffer));

        VmaAllocationInfo quadBufferAllocationInfo{ };
        vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), quadVertexBuffer.get<VmaAllocation>(), &quadBufferAllocationInfo);
        memcpy(quadBufferAllocationInfo.pMappedData, quadVertices.data(), quadVertices.size() * sizeof(QuadVertex));

        // Create compute pipeline
        gvk::Pipeline computePipeline;
        gvk_result(create_compute_pipeline(gvkDevice, spirvContext, &computePipeline));

        // Create graphics pipeline
        gvk::Pipeline graphicsPipeline;
        gvk_result(create_graphics_pipeline(gvkDevice, wsiContext.get<gvk::RenderPass>(), spirvContext, &graphicsPipeline));

        // Allocate descriptor sets for compute pipeline
        gvk::DescriptorPool computeDescriptorPool;
        std::vector<gvk::DescriptorSet> computeDescriptorSets;
        {
            auto& pipelineLayout = computePipeline.get<gvk::PipelineLayout>();
            auto& descriptorSetLayouts = pipelineLayout.get<gvk::DescriptorSetLayouts>();
            if (!descriptorSetLayouts.empty()) {
                std::vector<VkDescriptorPoolSize> poolSizes;
                std::vector<VkDescriptorSetLayout> setLayouts;
                for (const auto& descriptorSetLayout : descriptorSetLayouts) {
                    setLayouts.push_back(descriptorSetLayout);
                    const auto& createInfo = descriptorSetLayout.get<VkDescriptorSetLayoutCreateInfo>();
                    for (uint32_t i = 0; i < createInfo.bindingCount; ++i) {
                        poolSizes.push_back({ createInfo.pBindings[i].descriptorType, createInfo.pBindings[i].descriptorCount });
                    }
                }

                auto poolCreateInfo = gvk::get_default<VkDescriptorPoolCreateInfo>();
                poolCreateInfo.maxSets = (uint32_t)setLayouts.size();
                poolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
                poolCreateInfo.pPoolSizes = poolSizes.data();
                gvk_result(gvk::DescriptorPool::create(gvkDevice, &poolCreateInfo, nullptr, &computeDescriptorPool));

                auto allocInfo = gvk::get_default<VkDescriptorSetAllocateInfo>();
                allocInfo.descriptorPool = computeDescriptorPool;
                allocInfo.descriptorSetCount = (uint32_t)setLayouts.size();
                allocInfo.pSetLayouts = setLayouts.data();
                computeDescriptorSets.resize(setLayouts.size());
                gvk_result(gvk::DescriptorSet::allocate(gvkDevice, &allocInfo, computeDescriptorSets.data()));
            }
        }

        // Bind particle buffer to compute descriptor set
        if (!computeDescriptorSets.empty()) {
            auto descriptorBufferInfo = gvk::get_default<VkDescriptorBufferInfo>();
            descriptorBufferInfo.buffer = particleBuffer;
            descriptorBufferInfo.range = VK_WHOLE_SIZE;

            auto writeDescriptorSet = gvk::get_default<VkWriteDescriptorSet>();
            writeDescriptorSet.dstSet = computeDescriptorSets[0];
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
            gvkDevice.UpdateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
        }

        // Allocate descriptor sets for graphics pipeline
        gvk::DescriptorPool graphicsDescriptorPool;
        std::vector<gvk::DescriptorSet> graphicsDescriptorSets;
        {
            auto& pipelineLayout = graphicsPipeline.get<gvk::PipelineLayout>();
            auto& descriptorSetLayouts = pipelineLayout.get<gvk::DescriptorSetLayouts>();
            if (!descriptorSetLayouts.empty()) {
                std::vector<VkDescriptorPoolSize> poolSizes;
                std::vector<VkDescriptorSetLayout> setLayouts;
                for (const auto& descriptorSetLayout : descriptorSetLayouts) {
                    setLayouts.push_back(descriptorSetLayout);
                    const auto& createInfo = descriptorSetLayout.get<VkDescriptorSetLayoutCreateInfo>();
                    for (uint32_t i = 0; i < createInfo.bindingCount; ++i) {
                        poolSizes.push_back({ createInfo.pBindings[i].descriptorType, createInfo.pBindings[i].descriptorCount });
                    }
                }

                auto poolCreateInfo = gvk::get_default<VkDescriptorPoolCreateInfo>();
                poolCreateInfo.maxSets = (uint32_t)setLayouts.size();
                poolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
                poolCreateInfo.pPoolSizes = poolSizes.data();
                gvk_result(gvk::DescriptorPool::create(gvkDevice, &poolCreateInfo, nullptr, &graphicsDescriptorPool));

                auto allocInfo = gvk::get_default<VkDescriptorSetAllocateInfo>();
                allocInfo.descriptorPool = graphicsDescriptorPool;
                allocInfo.descriptorSetCount = (uint32_t)setLayouts.size();
                allocInfo.pSetLayouts = setLayouts.data();
                graphicsDescriptorSets.resize(setLayouts.size());
                gvk_result(gvk::DescriptorSet::allocate(gvkDevice, &allocInfo, graphicsDescriptorSets.data()));
            }
        }

        // Bind particle buffer to graphics descriptor set
        if (!graphicsDescriptorSets.empty()) {
            auto descriptorBufferInfo = gvk::get_default<VkDescriptorBufferInfo>();
            descriptorBufferInfo.buffer = particleBuffer;
            descriptorBufferInfo.range = VK_WHOLE_SIZE;

            auto writeDescriptorSet = gvk::get_default<VkWriteDescriptorSet>();
            writeDescriptorSet.dstSet = graphicsDescriptorSets[0];
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
            gvkDevice.UpdateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
        }

        bool showGui = true;

        gvk::math::Camera camera;
        camera.transform.translation = { 0, 0, -16 };
        gvk::math::FreeCameraController cameraController;
        cameraController.set_camera(&camera);

        // Particle selection state
        int32_t selectedParticleIndex = -1;
        float selectedParticleScale = 1.0f;

        gvk::system::Clock clock;
        while (
            !(systemSurface.get<gvk::system::Input>().keyboard.down(gvk::system::Key::Escape)) &&
            !(systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::CloseRequested)) {
            gvk::system::Surface::update();
            clock.update();

            // Update the gvk::math::FreeCameraController...
            auto deltaTime = clock.elapsed<gvk::system::Seconds<float>>();
            const auto& input = systemSurface.get<gvk::system::Input>();

            // Toggle the gui display with [`]
            if (input.keyboard.pressed(gvk::system::Key::OEM_Tilde)) {
                showGui = !showGui;
            }

            // When ImGui wants mouse/keyboard input, input should be ignored by the scene
            if (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard) {
                // Handle particle selection with left mouse click
                if (input.mouse.buttons.pressed(gvk::system::Mouse::Button::Left)) {
                    auto extent = wsiContext.get<gvk::SwapchainKHR>().get<VkSwapchainCreateInfoKHR>().imageExtent;
                    glm::vec2 mousePos = glm::vec2(input.mouse.position.current[0], input.mouse.position.current[1]);

                    // Find closest particle to mouse cursor by projecting particles to screen space
                    float closestScreenDist = FLT_MAX;
                    selectedParticleIndex = -1;

                    VmaAllocationInfo particleBufferAllocInfo{ };
                    vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), particleBuffer.get<VmaAllocation>(), &particleBufferAllocInfo);
                    Particle* pParticles = (Particle*)particleBufferAllocInfo.pMappedData;

                    glm::mat4 viewProj = camera.projection() * camera.view();

                    for (uint32_t i = 0; i < simParams.particleCount; ++i) {
                        glm::vec3 particlePos = glm::vec3(pParticles[i].position);

                        // Project particle to clip space
                        glm::vec4 clipPos = viewProj * glm::vec4(particlePos, 1.0f);

                        // Check if particle is in front of camera
                        if (clipPos.w > 0.0f) {
                            // Convert to NDC
                            glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

                            // Check if particle is within view frustum
                            if (ndcPos.x >= -1.0f && ndcPos.x <= 1.0f &&
                                ndcPos.y >= -1.0f && ndcPos.y <= 1.0f &&
                                ndcPos.z >= 0.0f && ndcPos.z <= 1.0f) {

                                // Convert NDC to screen space
                                // Vulkan NDC: X right [-1,1], Y down [-1,1], Z into screen [0,1]
                                glm::vec2 screenPos = glm::vec2(
                                    (ndcPos.x * 0.5f + 0.5f) * extent.width,
                                    (ndcPos.y * 0.5f + 0.5f) * extent.height  // Vulkan Y is already down
                                );

                                // Calculate distance to mouse cursor
                                float screenDist = glm::distance(screenPos, mousePos);

                                // Use particle's SCALED radius for selection
                                float particleRadius = pParticles[i].position.w;
                                float particleScale = pParticles[i].scale;
                                float effectiveRadius = particleRadius * particleScale;

                                // Project particle's effective radius to screen space
                                glm::vec4 radiusPoint = viewProj * glm::vec4(particlePos + glm::vec3(effectiveRadius, 0, 0), 1.0f);
                                if (radiusPoint.w > 0.0f) {
                                    glm::vec3 radiusNDC = glm::vec3(radiusPoint) / radiusPoint.w;
                                    float screenRadius = abs(radiusNDC.x - ndcPos.x) * 0.5f * extent.width;

                                    // Select if within particle's screen radius (with small tolerance) and closest so far
                                    if (screenDist < screenRadius * 1.1f && screenDist < closestScreenDist) {
                                        closestScreenDist = screenDist;
                                        selectedParticleIndex = i;
                                        selectedParticleScale = pParticles[i].scale;  // Read current scale from particle
                                    }
                                }
                            }
                        }
                    }
                }

                // Handle scroll wheel for selected particle scale
                if (selectedParticleIndex >= 0 && input.mouse.scroll.delta()[1] != 0.0f) {
                    VmaAllocationInfo particleBufferAllocInfo{ };
                    vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), particleBuffer.get<VmaAllocation>(), &particleBufferAllocInfo);
                    Particle* pParticles = (Particle*)particleBufferAllocInfo.pMappedData;

                    // Update the selected particle's scale directly in the buffer
                    pParticles[selectedParticleIndex].scale += input.mouse.scroll.delta()[1] * 1.0f;
                    pParticles[selectedParticleIndex].scale = glm::clamp(pParticles[selectedParticleIndex].scale, 0.1f, 128.0f);

                    // Update local tracking variable
                    selectedParticleScale = pParticles[selectedParticleIndex].scale;
                }

                gvk::math::FreeCameraController::UpdateInfo cameraControllerUpdateInfo {
                    /* .deltaTime           = */ deltaTime,
                    /* .moveUp              = */ input.keyboard.down(gvk::system::Key::Q),
                    /* .moveDown            = */ input.keyboard.down(gvk::system::Key::E),
                    /* .moveLeft            = */ input.keyboard.down(gvk::system::Key::A),
                    /* .moveRight           = */ input.keyboard.down(gvk::system::Key::D),
                    /* .moveForward         = */ input.keyboard.down(gvk::system::Key::W),
                    /* .moveBackward        = */ input.keyboard.down(gvk::system::Key::S),
                    /* .moveSpeedMultiplier = */ input.keyboard.down(gvk::system::Key::LeftShift) ? 2.0f : 1.0f,
                    /* .lookDelta           = */ { input.mouse.position.delta()[0], input.mouse.position.delta()[1] },
                    /* .fieldOfViewDelta    = */ 0.0f,
                };
                cameraController.lookEnabled = input.mouse.buttons.down(gvk::system::Mouse::Button::Right);
                if (cameraController.lookEnabled) {
                    systemSurface.set(gvk::system::Surface::CursorMode::Hidden);
                } else {
                    systemSurface.set(gvk::system::Surface::CursorMode::Visible);
                }
                if (input.mouse.buttons.pressed(gvk::system::Mouse::Button::Middle)) {
                    camera.fieldOfView = 60.0f;
                }
                cameraController.update(cameraControllerUpdateInfo);
            }

            gvk::wsi::AcquiredImageInfo acquiredImageInfo{ };
            gvk::RenderTarget acquiredImageRenderTarget = VK_NULL_HANDLE;
            auto wsiStatus = wsiContext.acquire_next_image(UINT64_MAX, VK_NULL_HANDLE, &acquiredImageInfo, &acquiredImageRenderTarget);
            if (wsiStatus == VK_SUCCESS || wsiStatus == VK_SUBOPTIMAL_KHR) {
                auto extent = wsiContext.get<gvk::SwapchainKHR>().get<VkSwapchainCreateInfoKHR>().imageExtent;
                camera.set_aspect_ratio(extent.width, extent.height);

                // Begin command buffer
                gvk::CommandBuffer gvkCommandBuffer = acquiredImageInfo.commandBuffer;
                gvk_result_assert(gvkCommandBuffer);
                gvk_result(gvkCommandBuffer.BeginCommandBuffer(&gvk::get_default<VkCommandBufferBeginInfo>()));

                // Compute pass: update particles
                if (!simParams.paused) {
                    gvkCommandBuffer.CmdBindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

                    if (!computeDescriptorSets.empty()) {
                        gvkCommandBuffer.CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.get<gvk::PipelineLayout>(), 0, 1, &computeDescriptorSets[0].get<VkDescriptorSet>(), 0, nullptr);
                    }

                    // Setup push constants
                    ComputePushConstants computePushConstants{ };
                    computePushConstants.particleCount = simParams.particleCount;
                    computePushConstants.deltaTime = simParams.timeStep;
                    computePushConstants.smoothingRadius = simParams.smoothingRadius;
                    computePushConstants.particleMass = simParams.particleMass;
                    computePushConstants.gasConstant = simParams.gasConstant;
                    computePushConstants.restDensity = simParams.restDensity;
                    computePushConstants.viscosity = simParams.viscosity;
                    computePushConstants.gravityStrength = simParams.gravityStrength;
                    computePushConstants.gravityRadius = simParams.gravityRadius;
                    computePushConstants.domainMinX = kDomainMinX;
                    computePushConstants.domainMaxX = kDomainMaxX;
                    computePushConstants.domainMinY = kDomainMinY;
                    computePushConstants.domainMaxY = kDomainMaxY;
                    computePushConstants.domainMinZ = kDomainMinZ;
                    computePushConstants.domainMaxZ = kDomainMaxZ;
                    computePushConstants.gridDimX = kGridDimensionX;
                    computePushConstants.gridDimY = kGridDimensionY;
                    computePushConstants.gridDimZ = kGridDimensionZ;

                    gvkCommandBuffer.CmdPushConstants(computePipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &computePushConstants);

                    // Dispatch compute shader
                    uint32_t workGroupCount = (simParams.particleCount + kComputeWorkGroupSize - 1) / kComputeWorkGroupSize;
                    gvkCommandBuffer.CmdDispatch(workGroupCount, 1, 1);

                    // Barrier: compute writes -> vertex shader reads
                    auto barrier = gvk::get_default<VkMemoryBarrier>();
                    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    gvkCommandBuffer.CmdPipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
                }

                // If the gvk::gui::Renderer is enabled, update values based on gui interaction
                if (showGui) {
                    // Update the gvk::system::Surface::CursorMode mode based on gui interaction
                    auto imguiCursor = ImGui::GetMouseCursor();
                    if (imguiCursor == ImGuiMouseCursor_None || ImGui::GetIO().MouseDrawCursor) {
                        systemSurface.set(gvk::system::Surface::CursorMode::Hidden);
                    } else {
                        switch (imguiCursor) {
                        case ImGuiMouseCursor_Arrow: systemSurface.set(gvk::system::Surface::CursorType::Arrow); break;
                        case ImGuiMouseCursor_TextInput: systemSurface.set(gvk::system::Surface::CursorType::IBeam); break;
                        case ImGuiMouseCursor_Hand: systemSurface.set(gvk::system::Surface::CursorType::Hand); break;
                        case ImGuiMouseCursor_ResizeNS: systemSurface.set(gvk::system::Surface::CursorType::ResizeNS); break;
                        case ImGuiMouseCursor_ResizeEW: systemSurface.set(gvk::system::Surface::CursorType::ResizeEW); break;
                        case ImGuiMouseCursor_ResizeAll: systemSurface.set(gvk::system::Surface::CursorType::ResizeAll); break;
                        case ImGuiMouseCursor_ResizeNESW: systemSurface.set(gvk::system::Surface::CursorType::ResizeNESW); break;
                        case ImGuiMouseCursor_ResizeNWSE: systemSurface.set(gvk::system::Surface::CursorType::ResizeNWSE); break;
                        case ImGuiMouseCursor_NotAllowed: systemSurface.set(gvk::system::Surface::CursorType::NotAllowed); break;
                        default: break;
                        }
                    }
                    if (systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::GainedFocus) {
                        ImGui::GetIO().AddFocusEvent(true);
                    }
                    if (systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::LostFocus) {
                        ImGui::GetIO().AddFocusEvent(false);
                    }

                    // Prepare a gvk::gui::Renderer::BeginInfo
                    const auto& textStream = systemSurface.get<gvk::system::Surface::TextStream>();
                    auto guiRendererBeginInfo = gvk::get_default<gvk::gui::Renderer::BeginInfo>();
                    guiRendererBeginInfo.deltaTime = deltaTime;
                    guiRendererBeginInfo.extent = { (float)extent.width, (float)extent.height };
                    guiRendererBeginInfo.pInput = &input;
                    guiRendererBeginInfo.textStreamCodePointCount = (uint32_t)textStream.size();
                    guiRendererBeginInfo.pTextStreamCodePoints = !textStream.empty() ? textStream.data() : nullptr;

                    // Call guiRenderer.begin_gui().  Note that all ImGui widgets must be handled
                    //  between calls to begin_gui()/end_gui()
                    guiRenderer.begin_gui(guiRendererBeginInfo);

                    // SPH Simulation Control Panel
                    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
                    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
                    if (ImGui::Begin("SPH Simulation Controls", nullptr)) {

                        // Simulation State
                        ImGui::SeparatorText("Simulation");
                        ImGui::Checkbox("Paused", &simParams.paused);
                        ImGui::SameLine();
                        if (ImGui::Button("Reset")) {
                            // Clamp particle count to max to be safe
                            simParams.particleCount = std::min(simParams.particleCount, kMaxParticleCount);

                            // Reinitialize particles
                            initialize_particles(particles, simParams.particleCount, simParams.initMode);

                            // Copy to GPU buffer (which is allocated for max particles)
                            memcpy(particleBufferAllocationInfo.pMappedData, particles.data(), particles.size() * sizeof(Particle));
                            selectedParticleIndex = -1;
                        }

                        // Particle count (requires reset)
                        ImGui::Text("Particle Count: %u", simParams.particleCount);
                        if (ImGui::SliderInt("##ParticleCount", (int*)&simParams.particleCount, 100, kMaxParticleCount)) {
                            simParams.particleCount = (simParams.particleCount / 100) * 100; // Round to hundreds
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Click 'Reset' to apply new particle count\nSpatial grid acceleration enabled for high counts");
                        }

                        // Initialization mode
                        const char* initModes[] = { "Uniform Random", "Gaussian Cloud", "Multiple Clusters", "Explosion" };
                        ImGui::Combo("Init Mode", (int*)&simParams.initMode, initModes, IM_ARRAYSIZE(initModes));

                        ImGui::Spacing();
                        ImGui::SeparatorText("Time Step");
                        ImGui::SliderFloat("Delta Time", &simParams.timeStep, kMinTimeStep, kMaxTimeStep, "%.4f");

                        ImGui::Spacing();
                        ImGui::SeparatorText("SPH Parameters");
                        ImGui::SliderFloat("Smoothing Radius", &simParams.smoothingRadius, 0.05f, 1.0f, "%.3f");
                        ImGui::SliderFloat("Particle Mass", &simParams.particleMass, 0.1f, 10.0f, "%.2f");
                        ImGui::SliderFloat("Gas Constant", &simParams.gasConstant, 10.0f, 1000.0f, "%.1f");
                        ImGui::SliderFloat("Rest Density", &simParams.restDensity, 1.0f, 50.0f, "%.1f");
                        ImGui::SliderFloat("Viscosity", &simParams.viscosity, 0.0f, 5.0f, "%.2f");

                        ImGui::Spacing();
                        ImGui::SeparatorText("Gravity");
                        ImGui::SliderFloat("Strength", &simParams.gravityStrength, 0.0f, 5.0f, "%.2f");
                        ImGui::SliderFloat("Radius", &simParams.gravityRadius, 0.0f, 5.0f, "%.2f");

                        ImGui::Spacing();
                        ImGui::SeparatorText("Selected Particle");
                        if (selectedParticleIndex >= 0) {
                            ImGui::Text("Particle #%d", selectedParticleIndex);

                            // Scale slider for selected particle
                            if (ImGui::SliderFloat("Scale", &selectedParticleScale, 0.1f, 128.0f, "%.2f", ImGuiSliderFlags_Logarithmic)) {
                                // Update the particle's scale in the buffer
                                VmaAllocationInfo particleBufferAllocInfo{ };
                                vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), particleBuffer.get<VmaAllocation>(), &particleBufferAllocInfo);
                                Particle* pParticles = (Particle*)particleBufferAllocInfo.pMappedData;
                                pParticles[selectedParticleIndex].scale = selectedParticleScale;
                            }

                            if (ImGui::Button("Deselect")) {
                                selectedParticleIndex = -1;
                            }
                        } else {
                            ImGui::TextDisabled("No particle selected");
                            ImGui::TextDisabled("Left-click to select");
                            ImGui::TextDisabled("Scroll wheel to scale");
                        }

                        ImGui::Spacing();
                        ImGui::SeparatorText("Performance");
                        ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
                        ImGui::Text("Frame Time: %.2f ms", deltaTime * 1000.0f);

                        ImGui::Spacing();
                        ImGui::SeparatorText("Controls");
                        ImGui::BulletText("Left Click: Select particle");
                        ImGui::BulletText("Right Click: Camera look");
                        ImGui::BulletText("Middle Click: Reset FOV");
                        ImGui::BulletText("Scroll: Scale particle");
                        ImGui::BulletText("WASD: Move camera");
                        ImGui::BulletText("Q/E: Up/Down");
                        ImGui::BulletText("` (backtick): Toggle GUI");
                    }
                    ImGui::End();

                    guiRenderer.end_gui(acquiredImageInfo.index);
                }

                auto renderPassBeginInfo = acquiredImageRenderTarget.get<VkRenderPassBeginInfo>();
                gvkCommandBuffer.CmdBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                // Set viewport and scissor (required for dynamic state)
                VkRect2D scissor{ { }, renderPassBeginInfo.renderArea.extent };
                gvkCommandBuffer.CmdSetScissor(0, 1, &scissor);
                VkViewport viewport{ 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                gvkCommandBuffer.CmdSetViewport(0, 1, &viewport);

                // Draw particles
                gvkCommandBuffer.CmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

                if (!graphicsDescriptorSets.empty()) {
                    gvkCommandBuffer.CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.get<gvk::PipelineLayout>(), 0, 1, &graphicsDescriptorSets[0].get<VkDescriptorSet>(), 0, nullptr);
                }

                // Setup push constants for graphics pipeline
                struct GraphicsPushConstants {
                    glm::mat4 viewProjection;
                    glm::vec3 cameraRight;
                    float particleScale;
                    glm::vec3 cameraUp;
                    uint32_t showCollisionCircles;
                    uint32_t selectedParticleIndex;
                    float selectedParticleScale;
                };
                GraphicsPushConstants graphicsPushConstants{ };
                graphicsPushConstants.viewProjection = camera.projection() * camera.view();
                graphicsPushConstants.cameraRight = camera.transform.right();
                graphicsPushConstants.particleScale = 1.0f;
                graphicsPushConstants.cameraUp = camera.transform.up();
                graphicsPushConstants.showCollisionCircles = 0;
                graphicsPushConstants.selectedParticleIndex = selectedParticleIndex >= 0 ? (uint32_t)selectedParticleIndex : UINT32_MAX;
                graphicsPushConstants.selectedParticleScale = selectedParticleScale;
                gvkCommandBuffer.CmdPushConstants(graphicsPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GraphicsPushConstants), &graphicsPushConstants);

                // Bind vertex buffer (quad vertices)
                VkDeviceSize offset = 0;
                VkBuffer vkQuadVertexBuffer = quadVertexBuffer.get<VkBuffer>();
                gvkCommandBuffer.CmdBindVertexBuffers(0, 1, &vkQuadVertexBuffer, &offset);

                // Draw instanced quads (one instance per particle)
                // 6 vertices = 2 triangles per quad
                gvkCommandBuffer.CmdDraw(6, simParams.particleCount, 0, 0);

                // If the gvk::gui::Renderer is enabled, record cmds to render it
                if (showGui) {
                    guiRenderer.record_cmds(acquiredImageInfo.commandBuffer, acquiredImageInfo.index);
                }

                gvkCommandBuffer.CmdEndRenderPass();

                gvk_result(gvkCommandBuffer.EndCommandBuffer());

                gvk_result(gvkQueue.QueueSubmit(1, &wsiContext.get<VkSubmitInfo>(acquiredImageInfo), acquiredImageInfo.fence));

                wsiStatus = wsiContext.queue_present(gvkQueue, &acquiredImageInfo);
                gvk_result((wsiStatus == VK_SUBOPTIMAL_KHR || wsiStatus == VK_ERROR_OUT_OF_DATE_KHR) ? VK_SUCCESS : wsiStatus);
            }
        }
        gvk_result(gvkDevice.DeviceWaitIdle());
    } gvk_result_scope_end;
    if (gvkResult) {
        std::cerr << gvk::to_string(gvkResult) << std::endl;
    }
    return (int)gvkResult;
}
