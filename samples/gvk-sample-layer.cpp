
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

#include "gvk-layer/registry.hpp"
#include "gvk-structures/to-string.hpp"

#include <iostream>

// To activate VK_LAYER_INTEL_gvk_sample with the sample applications, set the
//  following environment variables...
//      VK_LAYER_PATH=<path/to/gvk/sample/binary/directory>
//      VK_INSTANCE_LAYERS=VK_LAYER_INTEL_gvk_sample
// NOTE : This will disable the default Vulkan layers

// GvkSampleLayer extend gvk::layer::BasicLayer.  Any pre/post Vulkan API hooks
//  that override the base implementation will be called along with the
//  associated Vulkan API call.  If the API call has a return value, that value
//  will be passed through registered layers with the name 'gvkResult'.
class GvkSampleLayer final
    : public gvk::layer::BasicLayer
{
public:
    gvk::Printer::Flags printerFlags { gvk::Printer::Default & ~gvk::Printer::EnumValue };

    VkResult pre_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks*, VkInstance*, VkResult gvkResult) override final
    {
        assert(pCreateInfo);
        std::cout << "================================================================================" << std::endl;
        std::cout << "pre_vkCreateInstance() : pCreateInfo : " << gvk::to_string(*pCreateInfo, printerFlags) << std::endl;
        return gvkResult;
    }

    VkResult post_vkCreateInstance(const VkInstanceCreateInfo*, const VkAllocationCallbacks*, VkInstance* pInstance, VkResult gvkResult) override final
    {
        assert(pInstance);
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
        std::cout << "post_vkCreateInstance() : " << gvk::to_string(gvkResult, printerFlags) << " : " << gvk::to_string(*pInstance) << std::endl;
        std::cout << "================================================================================" << std::endl;
        return gvkResult;
    }

    VkResult pre_vkCreateDevice(VkPhysicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks*, VkDevice*, VkResult gvkResult) override final
    {
        assert(pCreateInfo);
        std::cout << "================================================================================" << std::endl;
        std::cout << "pre_vkCreateDevice() : pCreateInfo : " << gvk::to_string(*pCreateInfo, printerFlags) << std::endl;
        return gvkResult;
    }

    VkResult post_vkCreateDevice(VkPhysicalDevice, const VkDeviceCreateInfo*, const VkAllocationCallbacks*, VkDevice* pDevice, VkResult gvkResult) override final
    {
        assert(pDevice);
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
        std::cout << "post_vkCreateDevice() : " << gvk::to_string(gvkResult, printerFlags) << " : " << gvk::to_string(*pDevice) << std::endl;
        std::cout << "================================================================================" << std::endl;
        return gvkResult;
    }

    VkResult pre_vkAllocateMemory(VkDevice, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks*, VkDeviceMemory*, VkResult gvkResult) override final
    {
        assert(pAllocateInfo);
        std::cout << "================================================================================" << std::endl;
        std::cout << "pre_vkAllocateMemory() : pAllocateInfo : " << gvk::to_string(*pAllocateInfo, printerFlags) << std::endl;
        return gvkResult;
    }

    VkResult post_vkAllocateMemory(VkDevice, const VkMemoryAllocateInfo*, const VkAllocationCallbacks*, VkDeviceMemory* pMemory, VkResult gvkResult) override final
    {
        assert(pMemory);
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
        std::cout << "post_vkAllocateMemory() : " << gvk::to_string(gvkResult, printerFlags) << " : " << gvk::to_string(*pMemory) << std::endl;
        std::cout << "================================================================================" << std::endl;
        return gvkResult;
    }
};

// void gvk::layer::on_load(Registry&) must be provided by a custom gvk layer.
//  This function is used to register layers with the gvk::layer::Registry.
//  Note that if multiple layers are used, they will be viewed by the Vulkan
//  loader as a single layer, this is useful for configuring functionality
//  without requiring enable/disable logic be built directly into layers.  If
//  multiple different layers are required at the loader level, a CMake target
//  should be created for each individually.
// NOTE : pre API call hooks are run in the order registered, post API call
//  hooks are run in the opposite order, for example:
//
//      If the following layers were registered in on_load()...
//          registry.layers.push_back(std::make_unique<FooLayer>());
//          registry.layers.push_back(std::make_unique<BarLayer>());
//          registry.layers.push_back(std::make_unique<BazLayer>());
//
//      API call vkCreateDevice() will result in the following call order...
//          FooLayer::pre_vkCreateDevice()
//          BarLayer::pre_vkCreateDevice()
//          BazLayer::pre_vkCreateDevice()
//          vkCreateDevice()
//          BazLayer::post_vkCreateDevice()
//          BarLayer::post_vkCreateDevice()
//          FooLayer::post_vkCreateDevice()
namespace gvk {
namespace layer {

void on_load(Registry& registry)
{
    registry.layers.push_back(std::make_unique<GvkSampleLayer>());
}

} // namespace layer
} // namespace gvk

// VK_LAYER_EXPORT VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface*)
//  must be provided by a custom gvk layer.  This function is used to register
//  layers with the Vulkan loader.
#ifdef __cplusplus
extern "C" {
#endif

VK_LAYER_EXPORT VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pNegotiateLayerInterface)
{
    assert(pNegotiateLayerInterface);
    pNegotiateLayerInterface->pfnGetInstanceProcAddr = gvk::layer::get_instance_proc_addr;
    pNegotiateLayerInterface->pfnGetPhysicalDeviceProcAddr = gvk::layer::get_physical_device_proc_addr;
    pNegotiateLayerInterface->pfnGetDeviceProcAddr = gvk::layer::get_device_proc_addr;
    return VK_SUCCESS;
}

#ifdef __cplusplus
}
#endif
