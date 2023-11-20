
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

#include "gvk-spirv/context.hpp"
#include "gvk-handles/context.hpp"
#include "gvk-structures/defaults.hpp"
#include "spirv-test-utilities.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

#include <iostream>

TEST(spirv, Context)
{
    // Create a gvk::spirv::Context...
    gvk::spirv::Context spirvContext;
    EXPECT_FALSE(spirvContext);
    ASSERT_EQ(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext), VK_SUCCESS);
    ASSERT_TRUE(spirvContext);

    // Compile a shader from GLSL...
    gvk::spirv::ShaderInfo shaderInfo{
        /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
        /* .stage      = */ VK_SHADER_STAGE_VERTEX_BIT,
        /* .lineOffset = */ __LINE__,
        /* .source     = */ R"(
            #version 450

            out gl_PerVertex
            {
                vec4 gl_Position;
            };

            void main()
            {
                gl_Position = vec4(0, 0, 0, 1);
            }
        )",
        /* .spirv  = */ { },
        /* .errors = */ { }
    };
    EXPECT_EQ(spirvContext.compile(&shaderInfo), VK_SUCCESS);
    EXPECT_FALSE(shaderInfo.spirv.empty());
    EXPECT_TRUE(shaderInfo.errors.empty());

    // Fail to compile a shader from GLSL...
    shaderInfo.source = R"(
        #version 450

        out gl_PerVertex
        {
            vec4 gl_Position;
        };

        void main()
        {
            This shouldn't compile...

            gl_Position = vec4(0, 0, 0, 1);
        }
    )";
    EXPECT_EQ(spirvContext.compile(&shaderInfo), VK_ERROR_UNKNOWN);
    EXPECT_TRUE(shaderInfo.spirv.empty());
    EXPECT_FALSE(shaderInfo.errors.empty());
}

TEST(spirv, BindingInfo_UniformBuffer)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
            gvk::spirv::ShaderInfo{
                /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
                /* .stage      = */ VK_SHADER_STAGE_VERTEX_BIT,
                /* .lineOffset = */ __LINE__,
                /* .source     = */ R"(
                    #version 450

                    layout(set = 0, binding = 0)
                    uniform UniformBuffer
                    {
                        mat4 matrix;
                    } ubo;

                    void main()
                    {
                    }
                )",
                /* .spirv  = */ { },
                /* .errors = */ { }
            },
        },
        std::vector<std::vector<VkDescriptorSetLayoutBinding>>{
            std::vector<VkDescriptorSetLayoutBinding>{
                std::vector<VkDescriptorSetLayoutBinding>{
                    VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 0,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_VERTEX_BIT,
                        /* .pImmutableSamplers = */ nullptr
                    },
                },
            },
        },
        std::vector<VkPushConstantRange>{
        }
    );
}

TEST(spirv, BindingInfo_StorageBuffer)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
            gvk::spirv::ShaderInfo{
                /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
                /* .stage      = */ VK_SHADER_STAGE_VERTEX_BIT,
                /* .lineOffset = */ __LINE__,
                /* .source     = */ R"(
                    #version 450

                    layout(set = 0, binding = 0)
                    buffer StorageBuffer
                    {
                        float data[];
                    } sbo;

                    void main()
                    {
                    }
                )",
                /* .spirv  = */ { },
                /* .errors = */ { }
            },
        },
        std::vector<std::vector<VkDescriptorSetLayoutBinding>>{
            std::vector<VkDescriptorSetLayoutBinding>{
                std::vector<VkDescriptorSetLayoutBinding>{
                    VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 0,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_VERTEX_BIT,
                        /* .pImmutableSamplers = */ nullptr
                    },
                },
            },
        },
        std::vector<VkPushConstantRange>{
        }
    );
}

TEST(spirv, BindingInfo_StorageImage)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
            gvk::spirv::ShaderInfo{
                /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
                /* .stage      = */ VK_SHADER_STAGE_FRAGMENT_BIT,
                /* .lineOffset = */ __LINE__,
                /* .source     = */ R"(
                    #version 450

                    layout(set = 0, binding = 0, rgba32f)
                    uniform readonly image2D image;

                    layout(set = 1, binding = 0)
                    uniform writeonly image2D images[];

                    void main()
                    {
                    }
                )",
                /* .spirv  = */ { },
                /* .errors = */ { }
            },
        },
        std::vector<std::vector<VkDescriptorSetLayoutBinding>>{
            std::vector<VkDescriptorSetLayoutBinding>{
                std::vector<VkDescriptorSetLayoutBinding>{
                    VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 0,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_FRAGMENT_BIT,
                        /* .pImmutableSamplers = */ nullptr
                    },
                },
            },
            std::vector<VkDescriptorSetLayoutBinding>{
                std::vector<VkDescriptorSetLayoutBinding>{
                    VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 0,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_FRAGMENT_BIT,
                        /* .pImmutableSamplers = */ nullptr
                    },
                },
            },
        },
        std::vector<VkPushConstantRange>{
        }
    );
}

TEST(spirv, BindingInfo_CombinedImageSampler)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
            gvk::spirv::ShaderInfo{
                /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
                /* .stage      = */ VK_SHADER_STAGE_FRAGMENT_BIT,
                /* .lineOffset = */ __LINE__,
                /* .source     = */ R"(
                    #version 450

                    layout(set = 0, binding = 0)
                    uniform sampler2D image;

                    layout(set = 0, binding = 1)
                    uniform sampler2DArray imageArray;

                    void main()
                    {
                    }
                )",
                /* .spirv  = */ { },
                /* .errors = */ { }
            },
        },
        std::vector<std::vector<VkDescriptorSetLayoutBinding>>{
            std::vector<VkDescriptorSetLayoutBinding>{
                std::vector<VkDescriptorSetLayoutBinding>{
                    VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 0,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_FRAGMENT_BIT,
                        /* .pImmutableSamplers = */ nullptr
                    },
                    VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 1,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_FRAGMENT_BIT,
                        /* .pImmutableSamplers = */ nullptr
                    },
                },
            },
        },
        std::vector<VkPushConstantRange>{
        }
    );
}

TEST(spirv, BindingInfo_AccelerationStructure)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
            gvk::spirv::ShaderInfo{
                /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
                /* .stage      = */ VK_SHADER_STAGE_RAYGEN_BIT_KHR,
                /* .lineOffset = */ __LINE__,
                /* .source     = */ R"(

                    // FROM : https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_ray_tracing_pipeline.html

                    #version 460 core
                    #extension GL_EXT_ray_tracing : require

                    layout(set = 0, binding = 0, rgba8) uniform image2D image;
                    layout(set = 0, binding = 1) uniform accelerationStructureEXT as;
                    layout(location = 0) rayPayloadEXT float payload;

                    void main()
                    {
                       vec4 col = vec4(0, 0, 0, 1);
                       vec3 origin = vec3(float(gl_LaunchIDEXT.x) / float(gl_LaunchSizeEXT.x), float(gl_LaunchIDEXT.y) / float(gl_LaunchSizeEXT.y), 1.0);
                       vec3 dir = vec3(0.0, 0.0, -1.0);
                       traceRayEXT(as, 0, 0xff, 0, 1, 0, origin, 0.0, dir, 1000.0, 0);
                       col.y = payload;
                       imageStore(image, ivec2(gl_LaunchIDEXT.xy), col);
                    }
                )",
                /* .spirv  = */ { },
                /* .errors = */ { }
            },
        },
        std::vector<std::vector<VkDescriptorSetLayoutBinding>>{
            std::vector<VkDescriptorSetLayoutBinding>{
                std::vector<VkDescriptorSetLayoutBinding>{
                    VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 0,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_RAYGEN_BIT_KHR,
                        /* .pImmutableSamplers = */ nullptr
                    },
                    VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 1,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_RAYGEN_BIT_KHR,
                        /* .pImmutableSamplers = */ nullptr
                    },
                },
            },
        },
        std::vector<VkPushConstantRange>{
        }
    );
}

TEST(spirv, BindingInfo_PushConstants)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
            gvk::spirv::ShaderInfo{
                /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
                /* .stage      = */ VK_SHADER_STAGE_VERTEX_BIT,
                /* .lineOffset = */ __LINE__,
                /* .source     = */ R"(
                    #version 450

                    layout(push_constant)
                    uniform PushConstant {
                        mat4 matrix;
                    } pc;

                    layout(location = 0) out mat4 matrix;

                    void main()
                    {
                        // We need to reference the PushConstant in order for its size to be populated
                        //  when it's reflected...
                        matrix = pc.matrix;
                    }
                )",
                /* .spirv  = */ { },
                /* .errors = */ { }
            },
        },
        std::vector<std::vector<VkDescriptorSetLayoutBinding>>{
        },
        std::vector<VkPushConstantRange>{
            VkPushConstantRange{
                /* .stageFlags = */ VK_SHADER_STAGE_VERTEX_BIT,
                /* .offset     = */ 0,
                /* .size       = */ sizeof(float) * 16,
            }
        }
    );
}
