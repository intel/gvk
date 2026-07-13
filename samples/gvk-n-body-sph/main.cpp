
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

#include "create-binning-pipeline.hpp"
#include "create-compute-pipeline.hpp"
#include "create-graphics-pipeline.hpp"
#include "create-grid-visualization-pipeline.hpp"
#include "utilities.hpp"
#include "gvk-gui.hpp"

#include <array>
#include <iostream>

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

        // Initialize simulation parameters (needed for WSI context creation)
        SimulationParameters simParams;

        gvk::wsi::Context wsiContext;
        gvk_result(create_wsi_context(context, systemSurface, simParams.msaaEnabled, &wsiContext));

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

        // Create grid cell buffer on GPU for spatial binning (Milestone 2)
        // Size: 534K cells * 16 bytes = ~8.5 MB
        bufferCreateInfo.size = kGridCellCount * sizeof(GridCell);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        gvk::Buffer gridCellGpuBuffer;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &vmaAllocationCreateInfo, &gridCellGpuBuffer));

        // Create grid clear pipeline (clears cell counters before binning)
        gvk::Pipeline gridClearPipeline;
        gvk_result(create_grid_clear_pipeline(gvkDevice, spirvContext, &gridClearPipeline));

        // Create binning pipeline (bins particles into grid cells)
        gvk::Pipeline binningPipeline;
        gvk_result(create_binning_pipeline(gvkDevice, spirvContext, &binningPipeline));

        // Create compute pipeline
        gvk::Pipeline computePipeline;
        gvk_result(create_compute_pipeline(gvkDevice, spirvContext, &computePipeline));

        // Create graphics pipeline
        gvk::Pipeline graphicsPipeline;
        gvk_result(create_graphics_pipeline(gvkDevice, wsiContext.get<gvk::RenderPass>(), spirvContext, &graphicsPipeline));

        // Create grid visualization pipeline
        gvk::Pipeline gridVisualizationPipeline;
        gvk_result(create_grid_visualization_pipeline(gvkDevice, wsiContext.get<gvk::RenderPass>(), spirvContext, &gridVisualizationPipeline));

        // Create grid cell occupancy pipeline
        gvk::Pipeline gridCellPipeline;
        gvk_result(create_grid_cell_pipeline(gvkDevice, wsiContext.get<gvk::RenderPass>(), spirvContext, &gridCellPipeline));

        // Create grid line vertex buffer
        auto gridLines = generate_grid_lines();
        uint32_t gridLineVertexCount = static_cast<uint32_t>(gridLines.size());
        bufferCreateInfo.size = gridLines.size() * sizeof(GridLineVertex);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        gvk::Buffer gridLineBuffer;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &vmaAllocationCreateInfo, &gridLineBuffer));

        VmaAllocationInfo gridLineBufferAllocationInfo{ };
        vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), gridLineBuffer.get<VmaAllocation>(), &gridLineBufferAllocationInfo);
        memcpy(gridLineBufferAllocationInfo.pMappedData, gridLines.data(), gridLines.size() * sizeof(GridLineVertex));

        // Create grid cell buffer (dynamic, updated each frame with occupancy data)
        bufferCreateInfo.size = kGridCellCount * 6 * sizeof(GridCellVertex);  // Max: 6 vertices per cell
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        gvk::Buffer gridCellBuffer;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &vmaAllocationCreateInfo, &gridCellBuffer));
        uint32_t gridCellVertexCount = 0;  // Updated each frame

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

        // Allocate descriptor sets for grid clear pipeline
        gvk::DescriptorPool gridClearDescriptorPool;
        std::vector<gvk::DescriptorSet> gridClearDescriptorSets;
        {
            auto& pipelineLayout = gridClearPipeline.get<gvk::PipelineLayout>();
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
                gvk_result(gvk::DescriptorPool::create(gvkDevice, &poolCreateInfo, nullptr, &gridClearDescriptorPool));

                auto allocInfo = gvk::get_default<VkDescriptorSetAllocateInfo>();
                allocInfo.descriptorPool = gridClearDescriptorPool;
                allocInfo.descriptorSetCount = (uint32_t)setLayouts.size();
                allocInfo.pSetLayouts = setLayouts.data();
                gridClearDescriptorSets.resize(setLayouts.size());
                gvk_result(gvk::DescriptorSet::allocate(gvkDevice, &allocInfo, gridClearDescriptorSets.data()));
            }
        }

        // Bind grid cell buffer to grid clear descriptor set
        if (!gridClearDescriptorSets.empty()) {
            auto descriptorBufferInfo = gvk::get_default<VkDescriptorBufferInfo>();
            descriptorBufferInfo.buffer = gridCellGpuBuffer;
            descriptorBufferInfo.range = VK_WHOLE_SIZE;

            auto writeDescriptorSet = gvk::get_default<VkWriteDescriptorSet>();
            writeDescriptorSet.dstSet = gridClearDescriptorSets[0];
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
            gvkDevice.UpdateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
        }

        // Allocate descriptor sets for binning pipeline
        gvk::DescriptorPool binningDescriptorPool;
        std::vector<gvk::DescriptorSet> binningDescriptorSets;
        {
            auto& pipelineLayout = binningPipeline.get<gvk::PipelineLayout>();
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
                gvk_result(gvk::DescriptorPool::create(gvkDevice, &poolCreateInfo, nullptr, &binningDescriptorPool));

                auto allocInfo = gvk::get_default<VkDescriptorSetAllocateInfo>();
                allocInfo.descriptorPool = binningDescriptorPool;
                allocInfo.descriptorSetCount = (uint32_t)setLayouts.size();
                allocInfo.pSetLayouts = setLayouts.data();
                binningDescriptorSets.resize(setLayouts.size());
                gvk_result(gvk::DescriptorSet::allocate(gvkDevice, &allocInfo, binningDescriptorSets.data()));
            }
        }

        // Bind particle and grid cell buffers to binning descriptor set
        if (!binningDescriptorSets.empty()) {
            std::array<VkDescriptorBufferInfo, 2> descriptorBufferInfos{ };
            descriptorBufferInfos[0].buffer = particleBuffer;
            descriptorBufferInfos[0].range = VK_WHOLE_SIZE;
            descriptorBufferInfos[1].buffer = gridCellGpuBuffer;
            descriptorBufferInfos[1].range = VK_WHOLE_SIZE;

            std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{ };
            writeDescriptorSets[0] = gvk::get_default<VkWriteDescriptorSet>();
            writeDescriptorSets[0].dstSet = binningDescriptorSets[0];
            writeDescriptorSets[0].dstBinding = 0;
            writeDescriptorSets[0].descriptorCount = 1;
            writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[0].pBufferInfo = &descriptorBufferInfos[0];

            writeDescriptorSets[1] = gvk::get_default<VkWriteDescriptorSet>();
            writeDescriptorSets[1].dstSet = binningDescriptorSets[0];
            writeDescriptorSets[1].dstBinding = 1;
            writeDescriptorSets[1].descriptorCount = 1;
            writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[1].pBufferInfo = &descriptorBufferInfos[1];

            gvkDevice.UpdateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
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

                                        // Wake up selected particle immediately
                                        pParticles[i].sleepCounter = 0;
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

                    // Wake up particle when scale changes
                    pParticles[selectedParticleIndex].sleepCounter = 0;
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

                // GPU Binning Pass (Milestone 2): Clear and populate grid cells
                if (!simParams.paused) {
                    // Step 1: Clear grid cell counters
                    gvkCommandBuffer.CmdBindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, gridClearPipeline);
                    if (!gridClearDescriptorSets.empty()) {
                        gvkCommandBuffer.CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, gridClearPipeline.get<gvk::PipelineLayout>(), 0, 1, &gridClearDescriptorSets[0].get<VkDescriptorSet>(), 0, nullptr);
                    }
                    uint32_t gridCellCountConst = kGridCellCount;
                    gvkCommandBuffer.CmdPushConstants(gridClearPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &gridCellCountConst);
                    uint32_t clearWorkGroupCount = (kGridCellCount + 255) / 256;
                    gvkCommandBuffer.CmdDispatch(clearWorkGroupCount, 1, 1);

                    // Barrier: Clear writes -> Binning atomic reads/writes
                    auto barrier = gvk::get_default<VkMemoryBarrier>();
                    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    gvkCommandBuffer.CmdPipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);

                    // Step 2: Bin particles into grid cells
                    gvkCommandBuffer.CmdBindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, binningPipeline);
                    if (!binningDescriptorSets.empty()) {
                        gvkCommandBuffer.CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, binningPipeline.get<gvk::PipelineLayout>(), 0, 1, &binningDescriptorSets[0].get<VkDescriptorSet>(), 0, nullptr);
                    }
                    BinningPushConstants binningPushConstants{ };
                    binningPushConstants.particleCount = simParams.particleCount;
                    binningPushConstants.gridDimX = kGridDimensionX;
                    binningPushConstants.gridDimY = kGridDimensionY;
                    binningPushConstants.gridDimZ = kGridDimensionZ;
                    binningPushConstants.domainMinX = kDomainMinX;
                    binningPushConstants.domainMinY = kDomainMinY;
                    binningPushConstants.domainMinZ = kDomainMinZ;
                    binningPushConstants.gridCellSize = kGridCellSize;
                    gvkCommandBuffer.CmdPushConstants(binningPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(BinningPushConstants), &binningPushConstants);
                    uint32_t binningWorkGroupCount = (simParams.particleCount + kComputeWorkGroupSize - 1) / kComputeWorkGroupSize;
                    gvkCommandBuffer.CmdDispatch(binningWorkGroupCount, 1, 1);

                    // Barrier: Binning writes -> Physics reads
                    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    gvkCommandBuffer.CmdPipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
                }

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
                    computePushConstants.selectedParticleIndex = selectedParticleIndex >= 0 ? (uint32_t)selectedParticleIndex : UINT32_MAX;

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
                        const char* initModes[] = { "Stable Grid", "Uniform Random", "Gaussian Cloud", "Multiple Clusters", "Explosion" };
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
                                // Wake up particle when scale changes
                                pParticles[selectedParticleIndex].sleepCounter = 0;
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

                        // Sleep/Wake statistics
                        VmaAllocationInfo particleBufferAllocInfo{ };
                        vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), particleBuffer.get<VmaAllocation>(), &particleBufferAllocInfo);
                        Particle* pParticles = (Particle*)particleBufferAllocInfo.pMappedData;
                        uint32_t awakeCount = 0;
                        uint32_t asleepCount = 0;
                        for (uint32_t i = 0; i < simParams.particleCount; ++i) {
                            if (pParticles[i].sleepCounter >= kSleepThreshold) {
                                asleepCount++;
                            } else {
                                awakeCount++;
                            }
                        }
                        float awakePercent = (simParams.particleCount > 0) ? (100.0f * awakeCount / simParams.particleCount) : 0.0f;
                        ImGui::Text("Awake: %u (%.1f%%)", awakeCount, awakePercent);
                        ImGui::Text("Asleep: %u (%.1f%%)", asleepCount, 100.0f - awakePercent);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Sleep/Wake system improves performance\nby skipping physics for stationary particles");
                        }

                        ImGui::Spacing();
                        ImGui::SeparatorText("Rendering");
                        ImGui::Checkbox("Hard Edges", &simParams.hardEdges);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Toggle between hard (solid) and soft (gradient) particle edges");
                        }
                        ImGui::Checkbox("Velocity Colors", &simParams.showVelocityColors);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Debug: Color particles by velocity magnitude\nBlue = slow, Green = medium, Red = fast");
                        }
                        ImGui::Checkbox("Show Grid", &simParams.showGrid);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Debug: Visualize spatial grid cells (wireframe)");
                        }
                        ImGui::Checkbox("Show Grid Occupancy", &simParams.showGridOccupancy);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Debug: Show grid cell occupancy heatmap (GPU-accelerated)\nGray = empty, Blue = sparse, Yellow = medium, Red = dense");
                        }

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

                // Update grid cell occupancy buffer (now GPU-accelerated!)
                if (simParams.showGridOccupancy) {
                    // Read GPU grid cell data (already computed by binning pass)
                    VmaAllocationInfo gridCellGpuAllocInfo{ };
                    vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), gridCellGpuBuffer.get<VmaAllocation>(), &gridCellGpuAllocInfo);
                    GridCell* pGridCells = (GridCell*)gridCellGpuAllocInfo.pMappedData;

                    // Generate visualization geometry from GPU-computed cell data
                    auto gridCellVertices = generate_grid_cell_occupancy_from_gpu(pGridCells);
                    gridCellVertexCount = static_cast<uint32_t>(gridCellVertices.size());

                    if (gridCellVertexCount > 0) {
                        VmaAllocationInfo gridCellBufAllocInfo{ };
                        vmaGetAllocationInfo(gvkDevice.get<VmaAllocator>(), gridCellBuffer.get<VmaAllocation>(), &gridCellBufAllocInfo);
                        memcpy(gridCellBufAllocInfo.pMappedData, gridCellVertices.data(), gridCellVertices.size() * sizeof(GridCellVertex));
                    }
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
                // Note: Alignment must match GLSL std140 layout (vec3 aligned to 16 bytes)
                struct GraphicsPushConstants {
                    glm::mat4 viewProjection;      // 64 bytes, aligned to 16
                    alignas(16) glm::vec3 cameraRight;       // 12 bytes + 4 padding = 16 bytes
                    float particleScale;           // 4 bytes (packed after cameraRight)
                    alignas(16) glm::vec3 cameraUp;          // 12 bytes + 4 padding = 16 bytes  
                    uint32_t showCollisionCircles; // 4 bytes (packed after cameraUp)
                    uint32_t selectedParticleIndex; // 4 bytes
                    float selectedParticleScale;   // 4 bytes
                    uint32_t hardEdges;            // 4 bytes
                    uint32_t showVelocityColors;   // 4 bytes
                };
                GraphicsPushConstants graphicsPushConstants{ };
                graphicsPushConstants.viewProjection = camera.projection() * camera.view();
                graphicsPushConstants.cameraRight = camera.transform.right();
                graphicsPushConstants.particleScale = 1.0f;
                graphicsPushConstants.cameraUp = camera.transform.up();
                graphicsPushConstants.showCollisionCircles = 0;
                graphicsPushConstants.selectedParticleIndex = selectedParticleIndex >= 0 ? (uint32_t)selectedParticleIndex : UINT32_MAX;
                graphicsPushConstants.selectedParticleScale = selectedParticleScale;
                graphicsPushConstants.hardEdges = simParams.hardEdges ? 1u : 0u;
                graphicsPushConstants.showVelocityColors = simParams.showVelocityColors ? 1u : 0u;
                gvkCommandBuffer.CmdPushConstants(graphicsPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GraphicsPushConstants), &graphicsPushConstants);

                // Bind vertex buffer (quad vertices)
                VkDeviceSize offset = 0;
                VkBuffer vkQuadVertexBuffer = quadVertexBuffer.get<VkBuffer>();
                gvkCommandBuffer.CmdBindVertexBuffers(0, 1, &vkQuadVertexBuffer, &offset);

                // Draw instanced quads (one instance per particle)
                // 6 vertices = 2 triangles per quad
                gvkCommandBuffer.CmdDraw(6, simParams.particleCount, 0, 0);

                // Draw grid visualization if enabled
                if (simParams.showGrid) {
                    gvkCommandBuffer.CmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, gridVisualizationPipeline);

                    // Push constants: view-projection matrix and grid color
                    GridVisualizationPushConstants gridPushConstants;
                    gridPushConstants.viewProjection = camera.projection() * camera.view();
                    gridPushConstants.gridColor = glm::vec4(0.3f, 0.6f, 0.9f, 0.3f);  // Semi-transparent blue
                    gvkCommandBuffer.CmdPushConstants(
                        gridVisualizationPipeline.get<gvk::PipelineLayout>(),
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0,
                        sizeof(GridVisualizationPushConstants),
                        &gridPushConstants
                    );

                    VkDeviceSize gridOffset = 0;
                    VkBuffer vkGridLineBuffer = gridLineBuffer.get<VkBuffer>();
                    gvkCommandBuffer.CmdBindVertexBuffers(0, 1, &vkGridLineBuffer, &gridOffset);
                    gvkCommandBuffer.CmdDraw(gridLineVertexCount, 1, 0, 0);
                }

                // Draw grid cell occupancy heatmap if enabled
                if (simParams.showGridOccupancy && gridCellVertexCount > 0) {
                    gvkCommandBuffer.CmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, gridCellPipeline);

                    // Push constants: view-projection matrix (color is per-vertex)
                    GridVisualizationPushConstants gridPushConstants;
                    gridPushConstants.viewProjection = camera.projection() * camera.view();
                    gridPushConstants.gridColor = glm::vec4(1.0f);  // Unused
                    gvkCommandBuffer.CmdPushConstants(
                        gridCellPipeline.get<gvk::PipelineLayout>(),
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0,
                        sizeof(GridVisualizationPushConstants),
                        &gridPushConstants
                    );

                    VkDeviceSize cellOffset = 0;
                    VkBuffer vkGridCellBuffer = gridCellBuffer.get<VkBuffer>();
                    gvkCommandBuffer.CmdBindVertexBuffers(0, 1, &vkGridCellBuffer, &cellOffset);
                    gvkCommandBuffer.CmdDraw(gridCellVertexCount, 1, 0, 0);
                }

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
