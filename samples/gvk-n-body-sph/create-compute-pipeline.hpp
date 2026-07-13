
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

// Particle structure (GPU layout)
// Must match the layout in compute and vertex shaders
struct Particle
{
    glm::vec4 position;    // xyz = position, w = radius (16 bytes, offset 0)
    glm::vec4 velocity;    // xyz = velocity, w = mass (16 bytes, offset 16)
    glm::vec4 color;       // rgba color (16 bytes, offset 32)
    glm::vec4 force;       // xyz = accumulated force, w = padding (16 bytes, offset 48)
    glm::vec4 homePosition; // xyz = original position, w = padding (16 bytes, offset 64)
    float density;         // SPH density (4 bytes, offset 80)
    float pressure;        // SPH pressure (4 bytes, offset 84)
    float scale;           // Per-particle visual scale (4 bytes, offset 88)
    float wakeRadius;      // Distance to check for wake-up (4 bytes, offset 92)
    uint32_t gridCell;     // Spatial grid cell index (4 bytes, offset 96)
    uint32_t sleepCounter; // Frames stationary (4 bytes, offset 100)
    float padding1;        // Alignment (4 bytes, offset 104)
    float padding2;        // Alignment (4 bytes, offset 108)
};

// Push constants for compute shaders
struct ComputePushConstants
{
    uint32_t particleCount;
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
    uint32_t gridDimX;
    uint32_t gridDimY;
    uint32_t gridDimZ;
    uint32_t selectedParticleIndex;  // Index of selected particle (UINT32_MAX if none)
};

inline VkResult create_compute_pipeline(const gvk::Device& device, gvk::spirv::Context& spirvContext, gvk::Pipeline* pPipeline)
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
                float scale;      // Per-particle visual scale
                float wakeRadius;  // Distance to check for wake-up
                uint gridCell;
                uint sleepCounter; // Frames stationary (0 = awake, >threshold = asleep)
                float padding1;
                float padding2;
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
                uint selectedParticleIndex;  // Index of selected particle (UINT32_MAX if none)
            } pushConstants;

            // Sleep/Wake system constants
            const uint SLEEP_THRESHOLD = 20u;      // Marker value for asleep state
            const float SLEEP_VELOCITY_THRESHOLD = 0.01;  // Immediate sleep if velocity < this
            const float WAKE_RADIUS_MULTIPLIER = 1.2;     // Wake detection margin

            // SPH kernels for smooth fluid behavior
            float poly6Kernel(float r, float h)
            {
                if (r >= 0.0 && r <= h) {
                    float x = (h * h - r * r);
                    return 315.0 / (64.0 * 3.14159265 * pow(h, 9.0)) * x * x * x;
                }
                return 0.0;
            }

            vec3 spikyKernelGradient(vec3 r, float h)
            {
                float rLen = length(r);
                if (rLen > 0.0 && rLen <= h) {
                    float x = (h - rLen);
                    return -45.0 / (3.14159265 * pow(h, 6.0)) * x * x * (r / rLen);
                }
                return vec3(0.0);
            }

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
                float pEffectiveRadius = p.position.w * p.scale;

                // Save original position for selected particle (locked in place)
                vec3 savedPosition = p.position.xyz;
                bool isSelectedParticle = (index == pushConstants.selectedParticleIndex);

                // SLEEP/WAKE SYSTEM: Disabled for debugging
                // TODO: Re-enable once base physics are verified
                /*
                bool isAsleep = (p.sleepCounter >= SLEEP_THRESHOLD);
                if (isAsleep) {
                    particles[index] = p;
                    return;
                }
                */

                p.force.xyz = vec3(0.0);
                p.density = 0.0;

                // PASS 1: Density calculation
                for (uint j = 0; j < pushConstants.particleCount; j++) {
                    Particle pj = particles[j];
                    vec2 diff2d = p.position.xy - pj.position.xy;
                    float distSq = dot(diff2d, diff2d);
                    float smoothRadSq = pushConstants.smoothingRadius * pushConstants.smoothingRadius;

                    if (distSq < smoothRadSq) {
                        float dist = sqrt(distSq);
                        p.density += pushConstants.particleMass * poly6Kernel(dist, pushConstants.smoothingRadius);
                    }
                }

                p.density = max(p.density, 0.01);
                p.pressure = pushConstants.gasConstant * max(p.density - pushConstants.restDensity, 0.0);

                // PASS 2: Forces (SPH + Collision)
                vec3 pressureForce = vec3(0.0);
                vec3 viscosityForce = vec3(0.0);
                vec3 collisionForce = vec3(0.0);

                // Check ALL particles for forces
                for (uint j = 0; j < pushConstants.particleCount; j++) {
                    if (j == index) continue;

                    Particle pj = particles[j];
                    vec2 diff2d = p.position.xy - pj.position.xy;
                    float distSq = dot(diff2d, diff2d);

                    float pjEffectiveRadius = pj.position.w * pj.scale;
                    float maxInteractionRadius = max(pushConstants.smoothingRadius, pEffectiveRadius + pjEffectiveRadius);

                    if (distSq < maxInteractionRadius * maxInteractionRadius) {
                        vec3 diff = vec3(diff2d, 0.0);
                        float dist = sqrt(distSq);

                        if (dist < 0.001) {
                            diff = vec3(0.001, 0.0, 0.0);
                            dist = 0.001;
                        }

                        vec3 normal = normalize(diff);
                        float minDist = pEffectiveRadius + pjEffectiveRadius;

                        // Hard collision - PRIORITY OVER SPH
                        if (dist < minDist) {
                            float overlap = minDist - dist;

                            // Scale collision strength with particle size (larger = stronger)
                            float avgScale = (p.scale + pj.scale) * 0.5;
                            float baseStrength = 5000.0;
                            float scaleMultiplier = 1.0 + (avgScale - 1.0) * 0.5;  // Larger particles push harder
                            float collisionStrength = baseStrength * scaleMultiplier;

                            float overlapFactor = overlap / minDist;
                            collisionForce += normal * overlap * collisionStrength * (1.0 + overlapFactor * 5.0);

                            vec3 relativeVel = p.velocity.xyz - pj.velocity.xyz;
                            float velAlongNormal = dot(relativeVel, normal);
                            if (velAlongNormal < 0.0) {
                                collisionForce -= normal * velAlongNormal * 200.0;
                            }
                        }
                        // SPH forces - ONLY when not colliding (maintain spacing)
                        else if (dist < pushConstants.smoothingRadius) {
                            vec3 gradW = spikyKernelGradient(diff, pushConstants.smoothingRadius);
                            float pjDensity = max(particles[j].density, 0.01);

                            float pressureTerm = (p.pressure / (p.density * p.density)) + (pj.pressure / (pjDensity * pjDensity));
                            pressureForce -= pushConstants.particleMass * pressureTerm * gradW;

                            float lapW = viscosityKernelLaplacian(dist, pushConstants.smoothingRadius);
                            viscosityForce += pushConstants.viscosity * pushConstants.particleMass * 
                                             (pj.velocity.xyz - p.velocity.xyz) / pjDensity * lapW;
                        }
                    }
                }

                // Apply all forces: collision + SPH (no spring)
                p.force.xyz = collisionForce + pressureForce + viscosityForce;

                // Gravity (optional)
                if (pushConstants.gravityStrength > 0.001) {
                    vec2 toCenter = -p.position.xy;
                    float distToCenter = length(toCenter);
                    if (distToCenter > pushConstants.gravityRadius) {
                        vec2 gravityDir = toCenter / distToCenter;
                        float gravityMag = pushConstants.gravityStrength * (distToCenter - pushConstants.gravityRadius);
                        p.force.xy += gravityDir * gravityMag;
                    }
                }

                // Integration
                vec3 acceleration = p.force.xyz / pushConstants.particleMass;
                acceleration.z = 0.0;

                float maxAccel = 500.0;
                float accelMag = length(acceleration);
                if (accelMag > maxAccel) {
                    acceleration = (acceleration / accelMag) * maxAccel;
                }

                p.velocity.xyz += acceleration * pushConstants.deltaTime;
                p.velocity.z = 0.0;
                p.velocity.xyz *= 0.995;  // Standard damping

                float maxVel = 30.0;
                float velMag = length(p.velocity.xyz);
                if (velMag > maxVel) {
                    p.velocity.xyz = (p.velocity.xyz / velMag) * maxVel;
                }

                p.position.xyz += p.velocity.xyz * pushConstants.deltaTime;
                p.position.z = 0.0;

                // Lock selected particle in place (acts like a rock in the fluid)
                if (isSelectedParticle) {
                    p.position.xyz = savedPosition;  // Restore original position
                    p.velocity.xyz = vec3(0.0);      // Zero velocity (no movement)
                }

                // Boundaries
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

                // Color based on density
                float densityRatio = p.density / pushConstants.restDensity;
                float densityNormalized = clamp((densityRatio - 0.5) / 1.5, 0.0, 1.0);
                p.color = vec4(densityNormalized, 0.3 + densityNormalized * 0.3, 1.0 - densityNormalized, 1.0);

                // SLEEP/WAKE SYSTEM: Disabled for debugging
                // Keep particles awake for now
                p.sleepCounter = 0u;

                // Update wake radius based on current scale
                p.wakeRadius = p.position.w * p.scale * WAKE_RADIUS_MULTIPLIER;

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

} // namespace sph
} // namespace gvk
