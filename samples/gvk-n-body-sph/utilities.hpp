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

struct Particle;

// Grid cell metadata for spatial binning and dispatch culling
struct GridCell
{
    uint32_t particleCount;       // Total particles in this cell
    uint32_t particleStartIndex;  // Offset into sorted particle array (set by prefix sum)
    uint32_t awakeCount;          // Awake particles in cell (for future sleep system)
    uint32_t _pad;                // Alignment padding
};

// Helper: Convert 3D grid coordinates to flat cell index
inline uint32_t grid_cell_index(int x, int y, int z)
{
    return x + y * kGridDimensionX + z * kGridDimensionX * kGridDimensionY;
}

// Helper: Convert world position to grid cell coordinates
inline void world_to_grid_cell(float x, float y, float z, int* cellX, int* cellY, int* cellZ)
{
    *cellX = static_cast<int>((x - kDomainMinX) / kGridCellSize);
    *cellY = static_cast<int>((y - kDomainMinY) / kGridCellSize);
    *cellZ = static_cast<int>((z - kDomainMinZ) / kGridCellSize);

    // Clamp to valid grid range
    *cellX = std::max(0, std::min(static_cast<int>(kGridDimensionX) - 1, *cellX));
    *cellY = std::max(0, std::min(static_cast<int>(kGridDimensionY) - 1, *cellY));
    *cellZ = std::max(0, std::min(static_cast<int>(kGridDimensionZ) - 1, *cellZ));
}

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
    bool hardEdges = true;  // Toggle between hard and soft edges (default: hard)
    bool showVelocityColors = false;  // Debug: Color particles by velocity
    bool showGrid = false;  // Debug: Visualize spatial grid cells (wireframe)
    bool showGridOccupancy = false;  // Debug: Show grid cell occupancy heatmap
    bool msaaEnabled = true;  // MSAA antialiasing (startup only)
    bool paused = false;

    enum InitializationMode
    {
        StableGrid,
        UniformRandom,
        GaussianCloud,
        MultipleClusters,
        Explosion
    };
    InitializationMode initMode = StableGrid;
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
    bool enableMSAA,
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
