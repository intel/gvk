
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

#include "gvk-spirv/shader-group-handle-map.hpp"
#include "gvk-spirv/context.hpp"

#include <algorithm>
#include <map>
#include <vector>

namespace gvk {

VkResult create_shader_group_handle_map(const gvk::Device& gvkDevice, const gvk::ShaderGroupHandleMapCreateInfo* pShaderGroupHandleMapCreateInfo, gvk::ShaderGroupHandleMap* pShaderGroupHandleMap)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pShaderGroupHandleMapCreateInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result((pShaderGroupHandleMapCreateInfo->keysPipeline || pShaderGroupHandleMapCreateInfo->pShaderGroupHandleKeys) ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result((pShaderGroupHandleMapCreateInfo->valuesPipeline || pShaderGroupHandleMapCreateInfo->pShaderGroupHandleValues) ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pShaderGroupHandleMapCreateInfo->shaderGroupHandleCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pShaderGroupHandleMap ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        *pShaderGroupHandleMap = gvk::get_default<gvk::ShaderGroupHandleMap>();

        // Setup individual variables for convenience
        auto keysPipeline = pShaderGroupHandleMapCreateInfo->keysPipeline;
        auto pKeysData = pShaderGroupHandleMapCreateInfo->pShaderGroupHandleKeys;
        auto valuesPipeline = pShaderGroupHandleMapCreateInfo->valuesPipeline;
        auto pValuesData = pShaderGroupHandleMapCreateInfo->pShaderGroupHandleValues;
        auto shaderGroupHandleCount = pShaderGroupHandleMapCreateInfo->shaderGroupHandleCount;

        // Get shader group handle size
        auto physicalDeviceRayTracingProperties = gvk::get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
        auto physicalDeviceProperties2 = gvk::get_default<VkPhysicalDeviceProperties2>();
        physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
        gvkDevice.get<gvk::PhysicalDevice>().GetPhysicalDeviceProperties2(&physicalDeviceProperties2);
        auto shaderGroupHandleSize = physicalDeviceRayTracingProperties.shaderGroupHandleSize;

        // If pKeysData isn't provided, populate it using keysPipeline
        std::vector<uint8_t> shaderGroupHandleKeysStorage;
        if (!pKeysData) {
            shaderGroupHandleKeysStorage.resize(shaderGroupHandleCount * shaderGroupHandleSize);
            gvk_result(gvkDevice.GetRayTracingShaderGroupHandlesKHR(keysPipeline, 0, shaderGroupHandleCount, shaderGroupHandleKeysStorage.size(), shaderGroupHandleKeysStorage.data()));
            pKeysData = shaderGroupHandleKeysStorage.data();
        }

        // If pKeysData isn't provided, populate it using valuesPipeline
        std::vector<uint8_t> shaderGroupHandleValuesStorage;
        if (!pValuesData) {
            shaderGroupHandleValuesStorage.resize(shaderGroupHandleCount * shaderGroupHandleSize);
            gvk_result(gvkDevice.GetRayTracingShaderGroupHandlesKHR(valuesPipeline, 0, shaderGroupHandleCount, shaderGroupHandleValuesStorage.size(), shaderGroupHandleValuesStorage.data()));
            pValuesData = shaderGroupHandleValuesStorage.data();
        }

        // Create a std::map<> of pointers to key/value pairs with a custom comparator
        //  to sort by shaderGroupHandleSize bytes pointed to by each key
        auto comparator = [shaderGroupHandleSize](const uint8_t* pLhs, const uint8_t* pRhs)
        {
            return std::lexicographical_compare(pLhs, pLhs + shaderGroupHandleSize, pRhs, pRhs + shaderGroupHandleSize);
        };
        std::map<const uint8_t*, const uint8_t*, decltype(comparator)> keyValuePairs(comparator);
        for (uint32_t group_i = 0; group_i < shaderGroupHandleCount; ++group_i) {
            keyValuePairs.insert({ pKeysData, pValuesData });
            pKeysData += shaderGroupHandleSize;
            pValuesData += shaderGroupHandleSize;
        }

        // Allocate a buffer to hold all of the key/value pairs
        // TODO : Maybe move these to a device local buffer
        auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = shaderGroupHandleCount * shaderGroupHandleSize * 2;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        auto allocationCreateInfo = gvk::get_default<VmaAllocationCreateInfo>();
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &allocationCreateInfo, &pShaderGroupHandleMap->gvkBuffer));

        // Map the buffer
        uint8_t* pMappedKeysData = nullptr;
        gvk_result(vmaMapMemory(gvkDevice.get<VmaAllocator>(), pShaderGroupHandleMap->gvkBuffer.get<VmaAllocation>(), (void**)&pMappedKeysData));
        auto pMappedValuesData = pMappedKeysData + shaderGroupHandleCount * shaderGroupHandleSize;

        // Write the key/value pairs
        for (const auto& itr : keyValuePairs) {
            memcpy(pMappedKeysData, itr.first, shaderGroupHandleSize);
            memcpy(pMappedValuesData, itr.second, shaderGroupHandleSize);
            pMappedKeysData += shaderGroupHandleSize;
            pMappedValuesData += shaderGroupHandleSize;
        }

        // Unmap the buffer
        vmaUnmapMemory(gvkDevice.get<VmaAllocator>(), pShaderGroupHandleMap->gvkBuffer.get<VmaAllocation>());

        // Populate the ShaderGroupHandleMap
        auto bufferDeviceAddressInfo = gvk::get_default<VkBufferDeviceAddressInfo>();
        bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bufferDeviceAddressInfo.buffer = pShaderGroupHandleMap->gvkBuffer;
        pShaderGroupHandleMap->keys = gvkDevice.GetBufferDeviceAddressKHR(&bufferDeviceAddressInfo);
        pShaderGroupHandleMap->values = pShaderGroupHandleMap->keys + shaderGroupHandleCount * shaderGroupHandleSize;
        pShaderGroupHandleMap->kvpCount = shaderGroupHandleCount;
        pShaderGroupHandleMap->handleSize = shaderGroupHandleSize;
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult create_shader_group_handle_map_pipeline(const gvk::Device& gvkDevice, gvk::Pipeline* pShaderGroupHandleMapPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pShaderGroupHandleMapPipeline ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        pShaderGroupHandleMapPipeline->reset();

        // Get shader group handle size
        auto physicalDeviceRayTracingProperties = gvk::get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
        auto physicalDeviceProperties2 = gvk::get_default<VkPhysicalDeviceProperties2>();
        physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
        gvkDevice.get<gvk::PhysicalDevice>().GetPhysicalDeviceProperties2(&physicalDeviceProperties2);
        auto shaderGroupHandleSize = physicalDeviceRayTracingProperties.shaderGroupHandleSize;

        // Compile GLSL to SPIR-V
        gvk::spirv::Context spirvContext;
        gvk_result(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext));
        auto shaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        shaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderInfo.lineOffset = __LINE__;
        shaderInfo.source = R"(

            #version 450

            #extension GL_EXT_buffer_reference : require
            #extension GL_EXT_scalar_block_layout : require
            #extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
            #extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

            #define VkDeviceAddress uint64_t
            #define VkDeviceSize uint64_t

            #if 0
            // NOTE : 'initializer' : can't use with types containing arrays sized with a specialization constant
            // FROM : https://vulkan.lunarg.com/doc/view/1.4.313.2/windows/antora/glsl/latest/chapters/variables.html
            //  Types containing arrays sized with a specialization constant cannot be compared, assigned as aggregates,
            //  declared with an initializer, or used as an initializer.
            layout(constant_id = 0) const int SHADER_GROUP_HANDLE_SIZE = 32;
            #endif

            struct ShaderGroupHandle
            {
                uint8_t data[SHADER_GROUP_HANDLE_SIZE];
            };

            // TODO : Query GPU to create shader with ideal workgroup size
            layout(local_size_x = 16) in;
            layout(buffer_reference, scalar) writeonly buffer Dst { uint8_t data[]; };
            layout(buffer_reference, scalar) readonly buffer Src { uint8_t data[]; };
            layout(buffer_reference, scalar) readonly buffer Keys { ShaderGroupHandle data[]; };
            layout(buffer_reference, scalar) readonly buffer Values { ShaderGroupHandle data[]; };

            layout(push_constant, scalar) uniform PushConstants {
                VkDeviceAddress dst;
                VkDeviceAddress src;
                VkDeviceSize stride;
                VkDeviceSize count;
                VkDeviceAddress keys;
                VkDeviceAddress values;
                VkDeviceSize kvpCount;
            } pc;

            bool less(ShaderGroupHandle lhs, ShaderGroupHandle rhs)
            {
                for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                    if (lhs.data[i] < rhs.data[i]) {
                        return true;
                    } else if (rhs.data[i] < lhs.data[i]) {
                        return false;
                    }
                }
                return false;
            }

            bool equal(ShaderGroupHandle lhs, ShaderGroupHandle rhs)
            {
                for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                    if (lhs.data[i] != rhs.data[i]) {
                        return false;
                    }
                }
                return true;
            }

            ShaderGroupHandle get_shader_group_handle_map_value(ShaderGroupHandle key)
            {
                #if 1
                // Binary search to find the mapped value
                uint left = 0;
                uint right = uint(pc.kvpCount);
                while (left < right) {
                    uint mid = left + (right - left) / 2;
                    ShaderGroupHandle midKey = Keys(pc.keys).data[mid];
                    if (less(midKey, key)) {
                        left = mid + 1;
                    } else if (less(key, midKey)) {
                        right = mid;
                    } else {
                        return Values(pc.values).data[mid];
                    }
                }
                #else
                // Linear search, useful for debugging
                for (uint i = 0; i < uint(pc.kvpCount); ++i) {
                    ShaderGroupHandle shaderGroupHandle = Keys(pc.keys).data[i];
                    if (equal(key, shaderGroupHandle)) {
                        return Values(pc.values).data[i];
                    }
                }
                #endif

                #if 1
                // If the key isn't found, then something went wrong CPU side and there's not
                //  really anything that can be done to recover from here...just return the
                //  given handle.
                return key;
                #else
                // Return null ShaderGroupHandle, useful for debugging
                ShaderGroupHandle nullShaderGroupHandle;
                for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                    nullShaderGroupHandle.data[i] = 0;
                }
                return nullShaderGroupHandle;
                #endif
            }

            void main()
            {
                if (gl_GlobalInvocationID.x < pc.count) {

                    // Get the offset for the current entry
                    uint offset = gl_GlobalInvocationID.x * uint(pc.stride);

                    // Read the shader group handle at the beginning of the current Src entry
                    ShaderGroupHandle key;
                    for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                        key.data[i] = Src(pc.src).data[offset + i];
                    }

                    // Get the mapped value
                    ShaderGroupHandle value = get_shader_group_handle_map_value(key);

                    // Write the shader group handle to the beginning of the current Dst entry
                    for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                        Dst(pc.dst).data[offset + i] = value.data[i];
                    }
                }
            }

        )";
        // NOTE : 'initializer' : can't use with types containing arrays sized with a specialization constant
        // FROM : https://vulkan.lunarg.com/doc/view/1.4.313.2/windows/antora/glsl/latest/chapters/variables.html
        //  Types containing arrays sized with a specialization constant cannot be compared, assigned as aggregates,
        //  declared with an initializer, or used as an initializer.
        // NOTE : This comment is duplicated in the shader source
        shaderInfo.source = gvk::string::replace(shaderInfo.source, "SHADER_GROUP_HANDLE_SIZE", std::to_string(shaderGroupHandleSize));
        gvk_result(spirvContext.compile(&shaderInfo));

        // Create shader module
        auto shaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        shaderModuleCreateInfo.codeSize = shaderInfo.bytecode.size() * sizeof(uint32_t);
        shaderModuleCreateInfo.pCode = !shaderInfo.bytecode.empty() ? shaderInfo.bytecode.data() : nullptr;
        gvk::ShaderModule gvkShaderModule;
        gvk_result(gvk::ShaderModule::create(gvkDevice, &shaderModuleCreateInfo, nullptr, &gvkShaderModule));

        // Create pipeline layout
        gvk::spirv::BindingInfo spirvBindingInfo;
        spirvBindingInfo.add_shader(shaderInfo);
        gvk::PipelineLayout gvkPipelineLayout;
        gvk_result(gvk::spirv::create_pipeline_layout(gvkDevice, spirvBindingInfo, nullptr, &gvkPipelineLayout));

        // Create pipeline
        auto computePipelineCreateInfo = gvk::get_default<VkComputePipelineCreateInfo>();
        computePipelineCreateInfo.stage.module = gvkShaderModule;
        computePipelineCreateInfo.layout = gvkPipelineLayout;
        gvk_result(gvk::Pipeline::create(gvkDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, pShaderGroupHandleMapPipeline));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult execute_shader_group_handle_map(const gvk::Device& gvkDevice, VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, VkFence vkFence, const GpuAddressMapInfo* pGpuAddressMapInfo, const gvk::Pipeline& gpuAddressMapPipeline)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(vkQueue ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(vkCommandBuffer ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pGpuAddressMapInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(gpuAddressMapPipeline ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(gvk::execute_immediately(gvkDevice, vkQueue, vkCommandBuffer, vkFence,
            [&](auto)
            {
                gvkDevice.get<DispatchTable>().gvkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gpuAddressMapPipeline);
                gvkDevice.get<DispatchTable>().gvkCmdPushConstants(vkCommandBuffer, gpuAddressMapPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GpuAddressMapInfo), pGpuAddressMapInfo);
                // TODO : Query GPU to create shader with ideal workgroup size
                static const uint32_t local_size_x = 16;
                auto workgroupSize = (uint32_t)((pGpuAddressMapInfo->count + local_size_x - 1) / (uint64_t)local_size_x);
                gvkDevice.get<DispatchTable>().gvkCmdDispatch(vkCommandBuffer, workgroupSize, 1, 1);
            }
        ));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk
