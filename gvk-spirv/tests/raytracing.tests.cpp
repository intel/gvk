
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

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"
#include "spirv-test-utilities.hpp"

TEST(spirv, RayGen_stage)
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


TEST(spirv, ClosestHit_stage)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
            gvk::spirv::ShaderInfo{
                /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
                /* .stage      = */ VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                /* .lineOffset = */ __LINE__,
                /* .source     = */ R"(
            
                       /* Copyright (c) 2023, Sascha Willems
                         * SPDX-License-Identifier: MIT
                         */
                        #version 460
            
                        #extension GL_EXT_ray_tracing : require
                        #extension GL_EXT_nonuniform_qualifier : require
                        #extension GL_EXT_buffer_reference2 : require
                        #extension GL_EXT_scalar_block_layout : require
                        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
            
                        struct GeometryNode {
                            uint64_t vertexBufferDeviceAddress;
                            uint64_t indexBufferDeviceAddress;
                            int textureIndexBaseColor;
                            int textureIndexOcclusion;
                        };
            
                        layout(location = 0) rayPayloadInEXT vec3 hitValue;
                        layout(location = 3) rayPayloadInEXT uint payloadSeed;
                        layout(binding = 4, set = 0) buffer GeometryNodes { GeometryNode nodes[]; } geometryNodes;
                        layout(binding = 5, set = 0) uniform sampler2D textures[];
            
                        layout(buffer_reference, scalar) buffer Vertices {vec4 v[]; };
                        layout(buffer_reference, scalar) buffer Indices {uint i[]; };
                        layout(buffer_reference, scalar) buffer Data {vec4 f[]; };                  
            
                        void main()
                        {
                            GeometryNode geometryNode = geometryNodes.nodes[gl_GeometryIndexEXT];
                            vec4 color = texture(textures[nonuniformEXT(geometryNode.textureIndexBaseColor)], vec2(0.5));
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
                        /* .binding            = */ 4,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                        /* .pImmutableSamplers = */ nullptr
                    },
                        VkDescriptorSetLayoutBinding{
                        /* .binding            = */ 5,
                        /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        /* .descriptorCount    = */ 1,
                        /* .stageFlags         = */ VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                        /* .pImmutableSamplers = */ nullptr
                    },
                },
            },
        },
        std::vector<VkPushConstantRange>{
        }
    );
}

TEST(spirv, AnyHit_stage)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
        gvk::spirv::ShaderInfo{
            /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
            /* .stage      = */ VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
            /* .lineOffset = */ __LINE__,
            /* .source     = */ R"(
                /* Copyright (c) 2023, Sascha Willems
                 *
                 * SPDX-License-Identifier: MIT
                 *
                 */
                #version 460

                #extension GL_EXT_ray_tracing : require
                #extension GL_EXT_nonuniform_qualifier : require
                #extension GL_EXT_buffer_reference2 : require
                #extension GL_EXT_scalar_block_layout : require
                #extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

                layout(location = 0) rayPayloadInEXT vec3 hitValue;
                layout(location = 3) rayPayloadInEXT uint payloadSeed;

                struct GeometryNode {
                    uint64_t vertexBufferDeviceAddress;
                    uint64_t indexBufferDeviceAddress;
                    int textureIndexBaseColor;
                    int textureIndexOcclusion;
                };
                layout(binding = 4, set = 0) buffer GeometryNodes { GeometryNode nodes[]; } geometryNodes;

                layout(binding = 5, set = 0) uniform sampler2D textures[];

                void main()
                {

                    GeometryNode geometryNode = geometryNodes.nodes[gl_GeometryIndexEXT];
                    vec4 color = texture(textures[nonuniformEXT(geometryNode.textureIndexBaseColor)], vec2(0.5));

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
                    /* .binding            = */ 4,
                    /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    /* .descriptorCount    = */ 1,
                    /* .stageFlags         = */ VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                    /* .pImmutableSamplers = */ nullptr
                },
                    VkDescriptorSetLayoutBinding{
                    /* .binding            = */ 5,
                    /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    /* .descriptorCount    = */ 1,
                    /* .stageFlags         = */ VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                    /* .pImmutableSamplers = */ nullptr
                },
            },
        },
    },
    std::vector<VkPushConstantRange>{
    }
    );
}

TEST(spirv, MissHit_stage)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
        gvk::spirv::ShaderInfo{
            /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
            /* .stage      = */ VK_SHADER_STAGE_MISS_BIT_KHR,
            /* .lineOffset = */ __LINE__,
            /* .source     = */ R"(
                 /* Copyright (c) 2023, Sascha Willems
                 * SPDX-License-Identifier: MIT
                 */

                #version 460
                #extension GL_EXT_ray_tracing : enable

                layout(location = 0) rayPayloadInEXT vec3 hitValue;

                void main()
                {
                    hitValue = vec3(1.0);
                }
                )",
            /* .spirv  = */ { },
            /* .errors = */ { }
        },
    },
    std::vector<std::vector<VkDescriptorSetLayoutBinding>>{
    },
    std::vector<VkPushConstantRange>{
    }
    );
}

TEST(spirv, IntersectionBit_stage)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
        gvk::spirv::ShaderInfo{
            /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
            /* .stage      = */ VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
            /* .lineOffset = */ __LINE__,
            /* .source     = */ R"(
                /* Copyright (c) 2023, Sascha Willems
                 * SPDX-License-Identifier: MIT
                 */

                #version 460
                #extension GL_EXT_ray_tracing : require

                struct Sphere {
                    vec3 center;
                    float radius;
                    vec4 color;
                };
                layout(binding = 3, set = 0) buffer Spheres { Sphere s[]; } spheres;

                void main() {
                    Sphere sphere = spheres.s[gl_PrimitiveID];
                    float hit = sphere.radius;
                    reportIntersectionEXT(hit, 0);
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
                    /* .binding            = */ 3,
                    /* .descriptorType     = */ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    /* .descriptorCount    = */ 1,
                    /* .stageFlags         = */ VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
                    /* .pImmutableSamplers = */ nullptr
                },
            },
        },
    },
    std::vector<VkPushConstantRange>{
    }
    );
}

TEST(spirv, CallableBit_stage)
{
    gvk::validate_pipeline_layout_creation(
        std::vector<gvk::spirv::ShaderInfo>{
        gvk::spirv::ShaderInfo{
            /* .language   = */ gvk::spirv::ShadingLanguage::Glsl,
            /* .stage      = */ VK_SHADER_STAGE_CALLABLE_BIT_KHR,
            /* .lineOffset = */ __LINE__,
            /* .source     = */ R"(
               #version 460 core
                #extension GL_EXT_ray_tracing : enable

                layout(location = 0) callableDataInEXT vec3 outColor;

                void main()
                {
                    outColor = vec3(0.0, 1.0, 0.0);
                }
                )",
            /* .spirv  = */ { },
            /* .errors = */ { }
        },
    },
    std::vector<std::vector<VkDescriptorSetLayoutBinding>>{
    },
    std::vector<VkPushConstantRange>{
    }
    );
}
