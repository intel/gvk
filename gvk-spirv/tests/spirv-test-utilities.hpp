
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

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif

#include "gvk-spirv/context.hpp"
#include "gvk-handles/context.hpp"
#include "gvk-structures/defaults.hpp"
#include "gtest/gtest.h"

namespace gvk {

inline void validate_pipeline_layout_creation(
    std::vector<gvk::spirv::ShaderInfo> shaderInfos,
    const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings,
    const std::vector<VkPushConstantRange>& pushConstantRanges
)
{
    // Create gvk::spirv::Context
    gvk::spirv::Context spirvContext;
    ASSERT_EQ(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext), VK_SUCCESS);

    // Create gvk::spirv::BindingInfo
    auto bindingInfo = gvk::get_default<gvk::spirv::BindingInfo>();
    for (auto& shaderInfo : shaderInfos) {
        if (spirvContext.compile(&shaderInfo) == VK_SUCCESS) {
            bindingInfo.add_shader(shaderInfo);
        } else {
            for (const auto& error : shaderInfo.errors) {
                std::cerr << error << std::endl;
            }
        }
    }

    // Create gvk::Context
    gvk::Context context;
    ASSERT_EQ(gvk::Context::create(&gvk::get_default<gvk::Context::CreateInfo>(), nullptr, &context), VK_SUCCESS);

    // Create gvk::PipelineLayout
    gvk::PipelineLayout pipelineLayout;
    ASSERT_EQ(gvk::spirv::create_pipeline_layout(context.get_devices()[0], bindingInfo, nullptr, &pipelineLayout), VK_SUCCESS);

    // Validate descriptorSetLayoutBindings
    const auto& descriptorSetLayouts = pipelineLayout.get<gvk::DescriptorSetLayouts>();
    ASSERT_EQ(descriptorSetLayoutBindings.size(), descriptorSetLayouts.size());
    for (size_t layout_i = 0; layout_i < descriptorSetLayouts.size(); ++layout_i) {
        const auto& descriptorSetLayoutCreateInfo = descriptorSetLayouts[layout_i].get<VkDescriptorSetLayoutCreateInfo>();
        ASSERT_EQ(descriptorSetLayoutCreateInfo.bindingCount, descriptorSetLayoutBindings[layout_i].size());
        for (size_t binding_i = 0; binding_i < descriptorSetLayoutCreateInfo.bindingCount; ++binding_i) {
            const auto& expected = descriptorSetLayoutBindings[layout_i][binding_i];
            const auto& actual = descriptorSetLayoutCreateInfo.pBindings[binding_i];
            EXPECT_EQ(expected, actual);
        }
    }

    // Validate pushConstantRanges
    const auto& pipelineLayoutCreateInfo = pipelineLayout.get<VkPipelineLayoutCreateInfo>();
    ASSERT_EQ(pipelineLayoutCreateInfo.pushConstantRangeCount, pushConstantRanges.size());
    for (uint32_t i = 0; i < pipelineLayoutCreateInfo.pushConstantRangeCount; ++i) {
        EXPECT_EQ(pushConstantRanges[i], pipelineLayoutCreateInfo.pPushConstantRanges[i]);
    }
}

} // namespace gvk
