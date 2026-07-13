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

#include <cassert>
#include <random>

namespace gvk {
namespace sph {

VkBool32 SphContext::debug_utils_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    (void)messageTypes;
    (void)pUserData;
    if (pCallbackData && pCallbackData->pMessage) {
        if (messageSeverity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)) {
            std::cerr << pCallbackData->pMessage << std::endl;
        } else {
            std::cout << pCallbackData->pMessage << std::endl;
        }
    }
    return VK_FALSE;
}

VkBool32 SphContext::process_gvk_error(VkResult vkResult, const char* pFileLine, const char* pGvkCall)
{
    std::cerr << pFileLine << std::endl;
    std::cerr << pGvkCall << std::endl;
    std::cerr << gvk::to_string(vkResult, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
    return VK_FALSE;
}

VkResult SphContext::create(const char* pApplicationName, SphContext* pSphContext)
{
    gvk::gPfnGvkResultScopeCallback = process_gvk_error;

    assert(pApplicationName);
    assert(pSphContext);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        auto applicationInfo = gvk::get_default<VkApplicationInfo>();
        applicationInfo.pApplicationName = pApplicationName;
        auto instanceCreateInfo = gvk::get_default<VkInstanceCreateInfo>();
        instanceCreateInfo.pApplicationInfo = &applicationInfo;

        auto physicalDeviceFeatures = gvk::get_default<VkPhysicalDeviceFeatures>();
        physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
        auto deviceCreateInfo = gvk::get_default<VkDeviceCreateInfo>();
        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

        auto debugUtilsMessengerCreateInfo = gvk::get_default<VkDebugUtilsMessengerCreateInfoEXT>();
        debugUtilsMessengerCreateInfo.pfnUserCallback = debug_utils_messenger_callback;

        auto contextCreateInfo = gvk::get_default<gvk::Context::CreateInfo>();
        contextCreateInfo.pInstanceCreateInfo = &instanceCreateInfo;
        contextCreateInfo.loadApiDumpLayer = VK_FALSE;
        contextCreateInfo.loadValidationLayer = VK_TRUE;
        contextCreateInfo.loadWsiExtensions = VK_TRUE;
        contextCreateInfo.pDebugUtilsMessengerCreateInfo = &debugUtilsMessengerCreateInfo;
        contextCreateInfo.pDeviceCreateInfo = &deviceCreateInfo;
        gvk_result(gvk::Context::create(&contextCreateInfo, nullptr, pSphContext));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult create_system_surface(const gvk::Context& context, gvk::system::Surface* pSystemSurface)
{
    assert(context);
    assert(pSystemSurface);
    const auto& vkInstanceCreateInfo = context.get<gvk::Instance>().get<VkInstanceCreateInfo>();
    auto systemSurfaceCreateInfo = gvk::get_default<gvk::system::Surface::CreateInfo>();
    if (vkInstanceCreateInfo.pApplicationInfo) {
        systemSurfaceCreateInfo.pTitle = vkInstanceCreateInfo.pApplicationInfo->pApplicationName;
    }
    systemSurfaceCreateInfo.extent = { 1280, 720 };
    return (VkResult)gvk::system::Surface::create(&systemSurfaceCreateInfo, pSystemSurface);
}

VkResult create_wsi_context(
    const gvk::Context& gvkContext,
    const gvk::system::Surface& systemSurface,
    gvk::wsi::Context* pWsiContext
)
{
    assert(gvkContext);
    assert(pWsiContext);
    auto device = gvkContext.get<gvk::Devices>()[0];
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        const VkBaseInStructure* pSurfaceCreateInfo = nullptr;
#ifdef VK_USE_PLATFORM_XLIB_KHR
        auto xlibSurfaceCreateInfo = gvk::get_default<VkXlibSurfaceCreateInfoKHR>();
        xlibSurfaceCreateInfo.dpy = systemSurface.get<gvk::system::Surface::PlatformInfo>().x11Display;
        xlibSurfaceCreateInfo.window = systemSurface.get<gvk::system::Surface::PlatformInfo>().x11Window;
        pSurfaceCreateInfo = (VkBaseInStructure*)&xlibSurfaceCreateInfo;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        auto win32SurfaceCreateInfo = gvk::get_default<VkWin32SurfaceCreateInfoKHR>();
        win32SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
        win32SurfaceCreateInfo.hwnd = systemSurface.get<gvk::system::Surface::PlatformInfo>().hwnd;
        pSurfaceCreateInfo = (VkBaseInStructure*)&win32SurfaceCreateInfo;
#endif
        gvk::SurfaceKHR surface = VK_NULL_HANDLE;
        gvk_result(gvk::SurfaceKHR::create(device.get<gvk::Instance>(), pSurfaceCreateInfo, nullptr, &surface));

        auto wsiContextCreateInfo = gvk::get_default<gvk::wsi::Context::CreateInfo>();
        wsiContextCreateInfo.queueFamilyIndex = gvk::get_queue_family(device, 0).queues[0].get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
        wsiContextCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        wsiContextCreateInfo.depthFormat = VK_FORMAT_D32_SFLOAT;
        wsiContextCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
        gvk_result(gvk::wsi::Context::create(device, surface, &wsiContextCreateInfo, nullptr, pWsiContext));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult validate_shader_info(const gvk::spirv::ShaderInfo& shaderInfo)
{
    if (!shaderInfo.errors.empty()) {
        for (const auto& error : shaderInfo.errors) {
            std::cerr << error << std::endl;
        }
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

void initialize_particles(
    std::vector<Particle>& particles,
    uint32_t count,
    SimulationParameters::InitializationMode mode
)
{
    particles.resize(count);

    std::random_device rd;
    std::mt19937 gen(rd());

    switch (mode) {
    case SimulationParameters::UniformRandom: {
        std::uniform_real_distribution<float> distX(kDomainMinX * 0.5f, kDomainMaxX * 0.5f);
        std::uniform_real_distribution<float> distY(kDomainMinY * 0.5f, kDomainMaxY * 0.5f);
        std::uniform_real_distribution<float> velDist(-1.0f, 1.0f);

        for (auto& particle : particles) {
            particle.position = glm::vec4(distX(gen), distY(gen), 0.0f, kDefaultParticleRadius);
            particle.velocity = glm::vec4(velDist(gen), velDist(gen), 0.0f, kDefaultParticleMass);
            particle.color = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);
            particle.force = glm::vec4(0.0f);
            particle.density = 0.0f;
            particle.pressure = 0.0f;
            particle.gridCell = 0;
            particle.scale = 1.0f;
        }
        break;
    }

    case SimulationParameters::GaussianCloud: {
        std::normal_distribution<float> distX(0.0f, 1.5f);
        std::normal_distribution<float> distY(0.0f, 1.5f);
        std::uniform_real_distribution<float> velNoise(-0.1f, 0.1f);

        for (auto& particle : particles) {
            glm::vec3 pos(distX(gen), distY(gen), 0.0f);
            particle.position = glm::vec4(pos, kDefaultParticleRadius);
            // Give particles small random velocities for natural movement
            particle.velocity = glm::vec4(velNoise(gen), velNoise(gen), 0.0f, kDefaultParticleMass);
            particle.color = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);
            particle.force = glm::vec4(0.0f);
            particle.density = 0.0f;
            particle.pressure = 0.0f;
            particle.gridCell = 0;
            particle.scale = 1.0f;
        }
        break;
    }

    case SimulationParameters::MultipleClusters: {
        std::uniform_real_distribution<float> clusterDist(-3.0f, 3.0f);
        std::normal_distribution<float> particleDist(0.0f, 0.5f);
        std::uniform_real_distribution<float> velNoiseDist(-0.1f, 0.1f);

        uint32_t clustersCount = 3;
        uint32_t particlesPerCluster = count / clustersCount;

        for (uint32_t i = 0; i < clustersCount; ++i) {
            glm::vec3 clusterCenter(clusterDist(gen), clusterDist(gen), 0.0f);
            // Give each cluster a velocity toward the origin
            glm::vec3 clusterVelocity = -glm::normalize(clusterCenter + glm::vec3(0.001f)) * 0.5f;

            for (uint32_t j = 0; j < particlesPerCluster && (i * particlesPerCluster + j) < count; ++j) {
                auto& particle = particles[i * particlesPerCluster + j];
                particle.position = glm::vec4(
                    clusterCenter.x + particleDist(gen),
                    clusterCenter.y + particleDist(gen),
                    0.0f,
                    kDefaultParticleRadius
                );
                // Add some random noise to cluster velocity
                particle.velocity = glm::vec4(
                    clusterVelocity.x + velNoiseDist(gen),
                    clusterVelocity.y + velNoiseDist(gen),
                    0.0f,
                    kDefaultParticleMass
                );
                particle.color = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);
                particle.force = glm::vec4(0.0f);
                particle.density = 0.0f;
                particle.pressure = 0.0f;
                particle.gridCell = 0;
                particle.scale = 1.0f;
            }
        }
        break;
    }

    case SimulationParameters::Explosion: {
        std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
        std::uniform_real_distribution<float> speedDist(0.5f, 2.0f);
        std::normal_distribution<float> radiusDist(0.0f, 0.5f);

        for (auto& particle : particles) {
            float angle = angleDist(gen);
            float speed = speedDist(gen);
            float radius = std::abs(radiusDist(gen));

            particle.position = glm::vec4(
                radius * std::cos(angle),
                radius * std::sin(angle),
                0.0f,
                kDefaultParticleRadius
            );
            particle.velocity = glm::vec4(
                speed * std::cos(angle),
                speed * std::sin(angle),
                0.0f,
                kDefaultParticleMass
            );
            particle.color = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);
            particle.force = glm::vec4(0.0f);
            particle.density = 0.0f;
            particle.pressure = 0.0f;
            particle.gridCell = 0;
            particle.scale = 1.0f;
        }
        break;
    }
    }
}

} // namespace sph
} // namespace gvk
