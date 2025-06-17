
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

#include "gvk-pipeline-explorer/cmd-tracker.hpp"
#include "gvk-pipeline-explorer/device-address-tracker.hpp"
#include "gvk-pipeline-explorer/utilities.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-spirv/gpu-memcpy.hpp"
#include "gvk-spirv/shader-group-handle-map.hpp"
#include "gvk-defines.hpp"
#include "gvk-environment.hpp"
#include "gvk-handles.hpp"
#include "gvk-reference.hpp"
#include "gvk-structures.hpp"

#include "boost/multiprecision/integer.hpp"

#include "vulkan/vk_layer.h"

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace gvk {
namespace pipeline_explorer {

template<typename HandleIdType>
class HandleInfo final
{
public:
    class ControlBlock;
    gvk::Reference<ControlBlock, HandleIdType> reference;
    using HandleId = HandleIdType;
    HandleInfo() = default;
    inline HandleInfo(std::nullptr_t) { };
    inline HandleInfo(gvk::nullref_t) { };
    inline HandleInfo(gvk::newref_t, const HandleIdType& handleId) { reference.reset(gvk::newref, handleId); }
    inline HandleInfo(const HandleIdType& handleId) { reference = gvk::Reference<ControlBlock, HandleIdType>::get(handleId); }
    HandleInfo(const HandleInfo&) = default;
    HandleInfo& operator=(const HandleInfo&) = default;
    HandleInfo(HandleInfo&&) = default;
    HandleInfo& operator=(HandleInfo&&) = default;
    inline operator bool() const { return reference; }
    inline ControlBlock& operator*() { return reference.get_obj(); }
    inline ControlBlock* operator->() { return &reference.get_obj(); }
    inline const ControlBlock& operator*() const { return reference.get_obj(); }
    inline const ControlBlock* operator->() const { return &reference.get_obj(); }
    inline friend bool operator==(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference == rhs.reference; }
    inline friend bool operator!=(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference != rhs.reference; }
    inline friend bool operator<(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference < rhs.reference; }
    inline friend bool operator>(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference > rhs.reference; }
    inline friend bool operator<=(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference <= rhs.reference; }
    inline friend bool operator>=(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference >= rhs.reference; }
};

template<typename VkHandleType>
class BasicControlBlock
{
public:
    UUID uuid{ };
    VkHandleType vkHandle{ };
    BasicControlBlock() = default;
    virtual ~BasicControlBlock() = 0;
    BasicControlBlock(BasicControlBlock const&) = delete;
    BasicControlBlock& operator=(BasicControlBlock const&) = delete;
};

template<typename VkHandleType>
BasicControlBlock<VkHandleType>::~BasicControlBlock()
{
}

// Dispatchable handles
using InstanceInfo = HandleInfo<VkInstance>;
using DeviceInfo = HandleInfo<VkDevice>;
using PhysicalDeviceInfo = HandleInfo<VkPhysicalDevice>;
using QueueInfo = HandleInfo<VkQueue>;
using CommandBufferInfo = HandleInfo<VkCommandBuffer>;

// Nondispatchable handles
using CommandPoolInfo = HandleInfo<gvk::HandleId<VkDevice, VkCommandPool>>;
using DescriptorSetLayoutInfo = HandleInfo<gvk::HandleId<VkDevice, VkDescriptorSetLayout>>;
using PipelineInfo = HandleInfo<gvk::HandleId<VkDevice, VkPipeline>>;
using PipelineLayoutInfo = HandleInfo<gvk::HandleId<VkDevice, VkPipelineLayout>>;
using RenderPassInfo = HandleInfo<gvk::HandleId<VkDevice, VkRenderPass>>;
using SamplerInfo = HandleInfo<gvk::HandleId<VkDevice, VkSampler>>;
using ShaderModuleInfo = HandleInfo<gvk::HandleId<VkDevice, VkShaderModule>>;
using BufferInfo = HandleInfo<gvk::HandleId<VkDevice, VkBuffer>>;
using DeviceMemoryInfo = HandleInfo<gvk::HandleId<VkDevice, VkDeviceMemory>>;

template<>
class InstanceInfo::ControlBlock final
    : public BasicControlBlock<VkInstance>
{
public:
    gvk::Auto<VkInstanceCreateInfo> instanceCreateInfo;
    std::vector<PhysicalDeviceInfo> physicalDevices;
    bool VK_EXT_debug_utils_enabled{ };
    bool VK_KHR_get_physical_device_properties2_enabled{ };
};

template<>
class PhysicalDeviceInfo::ControlBlock final
    : public BasicControlBlock<VkPhysicalDevice>
{
public:
    InstanceInfo instanceInfo;
    gvk::Auto<VkPhysicalDeviceProperties> physicalDeviceProperties;
    gvk::Auto<VkPhysicalDeviceMemoryProperties> physicalDeviceMemoryProperties;
    gvk::Auto<VkPhysicalDeviceRayTracingPipelinePropertiesKHR> physicalDeviceRayTracingPipelineProperties;
};

template<>
class DeviceInfo::ControlBlock final
    : public BasicControlBlock<VkDevice>
{
public:
    PhysicalDeviceInfo physicalDeviceInfo;
    gvk::Auto<VkDeviceCreateInfo> deviceCreateInfo;
    std::map<uint32_t, std::vector<QueueInfo>> queueInfos;
    VkBool32 pipelineStatisticsQuery_enabled{ };
    VkBool32 VK_EXT_pipeline_creation_cache_control_enabled{ };
    VkBool32 VK_EXT_shader_module_identifier_enabled{ };
    VkBool32 VK_KHR_performance_query_enabled{ };
    VkBool32 VK_KHR_pipeline_binary_enabled{ };
    VkBool32 VK_KHR_pipeline_executable_properties_enabled{ };
    VkBool32 VK_KHR_pipeline_properties_enabled{ };
    DeviceAddressTracker deviceAddressTracker;
};

template<>
class QueueInfo::ControlBlock final
    : public BasicControlBlock<VkQueue>
{
public:
    inline VkQueryPipelineStatisticFlags get_enabled_pipeline_statistics() const
    {
        return pipelineStatisticsQueryPool ? pipelineStatisticsQueryPool.get<VkQueryPoolCreateInfo>().pipelineStatistics : 0;
    }

    inline size_t get_enabled_pipieline_statistics_count() const
    {
        return std::bitset<32>(get_enabled_pipeline_statistics()).count();
    }

    inline size_t get_enabled_pipeline_statistics_result_size() const
    {
        return get_enabled_pipieline_statistics_count() * sizeof(uint64_t);
    }

    DeviceInfo deviceInfo;
    gvk::Auto<VkDeviceQueueCreateInfo> deviceQueueCreateInfo;
    gvk::QueryPool timestampQueryPool;
    uint32_t timestampQueryIndex{ };
    gvk::QueryPool pipelineStatisticsQueryPool;
    uint32_t pipelineStatisticsQueryIndex{ };
    gvk::Fence fence;

    class ShaderBindingTableReplacementResources final
    {
    public:
        inline VkResult get_gpu_address_map_pipeline(const gvk::Device& gvkDevice, gvk::Pipeline* pGvkPipeline)
        {
            gvk_result_scope_begin(VK_SUCCESS) {
                gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                gvk_result(pGvkPipeline ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                if (!mGpuAddressMapPipeline) {
                    gvk_result(gvk::create_shader_group_handle_map_pipeline(gvkDevice, &mGpuAddressMapPipeline));
                }
                *pGvkPipeline = mGpuAddressMapPipeline;
            } gvk_result_scope_end;
            return gvkResult;
        }

        inline VkResult get_gpu_memcpy_pipeline(const gvk::Device& gvkDevice, gvk::Pipeline* pGvkPipeline)
        {
            gvk_result_scope_begin(VK_SUCCESS) {
                gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                gvk_result(pGvkPipeline ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                if (!mGpuMemcpyPipeline) {
                    gvk_result(gvk::create_gpu_memcpy_pipeline(gvkDevice, &mGpuMemcpyPipeline));
                }
                *pGvkPipeline = mGpuMemcpyPipeline;
            } gvk_result_scope_end;
            return gvkResult;
        }

        inline VkResult get_buffer(const gvk::Device& gvkDevice, VkDeviceSize size, gvk::Buffer* pGvkBuffer)
        {
            gvk_result_scope_begin(VK_SUCCESS) {
                gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                gvk_result(size ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                gvk_result(pGvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
                bufferCreateInfo.size = size;
                bufferCreateInfo.usage =
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                    VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
                auto& availableBuffers = mAvailableBuffers[bufferCreateInfo.size];
                auto gvkBuffer = !availableBuffers.empty() ? *availableBuffers.begin() : VK_NULL_HANDLE;
                if (gvkBuffer) {
                    availableBuffers.erase(availableBuffers.begin());
                } else {
                    auto allocationCreateInfo = gvk::get_default<VmaAllocationCreateInfo>();
                    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
                    gvk_result(gvk::Buffer::create(gvkDevice, &bufferCreateInfo, &allocationCreateInfo, &gvkBuffer));
                }
                auto inserted = mInUseBuffers.insert(gvkBuffer).second;
                gvk_result(inserted ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                *pGvkBuffer = gvkBuffer;
            } gvk_result_scope_end;
            return gvkResult;
        }

        inline VkResult reset_available_resources()
        {
            gvk_result_scope_begin(VK_SUCCESS) {
                for (auto& buffer : mInUseBuffers) {
                    gvk_result(buffer ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                    auto size = buffer.get<VkBufferCreateInfo>().size;
                    auto inserted = mAvailableBuffers[size].insert(buffer).second;
                    gvk_result(inserted ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                }
                mInUseBuffers.clear();
            } gvk_result_scope_end;
            return gvkResult;
        }

        inline VkResult inspect_in_use_resources()
        {
            gvk_result_scope_begin(VK_SUCCESS) {
                std::unordered_map<VkBuffer, VkDeviceAddress> devicesAddresses;
                std::unordered_map<VkBuffer, std::vector<uint8_t>> data;
                for (auto& buffer : mInUseBuffers) {
                    gvk_result(buffer ? VK_SUCCESS : VK_ERROR_UNKNOWN);

                    auto bufferDeviceAddressInfo = gvk::get_default<VkBufferDeviceAddressInfo>();
                    bufferDeviceAddressInfo.buffer = buffer;
                    devicesAddresses[buffer] = buffer.get<gvk::Device>().GetBufferDeviceAddressKHR(&bufferDeviceAddressInfo);

                    uint8_t* pMappedData = nullptr;
                    gvkResult = vmaMapMemory(buffer.get<gvk::Device>().get<VmaAllocator>(), buffer.get<VmaAllocation>(), (void**)&pMappedData);
                    (void)gvkResult;
                    assert(gvkResult == VK_SUCCESS);
                    data[buffer].resize(buffer.get<VkBufferCreateInfo>().size);
                    memcpy(data[buffer].data(), pMappedData, buffer.get<VkBufferCreateInfo>().size);
                    vmaUnmapMemory(buffer.get<gvk::Device>().get<VmaAllocator>(), buffer.get<VmaAllocation>());
                }
            } gvk_result_scope_end;
            return gvkResult;
        }

    private:
        std::map<VkDeviceSize, std::set<gvk::Buffer>> mAvailableBuffers;
        std::set<gvk::Buffer> mInUseBuffers;
        gvk::Pipeline mGpuAddressMapPipeline;
        gvk::Pipeline mGpuMemcpyPipeline;
    };

    ShaderBindingTableReplacementResources shaderBindingTableReplacementResources;
};

template<>
class CommandBufferInfo::ControlBlock final
    : public BasicControlBlock<VkCommandBuffer>
{
public:
    inline VkResult reset(VkCommandBufferResetFlags flags)
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result(cmdTracker.record_vkResetCommandBuffer(vkHandle, flags));
            secondaryCommandBufferInfos.clear();
        } gvk_result_scope_end;
        return gvkResult;
    }

    inline VkResult begin(const VkCommandBufferBeginInfo* pBeginInfo)
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result(cmdTracker.record_vkBeginCommandBuffer(vkHandle, pBeginInfo));
            secondaryCommandBufferInfos.clear();
        } gvk_result_scope_end;
        return gvkResult;
    }

    inline VkResult end()
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result(cmdTracker.record_vkEndCommandBuffer(vkHandle));
        } gvk_result_scope_end;
        return gvkResult;
    }

    DeviceInfo deviceInfo;
    CommandPoolInfo commandPoolInfo;
    gvk::Auto<VkCommandBufferAllocateInfo> commandBufferAllocateInfo;
    std::set<CommandBufferInfo> secondaryCommandBufferInfos;
    VkCommandBuffer experimentCommandBuffer{VK_NULL_HANDLE};
    VkBool32 experimentEnabled{ };
    CmdTracker cmdTracker;
};

template<>
class CommandPoolInfo::ControlBlock final
    : public BasicControlBlock<VkCommandPool>
{
public:
    DeviceInfo deviceInfo;
    gvk::Auto<VkCommandPoolCreateInfo> commandPoolCreateInfo;
    std::set<CommandBufferInfo> commandBufferInfos;
};

template<>
class DescriptorSetLayoutInfo::ControlBlock final
    : public BasicControlBlock<VkDescriptorSetLayout>
{
public:
    DeviceInfo deviceInfo;
    gvk::Auto<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfo;
    std::vector<SamplerInfo> immutableSamplerInfos;
};

class ExecutableInfo final
{
public:
    gvk::Auto<VkPipelineExecutablePropertiesKHR> properties;
    std::vector<gvk::Auto<VkPipelineExecutableStatisticKHR>> statistics;
    std::vector<gvk::Auto<VkPipelineExecutableInternalRepresentationKHR>> internalRepresentations;
};

template<>
class PipelineInfo::ControlBlock final
    : public BasicControlBlock<VkPipeline>
{
public:
    DeviceInfo deviceInfo;
    VkPipelineBindPoint bindPoint{ };
    gvk::Auto<VkComputePipelineCreateInfo> computePipelineCreateInfo;
    gvk::Auto<VkGraphicsPipelineCreateInfo> graphicsPipelineCreateInfo;
    gvk::Auto<VkRayTracingPipelineCreateInfoKHR> rayTracingPipelineCreateInfo;
    std::vector<std::pair<VkShaderStageFlagBits, ShaderModuleInfo>> shaderModuleInfos;
    gvk::Auto<VkPipelinePropertiesIdentifierEXT> pipelinePropertiesIdentifier;
    std::vector<std::vector<uint8_t>> shaderGroupHandles;
    std::vector<std::vector<uint8_t>> experimentShaderGroupHandles;
    std::vector<ExecutableInfo> executableInfos;
    PipelineLayoutInfo pipelineLayoutInfo;
    RenderPassInfo renderPassInfo;
    UUID driverUUID{ };
    std::string name;
    std::set<std::string> labels;
    VkBool32 experimentEnabled{ };
    gvk::Pipeline experimentPipeline;
    gvk::ShaderGroupHandleMap shaderGroupHandleMap;
    gvk::Pipeline highlightPipeline;
    VkBool32 highlightEnabled{ };
    float highlightColor[4]{ };
};

template<>
class PipelineLayoutInfo::ControlBlock final
    : public BasicControlBlock<VkPipelineLayout>
{
public:
    DeviceInfo deviceInfo;
    gvk::Auto<VkPipelineLayoutCreateInfo> pipelineLayoutCreateInfo;
    std::vector<DescriptorSetLayoutInfo> descriptorSetLayoutInfos;
};

template<>
class RenderPassInfo::ControlBlock final
    : public BasicControlBlock<VkRenderPass>
{
public:
    DeviceInfo deviceInfo;
    gvk::Auto<VkRenderPassCreateInfo> renderPassCreateInfo;
    gvk::Auto<VkRenderPassCreateInfo2> renderPassCreateInfo2;
};

template<>
class SamplerInfo::ControlBlock final
    : public BasicControlBlock<VkSampler>
{
public:
    DeviceInfo deviceInfo;
    gvk::Auto<VkSamplerCreateInfo> samplerCreateInfo;
};

template<>
class ShaderModuleInfo::ControlBlock final
    : public BasicControlBlock<VkShaderModule>
{
public:
    DeviceInfo deviceInfo;
    gvk::Auto<VkShaderModuleCreateInfo> shaderModuleCreateInfo;
    gvk::Auto<VkShaderModuleIdentifierEXT> shaderModuleIdentifier;
    UUID driverUUID{ };
    std::string glsl;
    std::string spirv;
};

template<>
class BufferInfo::ControlBlock final
    : public BasicControlBlock<VkBuffer>
{
public:
    DeviceInfo deviceInfo;
    gvk::Auto<VkBufferCreateInfo> bufferCreateInfo;
};

template<>
class DeviceMemoryInfo::ControlBlock final
    : public BasicControlBlock<VkDeviceMemory>
{
public:
    DeviceInfo deviceInfo;
    gvk::Auto<VkMemoryAllocateInfo> memoryAllocateInfo;
};

////////////////////////////////////////////////////////////////////////////////
// TODO : Find a permanent home for these...

template <typename T>
inline typename std::vector<T>::iterator insert_sorted(std::vector<T>& vctr, const T& obj)
{
    return vctr.insert(std::upper_bound(vctr.begin(), vctr.end(), obj), obj);
}

#define DEBUG_UUID 0
template <typename CreateInfoType>
inline UUID get_uuid(VkDevice device, const gvk::Auto<CreateInfoType>& createInfo)
{
#if DEBUG_UUID
    std::cout << "Entered get_uuid(" << gvk::to_string(createInfo, gvk::Printer::EnumValue) << ")" << std::endl;
    std::cout << "{" << std::endl;
#endif

    auto createInfoCopy = createInfo;
    std::vector<UUID> dependencyUUIDs;
    gvk::detail::enumerate_structure_handles(
        *createInfoCopy,
        [&](VkObjectType objectType, const uint64_t& handle)
        {
            UUID dependencyUUID{ };
            switch (objectType) {
            case VK_OBJECT_TYPE_SAMPLER: {
                pipeline_explorer::SamplerInfo samplerInfo({ device, (VkSampler)handle });
                dependencyUUID = samplerInfo ? samplerInfo->uuid : 0;
            } break;
            case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: {
                pipeline_explorer::DescriptorSetLayoutInfo descriptorSetLayoutInfo({ device, (VkDescriptorSetLayout)handle });
                dependencyUUID = descriptorSetLayoutInfo ? descriptorSetLayoutInfo->uuid : 0;
            } break;
            case VK_OBJECT_TYPE_PIPELINE_LAYOUT: {
                pipeline_explorer::PipelineLayoutInfo pipelineLayoutInfo({ device, (VkPipelineLayout)handle });
                dependencyUUID = pipelineLayoutInfo ? pipelineLayoutInfo->uuid : 0;
            } break;
            case VK_OBJECT_TYPE_RENDER_PASS: {
                pipeline_explorer::RenderPassInfo renderPassInfo({ device, (VkRenderPass)handle });
                dependencyUUID = renderPassInfo ? renderPassInfo->uuid : 0;
            } break;
            case VK_OBJECT_TYPE_SHADER_MODULE: {
                pipeline_explorer::ShaderModuleInfo shaderModuleInfo({ device, (VkShaderModule)handle });
                dependencyUUID = shaderModuleInfo ? shaderModuleInfo->uuid : 0;
            } break;
            default: {
            } break;
            }
            if (dependencyUUID) {
                pipeline_explorer::insert_sorted(dependencyUUIDs, dependencyUUID);
            } else {
                // TODO : Error to GUI : Missing VkPipeline dependency; likely caused by unserviced extension; gvk maintenance required
            }
            const_cast<uint64_t&>(handle) = 0;
        }
    );
    auto uuid = pipeline_explorer::get_256_bit_hash(gvk::to_string(createInfoCopy, 0));

#if DEBUG_UUID
    std::cout << "    uuid (precombine)                 = " << uuid << std::endl;
    for (const auto& dependencyUUID : dependencyUUIDs) {
        std::cout << "        " << dependencyUUID << std::endl;
    }
#endif

    for (const auto& dependencyUUID : dependencyUUIDs) {
        uuid ^= dependencyUUID;
    }

#if DEBUG_UUID
    static std::set<UUID> sUUIDs;
    std::stringstream strStrm;
    strStrm << std::hex << uuid;
    std::cout << "    uuid (postcombine)                = " << uuid << std::endl;
    std::cout << "    gvk::to_hex_string(uuid)          = " << gvk::to_hex_string(uuid) << std::endl;
    std::cout << "    std::stringstream << std::hex     = " << strStrm.str() << std::endl;
    std::cout << "    std::stringstream << std::hex(16) = " << strStrm.str().substr(0, 16) << std::endl;
    std::cout << "    " << (sUUIDs.insert(uuid).second ? "Unique uuid" : "Duplicate uuid") << std::endl;
    std::cout << "}" << std::endl;
    std::cout << "Leaving get_uuid()" << std::endl;
    std::cout << std::endl;
#endif

    return uuid;
}

class NamedEntryCollection final
{
public:
    inline std::set<std::string> validate(uint32_t layerPropertyCount, VkLayerProperties const* pLayerProperties)
    {
        std::set<std::string> availableEntries;
        for (uint32_t i = 0; i < layerPropertyCount; ++i) {
            availableEntries.insert(pLayerProperties[i].layerName);
        }
        return validate(availableEntries);
    }

    inline std::set<std::string> validate(uint32_t extensionPropertyCount, VkExtensionProperties const* pExtensionProperties)
    {
        std::set<std::string> availableEntries;
        for (uint32_t i = 0; i < extensionPropertyCount; ++i) {
            availableEntries.insert(pExtensionProperties[i].extensionName);
        }
        return validate(availableEntries);
    }

    inline void add(const char* pEntry, bool force = false)
    {
        assert(pEntry);
        if (force) {
            erase(pEntry);
        }
        if (mEntryIndices.insert({pEntry, (uint32_t)mEntries.size()}).second) {
            mEntries.push_back(pEntry);
        }
    }

    inline void add(uint32_t entryCount, const char* const* pEntries, bool force = false)
    {
        if (pEntries) {
            for (uint32_t i = 0; i < entryCount; ++i) {
                add(pEntries[i], force);
            }
        }
    }

    inline void erase(const char* pEntry)
    {
        if (pEntry) {
            auto itr = mEntryIndices.find(pEntry);
            if (itr != mEntryIndices.end()) {
                auto index = itr->second;
                assert(index < mEntries.size());
                assert(mEntries[index]);
                assert(!strcmp(mEntries[index], pEntry));
                mEntryIndices.erase(itr);
                mEntries.erase(mEntries.begin() + index);
                for (; index < mEntries.size(); ++index) {
                    itr = mEntryIndices.find(mEntries[index]);
                    assert(itr != mEntryIndices.end());
                    itr->second = index;
                }
            }
        }
    }

    inline bool contains(const char* pEntry) const
    {
        return pEntry ? mEntryIndices.count(pEntry) : 0;
    }

    inline void clear()
    {
        mEntryIndices.clear();
        mEntries.clear();
    }

    inline uint32_t count() const
    {
        return (uint32_t)mEntries.size();
    }

    inline const char* const* data() const
    {
        return !mEntries.empty() ? mEntries.data() : nullptr;
    }

private:
    inline std::set<std::string> validate(std::set<std::string> const& availableEntries)
    {
        std::set<std::string> invalidEntries;
        for (uint32_t i = 0; i < mEntries.size();) {
            assert(mEntries[i]);
            if (!availableEntries.count(mEntries[i])) {
                invalidEntries.insert(mEntries[i]);
                erase(mEntries[i]);
            } else {
                ++i;
            }
        }
        return invalidEntries;
    }

    std::map<std::string, uint32_t> mEntryIndices;
    std::vector<char const*> mEntries;
};

using LayerCollection = NamedEntryCollection;
using ExtensionCollection = NamedEntryCollection;

inline std::vector<VkLayerProperties> get_instance_layer_properties(PFN_vkEnumerateInstanceLayerProperties pfnEnumerateInstanceLayerProperties)
{
    assert(pfnEnumerateInstanceLayerProperties);
    uint32_t propertyCount = 0;
    auto vkResult = pfnEnumerateInstanceLayerProperties(&propertyCount, nullptr);
    (void)vkResult;
    assert(vkResult == VK_SUCCESS);
    std::vector<VkLayerProperties> properties(propertyCount);
    vkResult = pfnEnumerateInstanceLayerProperties(&propertyCount, properties.data());
    assert(vkResult == VK_SUCCESS);
    return properties;
}

inline std::vector<VkExtensionProperties> get_instance_extension_properties(PFN_vkEnumerateInstanceExtensionProperties pfnEnumerateInstanceExtensionProperties)
{
    assert(pfnEnumerateInstanceExtensionProperties);
    uint32_t propertyCount = 0;
    auto vkResult = pfnEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
    (void)vkResult;
    assert(vkResult == VK_SUCCESS);
    std::vector<VkExtensionProperties> properties(propertyCount);
    vkResult = pfnEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());
    assert(vkResult == VK_SUCCESS);
    return properties;
}

inline std::vector<VkExtensionProperties> get_device_extension_properties(VkPhysicalDevice physicalDevice, PFN_vkEnumerateDeviceExtensionProperties pfnEnumerateDeviceExtensionProperties)
{
    assert(pfnEnumerateDeviceExtensionProperties);
    uint32_t propertyCount = 0;
    auto vkResult = pfnEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr);
    (void)vkResult;
    assert(vkResult == VK_SUCCESS);
    std::vector<VkExtensionProperties> properties(propertyCount);
    vkResult = pfnEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, properties.data());
    assert(vkResult == VK_SUCCESS);
    return properties;
}

inline std::set<std::string> remove_layer_env_var_values(const std::string& key, const std::set<std::string>& layerValuesToRemove)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
    auto delimiter = ";";
#else
    auto delimiter = ":";
#endif
    std::string value;
    std::set<std::string> removedLayerValues;
    for (const auto& layer : gvk::string::split(gvk::get_env_var(key), delimiter)) {
        if (layerValuesToRemove.count(layer)) {
            removedLayerValues.insert(layer);
        } else {
            value += layer + delimiter;
        }
    }
    gvk::set_env_var(key, value);
    return removedLayerValues;
}

inline std::set<std::string> remove_api_dump_and_validation_layers_from_environment()
{
    std::set<std::string> layerValuesToRemove{
        "VK_LAYER_LUNARG_api_dump",
        "api_dump",
        "VK_LAYER_KHRONOS_validation",
        "validation",
        "VK_LAYER_LUNARG_crash_diagnostic",
        "crash_diagnostic",
    };
    auto instanceLayers = remove_layer_env_var_values("VK_INSTANCE_LAYERS", layerValuesToRemove);
    auto loaderLayers = remove_layer_env_var_values("VK_LOADER_LAYERS_ENABLE", layerValuesToRemove);
    instanceLayers.insert(loaderLayers.begin(), loaderLayers.end());
    return instanceLayers;
}

inline const std::vector<std::string>& get_validation_layer_setting_names()
{
    static const std::vector<std::string> sValidationLayerSettingNames{
        /* BOOL      : true                                            */ "VK_LAYER_FINE_GRAINED_LOCKING",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_VALIDATE_CORE",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_IMAGE_LAYOUT",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_COMMAND_BUFFER",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_OBJECT_IN_USE",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_QUERY",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_SHADERS",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_SHADERS_CACHING",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_UNIQUE_HANDLES",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_OBJECT_LIFETIME",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_STATELESS_PARAM",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_THREAD_SAFETY",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_VALIDATE_SYNC",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_SYNC_QUEUE_SUBMIT",
        /* ENUM      : GPU_BASED_NONE                                  */  // "VK_KHRONOS_VALIDATION_VALIDATE_GPU_BASED",
        /* BOOL      : true                                            */  // "VK_KHRONOS_VALIDATION_PRINTF_TO_STDOUT",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_PRINTF_VERBOSE",
        /* INT       : 1024                                            */  // "VK_KHRONOS_VALIDATION_PRINTF_BUFFER_SIZE",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_RESERVE_BINDING_SLOT",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_VMA_LINEAR_OUTPUT",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_GPUAV_DESCRIPTOR_CHECKS",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_WARN_ON_ROBUST_OOB",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_VALIDATE_INDIRECT_BUFFER",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_USE_INSTRUMENTED_SHADER_CACHE",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_SELECT_INSTRUMENTED_SHADERS",
        /* INT       : 10000                                           */  // "VK_KHRONOS_VALIDATION_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_ARM",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_AMD",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_IMG",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_NVIDIA",
        /* FLAGS     : VK_DBG_LAYER_ACTION_LOG_MSG                     */  // "VK_KHRONOS_VALIDATION_DEBUG_ACTION",
        /* SAVE_FILE : stdout                                          */  // "VK_KHRONOS_VALIDATION_LOG_FILENAME",
        /* FLAGS     : error                                           */  // "VK_KHRONOS_VALIDATION_REPORT_FLAGS",
        /* BOOL      : true                                            */  // "VK_KHRONOS_VALIDATION_ENABLE_MESSAGE_LIMIT",
        /* INT       : 10                                              */  // "VK_LAYER_DUPLICATE_MESSAGE_LIMIT",
        /* LIST      :                                                 */  // "VK_LAYER_MESSAGE_ID_FILTER",
        /* FLAGS     : VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT */  // "VK_LAYER_DISABLES",
        /* FLAGS     :                                                 */  // "VK_LAYER_ENABLES",
    };
    return sValidationLayerSettingNames;
}

inline void create_pnext_chain(std::vector<VkBaseOutStructure*> structPtrs)
{
    if (!structPtrs.empty()) {
        structPtrs.back()->pNext = nullptr;
        for (size_t i = 0; i < structPtrs.size() - 1; ++i) {
            structPtrs[i]->pNext = structPtrs[i + 1];
        }
    }
}

inline void create_pnext_chain(std::unordered_map<VkStructureType, VkBaseOutStructure*> pNextMap)
{
    if (!pNextMap.empty()) {
#if 0
        for (size_t i = 0; i < structPtrs.size() - 1; ++i) {
            auto pCurr = structPtrs[i];
            assert(!pCurr->pNext);
            pCurr->pNext = structPtrs[i + 1];
        }
#endif
    }
}

inline std::unordered_map<VkStructureType, VkBaseOutStructure*> create_pnext_map(VkBaseOutStructure* pStruct)
{
    (void)pStruct;
    return { };
}

class PNextChainEditor final
{
public:
    PNextChainEditor(const VkBaseInStructure& root)
    {
        auto pAllocator = gvk::detail::validate_allocation_callbacks(nullptr);
        auto pCurrent = const_cast<VkBaseInStructure*>(root.pNext);
        while (pCurrent) {

            // Remove the pNext so the whole chain doesn't get copied
            // TODO : It would be nice to use generated code for this, the only problem is
            //  handling VkLoaderInstanceCreateInfo and VkLoaderDeviceCreateInfo.  It should
            //  be possible to get those generated, but vk_layer.h isn't part of the core
            //  API so the generated code will need some forward declarations and custom
            //  handlers.
            auto pNext = pCurrent->pNext;
            pCurrent->pNext = nullptr;

            // TODO : Documentation
            VkBaseOutStructure* pObj = nullptr;

            // Process individual pNext chain members
            switch (pCurrent->sType) {
            case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO: {
                pObj = (VkBaseOutStructure*)pAllocator->pfnAllocation(nullptr, sizeof(VkLayerInstanceCreateInfo), 0, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
                *(VkLayerInstanceCreateInfo*)pObj = *(VkLayerInstanceCreateInfo*)pCurrent;
            } break;
            case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO: {
                pObj = (VkBaseOutStructure*)pAllocator->pfnAllocation(nullptr, sizeof(VkLayerDeviceCreateInfo), 0, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
                *(VkLayerDeviceCreateInfo*)pObj = *(VkLayerDeviceCreateInfo*)pCurrent;
            } break;
            default: {
                pObj = (VkBaseOutStructure*)gvk::detail::create_pnext_copy(pCurrent, pAllocator);
                auto inserted = mPNextMap.insert({ pObj->sType, pObj }).second;
                (void)inserted;
                assert(inserted);
            } break;
            }

            // TODO : Documentation
            if (!mPNextChain.empty()) {
                mPNextChain.back()->pNext = pObj;
            }
            mPNextChain.push_back(pObj);

            // Revert pCurrent's pNext, then advance
            pCurrent->pNext = pNext;
            pCurrent = const_cast<VkBaseInStructure*>(pCurrent->pNext);
        }
    }

    ~PNextChainEditor()
    {
        auto pAllocator = gvk::detail::validate_allocation_callbacks(nullptr);
        for (auto pObj : mPNextChain) {
            pObj->pNext = nullptr;
            switch (pObj->sType) {
            case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO:
            case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO: {
                pAllocator->pfnFree(nullptr, pObj);
            } break;
            default: {
                gvk::detail::destroy_pnext_copy(pObj, pAllocator);
            } break;
            }
        }
    }

    VkBaseOutStructure* get(VkStructureType sType)
    {
        auto itr = mPNextMap.find(sType);
        return itr != mPNextMap.end() ? itr->second : nullptr;
    }

    template <typename StructureType>
    StructureType* get()
    {
        return (StructureType*)get(gvk::get_stype<StructureType>());
    }

    const VkBaseOutStructure* get() const
    {
        return !mPNextChain.empty() ? mPNextChain.front() : nullptr;
    }

    template <typename StructureType>
    void set(StructureType structure)
    {
        auto itr = mPNextMap.find(gvk::get_stype<StructureType>());
        if (itr != mPNextMap.end()) {
            *((StructureType*)itr->second) = structure;
        } else {
            structure.pNext = nullptr;
            auto pObj = (VkBaseOutStructure*)gvk::detail::create_dynamic_array_copy(1, &structure, nullptr);
            if (!mPNextChain.empty()) {
                mPNextChain.back()->pNext = pObj;
            }
            mPNextChain.push_back(pObj);
            auto inserted =  mPNextMap.insert({ gvk::get_stype<StructureType>(), pObj }).second;
            (void)inserted;
            assert(inserted);
        }
    }

private:
    std::vector<VkBaseOutStructure*> mPNextChain;
    std::unordered_map<VkStructureType, VkBaseOutStructure*> mPNextMap;

    PNextChainEditor(const PNextChainEditor&) = delete;
    PNextChainEditor& operator=(const PNextChainEditor&) = delete;
};
////////////////////////////////////////////////////////////////////////////////

} // namespace pipeline_explorer
} // namespace gvk
