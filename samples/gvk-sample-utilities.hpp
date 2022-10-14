
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

#include "gvk/math/camera.hpp"
#include "gvk/math/color.hpp"
#include "gvk/math/transform.hpp"
#include "gvk/system/time.hpp"
#include "gvk/context.hpp"
#include "gvk/defaults.hpp"
#include "gvk/format.hpp"
#include "gvk/handles.hpp"
#include "gvk/mesh.hpp"
#include "gvk/render-target.hpp"
#include "gvk/spir-v.hpp"
#include "gvk/structures.hpp"
#include "gvk/to-string.hpp"
#include "gvk-sample-png.hpp"

#include <algorithm>
#include <iostream>

// GvkSampleContext extends gvk::Context.  gvk::Context handles initialization
//  of gvk::Instance, gvk::Device(s)/gvk::Queue(s), gvk::WsiManager (Window
//  System Integration) and several other utility objects.  GvkSampleContext is
//  able to customize creation logic for these objects via virtual calls.
class GvkSampleContext
    : public gvk::Context
{
private:
    static VkBool32 debug_utils_messenger_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        // If gvk::Context is provided a VkDebugUtilsMessengerCreateInfoEXT at creation
        //  time, its pfnUserCallback member can be set to this function (or any other
        //  with a matching signature) and this callback will be called for messages
        //  originating from the Vulkan runtime as configured.
        //  https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerCallbackDataEXT.html
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

public:
    static VkResult create(const char* pApplicationName, GvkSampleContext* pGvkSampleContext)
    {
        // VkApplicationInfo is an optional member of gvk::Context::CreateInfo.
        auto applicationInfo = gvk::get_default<VkApplicationInfo>();
        applicationInfo.pApplicationName = pApplicationName;

        // gvk::sys::Surface::CreateInfo is an optional member of gvk::Context::CreateInfo.
        //  Providing a gvk::sys::Surface::CreateInfo indicates that platform specific
        //  surface extensions should be loaded.
        auto sysSurfaceCreateInfo = gvk::get_default<gvk::sys::Surface::CreateInfo>();

        // VkDebugUtilsMessengerCreateInfoEXT is an optional member of gvk::Context::CreateInfo.
        //  Providing a VkDebugUtilsMessengerCreateInfoEXT indicates that the debug
        //  utils extension should be loaded.
        auto debugUtilsMessengerCreateInfo = gvk::get_default<VkDebugUtilsMessengerCreateInfoEXT>();
        debugUtilsMessengerCreateInfo.messageSeverity =
            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsMessengerCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsMessengerCreateInfo.pfnUserCallback = debug_utils_messenger_callback;

        // Populate the gvk::Context::CreateInfo and call the base implementation.
        auto contextCreateInfo = gvk::get_default<gvk::Context::CreateInfo>();
        contextCreateInfo.pApplicationInfo = &applicationInfo;
        contextCreateInfo.pSysSurfaceCreateInfo = &sysSurfaceCreateInfo;
        contextCreateInfo.pDebugUtilsMessengerCreateInfo = &debugUtilsMessengerCreateInfo;
        return gvk::Context::create(&contextCreateInfo, nullptr, pGvkSampleContext);
    }

protected:
    VkResult create_instance(const VkInstanceCreateInfo* pInstanceCreateInfo, const VkAllocationCallbacks* pAllocator) override
    {
        // If gvk::Context::create_instance() is overriden, you'll receive a
        //  VkInstanceCreateInfo prepared with any layers/extensions required by the
        //  gvk::Context...from there you're free to modify it as necessary...
        assert(pInstanceCreateInfo);
        auto enabledLayerCount = pInstanceCreateInfo->enabledLayerCount;
        auto ppEnabledLayerNames = pInstanceCreateInfo->ppEnabledLayerNames;
        std::vector<const char*> layers(ppEnabledLayerNames, ppEnabledLayerNames + enabledLayerCount);

        // The order of layers matters, if you're trying to debug with both api dump
        //  and validation for instance, ensure that api dump is listed first so that
        //  both layers output matching handle id's...
#if 1
        layers.push_back("VK_LAYER_LUNARG_api_dump");
#endif
        layers.push_back("VK_LAYER_KHRONOS_validation");
        auto instanceCreateInfo = *pInstanceCreateInfo;
        instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
        instanceCreateInfo.ppEnabledLayerNames = layers.data();

        // Call the base implmentation of gvk::Context::create_instance() with the
        //  modified VkInstanceCreateInfo...
        return gvk::Context::create_instance(&instanceCreateInfo, pAllocator);
    }

    VkResult create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, const VkAllocationCallbacks* pAllocator) override
    {
        // If gvk::Context::create_devices() is overriden, you'll receive a
        //  VkDeviceCreateInfo prepared with any layers/extensions required by the
        //  gvk::Context...from there you're free to modify it as necessary.  Unlike
        //  create_instance(), you may opt to populate mDevices directly instead of
        //  calling into the base implemntation.  This is useful if you need to
        //  configure multiple gvk::Devices.
        assert(pDeviceCreateInfo);
        auto enabledFeatures = gvk::get_default<VkPhysicalDeviceFeatures>();
        enabledFeatures.samplerAnisotropy = VK_TRUE;
        auto deviceCreateInfo = *pDeviceCreateInfo;
        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
        return gvk::Context::create_devices(&deviceCreateInfo, pAllocator);
    }

    VkResult create_wsi_manager(const gvk::WsiManager::CreateInfo* pWsiManagerCreateInfo, const VkAllocationCallbacks* pAllocator) override
    {
        // If gvk::Context::create_wsi_manager() is overriden, you'll receive a
        //  gvk::WsiManager::CreateInfo prepared by the gvk::Context...from there
        //  you're free to modify it as necessary.
        assert(pWsiManagerCreateInfo);
        auto wsiManagerCreateInfo = *pWsiManagerCreateInfo;

        // sampleCount is a request.  The max supported sample count that is less than
        //  or equal to the requested value will be selected.
        wsiManagerCreateInfo.sampleCount = VK_SAMPLE_COUNT_64_BIT;

        // depthFormat is a request.  The supported VkFormat with the greatest bit
        //  depth that is less than or equal to the requested VkFormat will be selected.
        wsiManagerCreateInfo.depthFormat = VK_FORMAT_D32_SFLOAT;
        return gvk::Context::create_wsi_manager(&wsiManagerCreateInfo, pAllocator);
    }
};

// Following are the vertex types used in gvk samples.  Each vertex type has a
//  corresponding gvk::get_vertex_description<>() specialization that is used
//  to create VkVertexInputBindingDescriptions when creating pipelines...

struct EmptyVertex
{
};

template <>
inline auto gvk::get_vertex_description<EmptyVertex>(uint32_t binding)
{
    return gvk::get_vertex_input_attribute_descriptions<>(binding);
}

struct VertexPositionColor
{
    glm::vec3 position;
    glm::vec4 color;
};

template <>
inline auto gvk::get_vertex_description<VertexPositionColor>(uint32_t binding)
{
    return gvk::get_vertex_input_attribute_descriptions<
        glm::vec3,
        glm::vec4
    >(binding);
}

struct VertexPositionTexcoord
{
    glm::vec3 position;
    glm::vec2 texcoord;
};

template <>
inline auto gvk::get_vertex_description<VertexPositionTexcoord>(uint32_t binding)
{
    return gvk::get_vertex_input_attribute_descriptions<
        glm::vec3,
        glm::vec2
    >(binding);
}

struct VertexPositionTexcoordColor
{
    glm::vec3 position;
    glm::vec2 texcoord;
    glm::vec4 color;
};

template <>
inline auto gvk::get_vertex_description<VertexPositionTexcoordColor>(uint32_t binding)
{
    return gvk::get_vertex_input_attribute_descriptions<
        glm::vec3,
        glm::vec2,
        glm::vec4
    >(binding);
}

// Following are uniform types used in gvk samples...

struct ObjectUniforms
{
    glm::mat4 world{ };
};

struct CameraUniforms
{
    glm::mat4 view{ };
    glm::mat4 projection{ };
};

struct Uniforms
{
    ObjectUniforms object{ };
    CameraUniforms camera{ };
};

// Following are utility functions used in gvk samples...

inline VkResult gvk_sample_validate_shader_info(const gvk::spirv::ShaderInfo& shaderInfo)
{
    if (!shaderInfo.errors.empty()) {
        for (const auto& error : shaderInfo.errors) {
            std::cerr << error << std::endl;
        }
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

inline VkSampleCountFlagBits gvk_sample_get_render_pass_sample_count(const gvk::RenderPass& renderPass)
{
    assert(renderPass);
    auto sampleCount = VK_SAMPLE_COUNT_1_BIT;
    auto renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo2>();
    for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
        sampleCount = std::max(sampleCount, renderPassCreateInfo.pAttachments[i].samples);
    }
    return sampleCount;
}

inline VkFormat gvk_sample_get_render_pass_depth_format(const gvk::RenderPass& renderPass)
{
    assert(renderPass);
    auto renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo2>();
    for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
        auto format = renderPassCreateInfo.pAttachments[i].format;
        if (gvk::get_image_aspect_flags(format) & VK_IMAGE_ASPECT_DEPTH_BIT) {
            return format;
        }
    }
    return VK_FORMAT_UNDEFINED;
}

template <typename VertexType = EmptyVertex>
inline VkResult gvk_sample_create_pipeline(
    const gvk::RenderPass& renderPass,
    VkCullModeFlagBits cullMode,
    gvk::spirv::ShaderInfo& vertexShaderInfo,
    gvk::spirv::ShaderInfo& fragmentShaderInfo,
    gvk::Pipeline* pPipeline
)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // Create a gvk::spirv::Compiler, compile GLSL to SPIR-V, then validate both
        //  shaders...
        gvk::spirv::Context spirvContext;
        gvk_result(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext));
        gvk_result(spirvContext.compile(&vertexShaderInfo));
        gvk_result(spirvContext.compile(&fragmentShaderInfo));
        auto vsVkResult = gvk_sample_validate_shader_info(vertexShaderInfo);
        auto fsVkResult = gvk_sample_validate_shader_info(fragmentShaderInfo);
        gvk_result(vsVkResult);
        gvk_result(fsVkResult);

        // Create a gvk::ShaderModule for the vertex shader...
        auto vertexShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        vertexShaderModuleCreateInfo.codeSize = vertexShaderInfo.spirv.size() * sizeof(uint32_t);
        vertexShaderModuleCreateInfo.pCode = vertexShaderInfo.spirv.data();
        gvk::ShaderModule vertexShaderModule;
        gvk_result(gvk::ShaderModule::create(renderPass.get<gvk::Device>(), &vertexShaderModuleCreateInfo, nullptr, &vertexShaderModule));
        auto vertexPipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        vertexPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexPipelineShaderStageCreateInfo.module = vertexShaderModule;

        // Create a gvk::ShaderModule for the fragment shader...
        auto fragmentShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        fragmentShaderModuleCreateInfo.codeSize = fragmentShaderInfo.spirv.size() * sizeof(uint32_t);
        fragmentShaderModuleCreateInfo.pCode = fragmentShaderInfo.spirv.data();
        gvk::ShaderModule fragmentShaderModule;
        gvk_result(gvk::ShaderModule::create(renderPass.get<gvk::Device>(), &fragmentShaderModuleCreateInfo, nullptr, &fragmentShaderModule));
        auto fragmentPipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        fragmentPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentPipelineShaderStageCreateInfo.module = fragmentShaderModule;

        // Create an array of VkPipelineShaderStageCreateInfo for the shaders used in
        //  this gvk::Pipeline...
        std::array<VkPipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos{
            vertexPipelineShaderStageCreateInfo,
            fragmentPipelineShaderStageCreateInfo,
        };

        // VkVertexInputBindingDescription describes how the gvk::Pipeline should
        //  traverse vertex buffer data when draw calls are issued...
        // NOTE : gvk::get_vertex_description<VertexType>(0) is used to get an array of
        //  VkVertexInputAttributeDescriptions at binding 0 which indicates that the
        //  array is associated with the 0th element of pVertexBindingDescriptions...
        VkVertexInputBindingDescription vertexInputBindingDescription{ 0, sizeof(VertexType), VK_VERTEX_INPUT_RATE_VERTEX };
        auto vertexInputAttributeDescriptions = gvk::get_vertex_description<VertexType>(0);
        auto pipelineVertexInputStateCreateInfo = gvk::get_default<VkPipelineVertexInputStateCreateInfo>();
        pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions.size();
        pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

        // VkPipelineRasterizationStateCreateInfo describes how rasterization should
        //  occur...this includes parameters for polygon mode, winding order, face
        //  culling, etc...
        auto pipelineRasterizationStateCreateInfo = gvk::get_default<VkPipelineRasterizationStateCreateInfo>();
        pipelineRasterizationStateCreateInfo.cullMode = cullMode;

        // VkPipelineMultisampleStateCreateInfo describes how multi sampling should
        //  occur.  rasterizationSamples should match the sample count of the
        //  gvk::RenderPass objects that will be used with this gvk::Pipeline.
        auto pipelineMultisampleStateCreateInfo = gvk::get_default<VkPipelineMultisampleStateCreateInfo>();
        pipelineMultisampleStateCreateInfo.rasterizationSamples = gvk_sample_get_render_pass_sample_count(renderPass);

        // VkPipelineDepthStencilStateCreateInfo describes how depth should be handled
        //  during fragment shading...
        auto depthTestEnable = gvk_sample_get_render_pass_depth_format(renderPass) != VK_FORMAT_UNDEFINED;
        auto pipelineDepthStencilStateCreateInfo = gvk::get_default<VkPipelineDepthStencilStateCreateInfo>();
        pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
        pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthTestEnable;
        pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;

        // Populate a gvk::spirv::BindingInfo with our gvk::spirv::ShaderInfo objects.
        //  This will run the shader byte code through SPIRV-Cross to reflect the
        //  resource bindings used in the shaders.  When the gvk::spirv::BindingInfo is
        //  passed to create_pipeline_layout() it will be used to create the necessary
        //  gvk::DescriptorSetLayout objects for the gvk::PipelineLayout.
        gvk::spirv::BindingInfo spirvBindingInfo;
        spirvBindingInfo.add_shader(vertexShaderInfo);
        spirvBindingInfo.add_shader(fragmentShaderInfo);
        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::spirv::create_pipeline_layout(renderPass.get<gvk::Device>(), spirvBindingInfo, nullptr, &pipelineLayout));

        // Finally we populate a VkGraphicsPipelineCreateInfo with the components
        //  necessary for this gvk::Pipeline...
        // NOTE : gvk::get_default<VkGraphicsPipelineCreateInfo>() is used to get the
        //  VkGraphicsPipelineCreateInfo that is prepared for this gvk::Pipeline.
        //  gvk::get_default<>() automatically sets sTypes and sensible default values
        //  where appropriate, for gvk::get_default<VkGraphicsPipelineCreateInfo>() the
        //  default values are generally configured to noop so that only portions of
        //  gvk::Pipeline that are purposefully configured will have an effect.
        auto graphicsPipelineCreateInfo = gvk::get_default<VkGraphicsPipelineCreateInfo>();
        graphicsPipelineCreateInfo.stageCount = (uint32_t)pipelineShaderStageCreateInfos.size();
        graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();
        if (pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount) {
            graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
        }
        graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
        graphicsPipelineCreateInfo.layout = pipelineLayout;
        graphicsPipelineCreateInfo.renderPass = renderPass;
        gvk_result(gvk::Pipeline::create(renderPass.get<gvk::Device>(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, pPipeline));
    } gvk_result_scope_end
    return gvkResult;
}

inline VkResult gvk_sample_allocate_descriptor_sets(const gvk::Pipeline& pipeline, std::vector<gvk::DescriptorSet>* pDescriptorSets)
{
    assert(pipeline);
    assert(pDescriptorSets);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // Use the provided gvk::Pipeline's gvk::PipelineLayout to determine what types
        //  and how many descriptors we'll need...the samples generally allocate a very
        //  limited number of descriptors so this works fine...in real world scenario
        //  you'd likely employ a much more robust strategy for managing descriptors.
        //  Startegies for managing descriptors are highly application dependent...
        std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
        std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
        for (const auto& descriptorSetLayout : pipeline.get<gvk::PipelineLayout>().get<gvk::DescriptorSetLayouts>()) {
            vkDescriptorSetLayouts.push_back(descriptorSetLayout);
            auto descriptorSetLayoutCreateInfo = descriptorSetLayout.get<VkDescriptorSetLayoutCreateInfo>();
            for (uint32_t i = 0; i < descriptorSetLayoutCreateInfo.bindingCount; ++i) {
                const auto& descriptorSetLayoutBinding = descriptorSetLayoutCreateInfo.pBindings[i];
                descriptorPoolSizes.push_back({
                    /* .type            = */ descriptorSetLayoutBinding.descriptorType,
                    /* .descriptorCount = */ descriptorSetLayoutBinding.descriptorCount
                });
            }
        }

        // Create a gvk::DescriptorPool...
        auto descriptorPoolCreateInfo = gvk::get_default<VkDescriptorPoolCreateInfo>();
        descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        descriptorPoolCreateInfo.maxSets = (uint32_t)vkDescriptorSetLayouts.size();
        descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        gvk::DescriptorPool descriptorPool;
        gvk_result(gvk::DescriptorPool::create(pipeline.get<gvk::Device>(), &descriptorPoolCreateInfo, nullptr, &descriptorPool));

        // And allocate gvk::DescriptorSets...
        // NOTE : The allocated gvk::DescriptorSets will hold references to the
        //  gvk::DescriptorPool so there's no need for user code to maintain an
        //  explicit reference.  A gvk::DescriptorSet's gvk::DescriptorPool can be
        //  retrieved using descriptorSet.get<gvk::DescriptorPool>().
        // NOTE : vkResetDescriptorPool() must not be used with gvk::DescriptorSets.
        // NOTE : A gvk::DescriptorPool may be used to allocate VkDescriptorSets and
        //  use vkResetDescriptorPool() as normal.
        auto descriptorSetAllocateInfo = gvk::get_default<VkDescriptorSetAllocateInfo>();
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)vkDescriptorSetLayouts.size();
        descriptorSetAllocateInfo.pSetLayouts = vkDescriptorSetLayouts.data();
        pDescriptorSets->resize(vkDescriptorSetLayouts.size());
        gvk_result(gvk::DescriptorSet::allocate(pipeline.get<gvk::Device>(), &descriptorSetAllocateInfo, pDescriptorSets->data()));
    } gvk_result_scope_end
    return gvkResult;
}

template <typename UniformBufferObjectType>
inline VkResult gvk_sample_create_uniform_buffer(const gvk::Context& context, gvk::Buffer* pUniformBuffer)
{
    assert(pUniformBuffer);
    // Creates a persistently mapped gvk::Buffer for writing uniform buffer data...
    auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
    bufferCreateInfo.size = sizeof(UniformBufferObjectType);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VmaAllocationCreateInfo vmaAllocationCreateInfo{ };
    vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    return gvk::Buffer::create(context.get_devices()[0], &bufferCreateInfo, &vmaAllocationCreateInfo, pUniformBuffer);
}

struct GvkSampleRenderTargetCreateInfo
{
    VkExtent2D extent{ };
    VkFormat colorFormat{ VK_FORMAT_UNDEFINED };
    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
    VkSampleCountFlagBits sampleCount{ VK_SAMPLE_COUNT_1_BIT };
};

inline VkResult gvk_sample_create_render_target(const gvk::Context& context, GvkSampleRenderTargetCreateInfo createInfo, gvk::RenderTarget* pRenderTarget)
{
    assert(pRenderTarget);
    assert(createInfo.colorFormat);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // If we have a depthFormat, we need to check what VkFormats are supported
        //  and find the VkFormat the best matches the request.  This is similar to
        //  the VkFormat selection algorithm used by gvk::WsiManager...it will select
        //  the supported VkFormat with the highest bit depth that is less than or
        //  equal to the requested VkFormat.
        if (createInfo.depthFormat) {
            auto requestedDepthFormat = createInfo.depthFormat;
            auto requestedDepthBits = gvk::get_format_info(requestedDepthFormat).components[0].bits;
            enumerate_formats(
                context.get_devices()[0].get<gvk::PhysicalDevice>(),
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT,
                [&](VkFormat format)
                {
                    if (format == requestedDepthFormat) {
                        createInfo.depthFormat = format;
                    }
                    if (createInfo.depthFormat != requestedDepthFormat) {
                        auto actualDepthBits = createInfo.depthFormat ? gvk::get_format_info(createInfo.depthFormat).components[0].bits : 0;
                        auto formatDepthBits = gvk::get_format_info(format).components[0].bits;
                        if (actualDepthBits < formatDepthBits && formatDepthBits <= requestedDepthBits) {
                            createInfo.depthFormat = format;
                        }
                    }
                    return createInfo.depthFormat != requestedDepthFormat;
                }
            );
        }

        // We do a similar validation for sampleCount...
        if (VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount) {
            const auto& physicalDevice = context.get_devices()[0].get<gvk::PhysicalDevice>();
            auto maxSampleCount = gvk::get_max_framebuffer_sample_count(physicalDevice, VK_TRUE, createInfo.depthFormat != VK_FORMAT_UNDEFINED, VK_FALSE);
            createInfo.sampleCount = std::min(createInfo.sampleCount, maxSampleCount);
        }

        // Setup the MSAA attachment description...
        auto msaaAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
        msaaAttachmentDescription.format = createInfo.colorFormat;
        msaaAttachmentDescription.samples = createInfo.sampleCount;
        msaaAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        msaaAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        msaaAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        auto msaaAttachmentReference = gvk::get_default<VkAttachmentReference2>();
        msaaAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        msaaAttachmentReference.aspectMask = gvk::get_image_aspect_flags(createInfo.colorFormat);

        // Setup the color attachment description...
        auto colorAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
        colorAttachmentDescription.format = msaaAttachmentDescription.format;
        colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        auto colorAttachmentReference = msaaAttachmentReference;

        // Setup the depth attachment description...
        auto depthAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
        depthAttachmentDescription.format = createInfo.depthFormat;
        depthAttachmentDescription.samples = createInfo.sampleCount;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        auto depthAttachmentReference = gvk::get_default<VkAttachmentReference2>();
        depthAttachmentReference.layout = depthAttachmentDescription.finalLayout;
        depthAttachmentReference.aspectMask = gvk::get_image_aspect_flags(createInfo.depthFormat);

        // Setup VkAttachmentDescription2 and VkAttachmentReference2 objects.  The
        //  indices that each attachment refers to will depend on what attachments
        //  we have...
        uint32_t attachmentCount = 1;
        std::array<VkAttachmentDescription2, 3> attachmentDescriptions{
            msaaAttachmentDescription,
            colorAttachmentDescription,
            depthAttachmentDescription,
        };
        auto pAttachmentDescriptions = &attachmentDescriptions[1];
        if (VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount) {
            pAttachmentDescriptions = &attachmentDescriptions[0];
            colorAttachmentReference.attachment = 1;
            ++attachmentCount;
        }
        if (createInfo.depthFormat) {
            depthAttachmentReference.attachment = colorAttachmentReference.attachment + 1;
            ++attachmentCount;
        }

        // Setup VkSubpassDescription2...
        auto subpassDescription = gvk::get_default<VkSubpassDescription2>();
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount ? &msaaAttachmentReference : &colorAttachmentReference;
        subpassDescription.pResolveAttachments = VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount ? &colorAttachmentReference : nullptr;
        subpassDescription.pDepthStencilAttachment = depthAttachmentDescription.format ? &depthAttachmentReference : nullptr;

        // Create gvk::RenderPass...
        auto renderPassCreateInfo = gvk::get_default<VkRenderPassCreateInfo2>();
        renderPassCreateInfo.attachmentCount = attachmentCount;
        renderPassCreateInfo.pAttachments = pAttachmentDescriptions;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        gvk::RenderPass renderPass;
        gvk_result(gvk::RenderPass::create(context.get_devices()[0], &renderPassCreateInfo, nullptr, &renderPass));

        // Prepare VkFramebufferCreateInfo.  Any attachments that aren't proivded via
        //  the VkFramebufferCreateInfo pAttachments member will be automatically
        //  created by gvk::RenderTarget.  We're not creating any here explicitly, so
        //  all of the attachments are created by the gvk::RenderTarget implicitly...
        auto framebufferCreateInfo = gvk::get_default<VkFramebufferCreateInfo>();
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.width = createInfo.extent.width;
        framebufferCreateInfo.height = createInfo.extent.height;

        // Create gvk::RenderTarget...
        auto renderTargetCreateInfo = gvk::get_default<gvk::RenderTarget::CreateInfo>();
        renderTargetCreateInfo.pFramebufferCreateInfo = &framebufferCreateInfo;
        gvk_result(gvk::RenderTarget::create(context.get_devices()[0], &renderTargetCreateInfo, nullptr, pRenderTarget));

        // Transition the gvk::RenderTarget object's gvk::Image objects to the correct
        //  VkImageLayouts.  gvk::RenderTarget::get_image_memory_barrier() returns a
        //  a VkImageMemoryBarrier prepared to transition the gvk::Image at the given
        //  index from its finalLayout to its initialLayout.  This is useful for
        //  reverting a gvk::Image to the layout the gvk::RenderTarget expects it to be
        //  in before execution.  If the gvk::Image object's VkImageLayout has been
        //  changed by something besides the associated gvk::RenderPass, your
        //  application must keep track of this.
        gvk::execute_immediately(
            gvk::get_queue_family(context.get_devices()[0], 0).queues[0],
            context.get_command_buffers()[0],
            VK_NULL_HANDLE,
            [&](const gvk::CommandBuffer& commandBuffer)
            {
                auto attachmentCount = pRenderTarget->get_framebuffer().get<gvk::ImageViews>().size();
                for (size_t i = 0; i < attachmentCount; ++i) {
                    auto imageMemoryBarrier = pRenderTarget->get_image_memory_barrier((uint32_t)i);
                    if (imageMemoryBarrier.oldLayout) {
                        imageMemoryBarrier.newLayout = imageMemoryBarrier.oldLayout;
                        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        vkCmdPipelineBarrier(
                            commandBuffer,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            0,
                            0, nullptr,
                            0, nullptr,
                            1, &imageMemoryBarrier
                        );
                    }
                }
            }
        );
    } gvk_result_scope_end
    return gvkResult;
}

VkResult gvk_sample_acquire_submit_present(GvkSampleContext& context)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        const auto& device = context.get_devices()[0];
        const auto& queue = gvk::get_queue_family(device, 0).queues[0];
        auto& wsiManager = context.get_wsi_manager();
        if (wsiManager.is_enabled()) {
            // If the gvk::WsiManager is enabled, we need to acquire the next gvk::Image to
            //  render to...this method may return VK_SUBOPTIMAL_KHR...the gvk::WsiManager
            //  will update itself when this occurs, so we don't want to bail from the
            //  gvk_result_scope when this happens...
            uint32_t imageIndex = 0;
            auto vkResult = wsiManager.acquire_next_image(UINT64_MAX, VK_NULL_HANDLE, &imageIndex);
            gvk_result((vkResult == VK_SUCCESS || vkResult == VK_SUBOPTIMAL_KHR) ? VK_SUCCESS : vkResult);

            // Once we have the gvk::Image acquired, we need to make sure that we wait on
            //  the associated gvk::Fence...this ensures that we're not trying to reuse the
            //  gvk::Image while it's in flight...
            const auto& fence = wsiManager.get_fences()[imageIndex];
            gvk_result(vkWaitForFences(device, 1, &(const VkFence&)fence, VK_TRUE, UINT64_MAX));

            // Reset the gvk::Fence because we're going to use it again right away...
            gvk_result(vkResetFences(device, 1, &(const VkFence&)fence));

            // Submit...
            //  When this submission finishes, the associated gvk::Fence will be signaled
            //  so we know this gvk::Image is ready to be used again...
            auto submitInfo = wsiManager.get_submit_info(imageIndex);
            gvk_result(vkQueueSubmit(queue, 1, &submitInfo, fence));

            // Present...
            // Like acquire_next_image(), VK_SUBOPTIMAL_KHR is ok and will be handled by
            //  the gvk::WsiManager...
            auto presentInfo = wsiManager.get_present_info(&imageIndex);
            vkResult = vkQueuePresentKHR(queue, &presentInfo);
            gvk_result((vkResult == VK_SUCCESS || vkResult == VK_SUBOPTIMAL_KHR) ? VK_SUCCESS : vkResult);
        }
    } gvk_result_scope_end
    return gvkResult;
}
