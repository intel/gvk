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

#include "defines.hpp"

#include "gvk-defines.hpp"
#include "gvk-format-info.hpp"
#include "gvk-handles.hpp"
#include "gvk-math.hpp"
#include "gvk-spirv.hpp"
#include "gvk-string.hpp"
#include "gvk-structures.hpp"
#include "gvk-system.hpp"

#include <iostream>
#include <vector>

namespace gvk {
namespace sph {

// Particle structure (GPU layout)
// Must match the layout in compute and vertex shaders
struct Particle
{
    glm::vec4 position;    // xyz = position, w = radius
    glm::vec4 velocity;    // xyz = velocity, w = mass
    glm::vec4 color;       // rgba color
    glm::vec4 force;       // xyz = accumulated force, w = padding
    float density;         // SPH density
    float pressure;        // SPH pressure
    uint32_t gridCell;     // Spatial grid cell index
    float scale;           // Per-particle visual scale
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

// Simulation parameters (GUI-controllable)
struct SimulationParameters
{
    uint32_t particleCount = kDefaultParticleCount;
    float timeStep = kDefaultTimeStep;
    float smoothingRadius = kDefaultSmoothingRadius;
    float particleMass = kDefaultParticleMass;
    float gasConstant = kDefaultGasConstant;
    float restDensity = kDefaultRestDensity;
    float viscosity = kDefaultViscosity;
    float gravityStrength = kDefaultGravityStrength;
    float gravityRadius = kDefaultGravityRadius;
    float particleScale = kDefaultParticleScale;
    bool showCollisionCircles = false;
    bool paused = false;

    enum InitializationMode
    {
        UniformRandom,
        GaussianCloud,
        MultipleClusters,
        Explosion
    };
    InitializationMode initMode = GaussianCloud;
};

struct CameraUniforms
{
    glm::mat4 view{ };
    glm::mat4 projection{ };
};

// Application context (not using gvk-sample-utilities)
class SphContext final : public gvk::Context
{
public:
    using gvk::Context::Context;

    static VkResult create(const char* pApplicationName, SphContext* pSphContext);

private:
    static VkBool32 debug_utils_messenger_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    static VkBool32 process_gvk_error(VkResult vkResult, const char* pFileLine, const char* pGvkCall);
};

// System surface creation
VkResult create_system_surface(const gvk::Context& context, gvk::system::Surface* pSystemSurface);

// WSI context creation
VkResult create_wsi_context(
    const gvk::Context& gvkContext,
    const gvk::system::Surface& systemSurface,
    gvk::wsi::Context* pWsiContext
);

// Shader compilation helper
VkResult validate_shader_info(const gvk::spirv::ShaderInfo& shaderInfo);

// Particle initialization
void initialize_particles(
    std::vector<Particle>& particles,
    uint32_t count,
    SimulationParameters::InitializationMode mode
);

} // namespace sph
} // namespace gvk
