
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

#include "gvk-environment.hpp"
#include "gvk-handles.hpp"

#include "gtest/gtest.h"

namespace gvk {
namespace spirv {
namespace validation {

// NOTE : Duplicated in...
//  gvk/gvk-pipeline-explorer/source/gvk-pipeline-explorer/backend/device.cpp
//  gvk/gvk-state-tracker/tests/state-tracker-test-utilities.cpp
//  gvk/gvk-spirv/tests/spirv-validation-context.hpp
// TODO : Move to a common location
template <typename PhysicalDeviceFeatures>
inline PhysicalDeviceFeatures get_available_physical_device_features(const gvk::PhysicalDevice& gvkPhysicalDevice)
{
    assert(gvkPhysicalDevice);
    auto physicalDeviceFeatures = gvk::get_default<PhysicalDeviceFeatures>();
    auto physicalDeviceFeatures2 = gvk::get_default<VkPhysicalDeviceFeatures2>();
    physicalDeviceFeatures2.pNext = &physicalDeviceFeatures;
    gvkPhysicalDevice.GetPhysicalDeviceFeatures2(&physicalDeviceFeatures2);
    return physicalDeviceFeatures;
}

class Context final
    : public gvk::Context
{
public:
    static VkResult create(Context* pContext)
    {
        assert(pContext);
#if defined(_WIN32) || defined(_WIN64)
        auto vkLayerPath = gvk::get_env_var("VK_LAYER_PATH");
        if (vkLayerPath.empty()) {
            gvk::set_vk_layer_path_from_windows_registry();
        }
#endif
        auto instanceCreateInfo = gvk::get_default<VkInstanceCreateInfo>();
        auto contextCreateInfo = gvk::get_default<gvk::Context::CreateInfo>();
#if defined(_WIN32) || defined(_WIN64)
        contextCreateInfo.loadValidationLayer = VK_TRUE;
#endif
        contextCreateInfo.pInstanceCreateInfo = &instanceCreateInfo;
        return gvk::Context::create(&contextCreateInfo, nullptr, pContext);
    }

    const VkPhysicalDevice8BitStorageFeatures& get_physical_device_8_bit_storage_features() const
    {
        return mPhysicalDevice8BitStorageFeatures;
    }

    const VkPhysicalDeviceShaderFloat16Int8Features& get_physical_device_shader_float_16_int_8_features() const
    {
        return mPhysicalDeviceShaderFloat16Int8Features;
    }

    const VkPhysicalDeviceBufferDeviceAddressFeatures& get_physical_device_buffer_device_address_features() const
    {
        return mPhysicalDeviceBufferDeviceAddressFeatures;
    }

protected:
    VkResult create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, std::vector<gvk::Device>* pDevices) const override final
    {
        assert(pDeviceCreateInfo);

        const auto& gvkPhysicalDevices = get<gvk::PhysicalDevices>();
        assert(!gvkPhysicalDevices.empty());
        const auto& gvkPhysicalDevice = gvkPhysicalDevices[0];

        // TODO : Refactor all this to use PNextChainEditor and ExtensionsCollection
        //  from gvk-pipeline-explorer
        std::vector<const char*> extensions(pDeviceCreateInfo->ppEnabledExtensionNames, pDeviceCreateInfo->ppEnabledExtensionNames + pDeviceCreateInfo->enabledExtensionCount);
        auto enabledPhysicalDeviceFeatures = gvk::get_default<VkPhysicalDeviceFeatures2>();

        // TODO : It is nice to have these factory functions return their result so they
        //  can be const, but these const_cast<>() are no good...gotta take the consts
        //  off these Context::create() functions.
        const_cast<VkPhysicalDevice8BitStorageFeatures&>(mPhysicalDevice8BitStorageFeatures) = get_available_physical_device_features<VkPhysicalDevice8BitStorageFeatures>(gvkPhysicalDevice);
        const_cast<VkPhysicalDeviceShaderFloat16Int8Features&>(mPhysicalDeviceShaderFloat16Int8Features) = get_available_physical_device_features<VkPhysicalDeviceShaderFloat16Int8Features>(gvkPhysicalDevice);
        const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures&>(mPhysicalDeviceBufferDeviceAddressFeatures) = get_available_physical_device_features<VkPhysicalDeviceBufferDeviceAddressFeatures>(gvkPhysicalDevice);
        if (mPhysicalDevice8BitStorageFeatures.storageBuffer8BitAccess && mPhysicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress) {
            const_cast<VkPhysicalDevice8BitStorageFeatures&>(mPhysicalDevice8BitStorageFeatures).pNext = enabledPhysicalDeviceFeatures.pNext;
            enabledPhysicalDeviceFeatures.pNext = (void*)&mPhysicalDevice8BitStorageFeatures;
            const_cast<VkPhysicalDeviceShaderFloat16Int8Features&>(mPhysicalDeviceShaderFloat16Int8Features).pNext = enabledPhysicalDeviceFeatures.pNext;
            enabledPhysicalDeviceFeatures.pNext = (void*)&mPhysicalDeviceShaderFloat16Int8Features;
            const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures&>(mPhysicalDeviceBufferDeviceAddressFeatures).pNext = enabledPhysicalDeviceFeatures.pNext;
            enabledPhysicalDeviceFeatures.pNext = (void*)&mPhysicalDeviceBufferDeviceAddressFeatures;
            extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            enabledPhysicalDeviceFeatures.features.shaderInt64 = VK_TRUE;
        }

        auto deviceCreateInfo = *pDeviceCreateInfo;
        deviceCreateInfo.pNext = &enabledPhysicalDeviceFeatures;
        deviceCreateInfo.enabledExtensionCount = (uint32_t)extensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = !extensions.empty() ? extensions.data() : nullptr;
        pDevices->push_back({ });
        return gvk::Device::create(get<gvk::PhysicalDevices>()[0], &deviceCreateInfo, nullptr, &pDevices->back());
    }

private:
    VkPhysicalDevice8BitStorageFeatures mPhysicalDevice8BitStorageFeatures{ };
    VkPhysicalDeviceShaderFloat16Int8Features mPhysicalDeviceShaderFloat16Int8Features{ };
    VkPhysicalDeviceBufferDeviceAddressFeatures mPhysicalDeviceBufferDeviceAddressFeatures{ };
};

} // namespace validation
} // namespace spirv
} // namespace gvk
