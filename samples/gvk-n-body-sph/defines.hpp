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

#include "gvk-defines.hpp"

#include <cstdint>

namespace gvk {
namespace sph {

// Compile-time configuration
constexpr uint32_t kDefaultParticleCount = 10000;
constexpr uint32_t kMaxParticleCount = 1000000;  // 1 million for UI performance
constexpr uint32_t kComputeWorkGroupSize = 256;
constexpr uint32_t kMaxNeighborsToCheck = 64;  // Limit interactions per particle for performance

// Simulation domain
constexpr float kDomainSize = 10.0f;
constexpr float kDomainMinX = -kDomainSize * (16.0f / 9.0f);
constexpr float kDomainMaxX = kDomainSize * (16.0f / 9.0f);
constexpr float kDomainMinY = -kDomainSize;
constexpr float kDomainMaxY = kDomainSize;
constexpr float kDomainMinZ = -kDomainSize;
constexpr float kDomainMaxZ = kDomainSize;

// SPH parameters (tuned for graph layout - fast settling)
constexpr float kDefaultSmoothingRadius = 0.3f;
constexpr float kDefaultParticleMass = 1.0f;
constexpr float kDefaultGasConstant = 50.0f;
constexpr float kDefaultRestDensity = 10.0f;
constexpr float kDefaultViscosity = 5.0f;  // HIGH viscosity = faster settling (was 0.5)

// Gravity parameters (disabled by default for UI mode)
constexpr float kDefaultGravityStrength = 0.0f;  // Disabled for stable UI
constexpr float kDefaultGravityRadius = 1.0f;

// Rendering
constexpr float kDefaultParticleRadius = 0.05f;
constexpr float kDefaultParticleScale = 1.0f;

// Spatial grid configuration for dispatch culling and neighbor search
// Grid cell size matches SPH interaction radius for optimal neighbor queries
constexpr float kGridCellSize = kDefaultSmoothingRadius;

// Grid dimensions based on simulation domain and cell size
constexpr uint32_t kGridDimensionX = static_cast<uint32_t>((kDomainMaxX - kDomainMinX) / kGridCellSize) + 1;
constexpr uint32_t kGridDimensionY = static_cast<uint32_t>((kDomainMaxY - kDomainMinY) / kGridCellSize) + 1;
constexpr uint32_t kGridDimensionZ = static_cast<uint32_t>((kDomainMaxZ - kDomainMinZ) / kGridCellSize) + 1;
constexpr uint32_t kGridCellCount = kGridDimensionX * kGridDimensionY * kGridDimensionZ;

// Grid-dispatch configuration
constexpr uint32_t kGridWorkGroupSize = 64;  // Particles per workgroup in grid-based dispatch

// Simulation
constexpr float kDefaultTimeStep = 0.005f;  // Balanced for stability and performance
constexpr float kMinTimeStep = 0.001f;
constexpr float kMaxTimeStep = 0.033f;

// Sleep/Wake system for performance
constexpr uint32_t kSleepThreshold = 20;        // Marker value for asleep state (not a counter)
constexpr float kSleepVelocityThreshold = 0.01f; // Immediate sleep if velocity < this
constexpr float kWakeRadiusMultiplier = 1.2f;   // Wake radius = particle radius * scale * this

} // namespace sph
} // namespace gvk
