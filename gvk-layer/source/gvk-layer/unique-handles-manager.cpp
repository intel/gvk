
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

#include "gvk-layer/unique-handles-manager.hpp"
#include "gvk-layer/generated/unique-handles-layer-hooks.hpp"
#include "gvk-layer/registry.hpp"

#include <algorithm>

namespace gvk {
namespace layer {
namespace hooks {
namespace unique_handles {

/*

Order of operations in generated code

    Intiailize GvkCommandStructure

    set_current_command()

    // At this point, the command contains unique handle IDs.  This function will
    //  enumerate the command and replace unique handle IDs with the actual handles.
    unwrap_handles()

    // Execute the command using the unwrapped handles
    result = execute_command_structure()

    // on_create_object() is manually implmented in this file for some commands.
    //  This function runs before handles are rewrapped, so all handles in the given
    //  command are provided as unwrapped handles.
    on_create_object()

    // on_handle_out() is manually implmented in this file for some commands.
    //  This function runs before handles are rewrapped, so all handles in the given
    //  command are provided as unwrapped handles.
    on_handle_out()

    // This function will revert all of the unwrapped handles to unique handle IDs.
    rewrap_handles()

    // on_destroy_object() is manually implmented in this file for some commands.
    //  This function runs after handles are rewrapped, so all handles in the given
    //  command are provided as unique handle IDs.  Handles can be unwrapped as needed.
    on_destroy_object()

    clear_current_command()

    return result

*/

////////////////////////////////////////////////////////////////////////////////
// VkDescriptorPool/VkDescriptorSet
/*

VkDescriptorPool and VkDescriptorSet need manual tracking since VkDescriptorSet
lifetime is tied to the parent VkDescriptorPool.  To manage this, a collection
of VkDescriptorSet IDs is mapped to each VkDescriptorPool.
NOTE : VkCommandPool and VkCommandBuffer follow a similar pattern, but VkCommandBuffer
    is a dispatchable handle so no wrapping is done, therefore no tracking is neeeded.

*/
VkResult on_destroy_object(const GvkCommandStructureResetDescriptorPool& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Get UniqueHandlesManager
        auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
        gvk_result_assert(uniqueHandlesManager.enabled);

        // Unwrap VkDescriptorPool
        VkDescriptorPool descriptorPool = command.descriptorPool;
        gvk_result(uniqueHandlesManager.unwrap_handle(VK_OBJECT_TYPE_DESCRIPTOR_POOL, &descriptorPool));
        gvk_result_assert(descriptorPool);

        // Use VkDescriptorPool to look up associated VkDescriptorSet ID collection
        std::unordered_set<uint64_t>* pDescriptorSetIds = nullptr;
        {
            std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
            pDescriptorSetIds = &uniqueHandlesManager.descriptorPools[{ command.device, descriptorPool }];
        }

        // Remove each VkDescriptorSet ID from UniqueHandlesManager
        for (const auto& descriptorSetId : *pDescriptorSetIds) {
            gvk_result(uniqueHandlesManager.on_destroy_object((VkDescriptorSet)descriptorSetId));
        }

        // Clear VkDescriptorSet ID collection
        pDescriptorSetIds->clear();
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_destroy_object(const GvkCommandStructureDestroyDescriptorPool& command)
{
    gvk_result_scope_begin(VK_SUCCESS) {

        // Get UniqueHandlesManager
        auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
        gvk_result_assert(uniqueHandlesManager.enabled);

        // Unwrap VkDescriptorPool
        VkDescriptorPool descriptorPool = command.descriptorPool;
        gvk_result(uniqueHandlesManager.unwrap_handle(VK_OBJECT_TYPE_DESCRIPTOR_POOL, &descriptorPool));
        gvk_result_assert(descriptorPool);

        // Use VkDescriptorPool to look up associated VkDescriptorSet ID collection
        std::unordered_set<uint64_t>* pDescriptorSetIds = nullptr;
        {
            std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
            pDescriptorSetIds = &uniqueHandlesManager.descriptorPools[{ command.device, descriptorPool }];
        }

        // Remove each VkDescriptorSet ID from UniqueHandlesManager
        for (const auto& descriptorSetId : *pDescriptorSetIds) {
            gvk_result(uniqueHandlesManager.on_destroy_object((VkDescriptorSet)descriptorSetId));
        }

        // Remove VkDescriptorSet ID collection from UniqueHandlesManager
        {
            std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
            uniqueHandlesManager.descriptorPools.erase({ command.device, descriptorPool });
        }

        // Remove VkDescriptorPool ID from UniqueHandlesManager
        gvk_result(uniqueHandlesManager.on_destroy_object(command.descriptorPool));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_create_object(const GvkCommandStructureAllocateDescriptorSets& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Get UniqueHandlesManager
        auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
        gvk_result_assert(uniqueHandlesManager.enabled);

        // Use VkDescriptorPool to look up associated VkDescriptorSet ID collection
        std::unordered_set<uint64_t>* pDescriptorSetIds = nullptr;
        {
            std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
            pDescriptorSetIds = &uniqueHandlesManager.descriptorPools[{ command.device, command.pAllocateInfo->descriptorPool }];
        }

        // Create VkDescriptorSet IDs and add them to UniqueHandlesManager and ID collection
        for (uint32_t i = 0; i < command.pAllocateInfo->descriptorSetCount; ++i) {
            uint64_t id = 0;
            gvk_result(uniqueHandlesManager.on_create_object(command.pDescriptorSets + i, &id));
            pDescriptorSetIds->insert(id);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_destroy_object(const GvkCommandStructureFreeDescriptorSets& command)
{
    gvk_result_scope_begin(VK_SUCCESS) {

        // Get UniqueHandlesManager
        auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
        gvk_result_assert(uniqueHandlesManager.enabled);

        // Unwrap VkDescriptorPool
        VkDescriptorPool descriptorPool = command.descriptorPool;
        gvk_result(uniqueHandlesManager.unwrap_handle(VK_OBJECT_TYPE_DESCRIPTOR_POOL, &descriptorPool));
        gvk_result_assert(descriptorPool);

        // Use VkDescriptorPool to look up associated VkDescriptorSet ID collection
        std::unordered_set<uint64_t>* pDescriptorSetIds = nullptr;
        {
            std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
            pDescriptorSetIds = &uniqueHandlesManager.descriptorPools[{ command.device, descriptorPool }];
        }

        // Remove each VkDescriptorSet ID from UniqueHandlesManager and ID collection
        for (uint32_t i = 0; i < command.descriptorSetCount; ++i) {
            auto descriptorSet = command.pDescriptorSets[i];
            gvk_result(uniqueHandlesManager.on_destroy_object(descriptorSet));
            pDescriptorSetIds->erase((uint64_t)descriptorSet);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

////////////////////////////////////////////////////////////////////////////////
// VkPipelineBinaryKHR
/*

VkPipelineBinaryKHR is returned from its create command in a structure.

*/
VkResult on_create_object(const GvkCommandStructureCreatePipelineBinariesKHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Get UniqueHandlesManager
        auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
        gvk_result_assert(uniqueHandlesManager.enabled);

        // Create VkPipelineBinaryKHR IDs and add them to UniqueHandlesManager
        for (uint32_t i = 0; i < command.pBinaries->pipelineBinaryCount; ++i) {
            gvk_result(uniqueHandlesManager.on_create_object(command.pBinaries->pPipelineBinaries + i));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

////////////////////////////////////////////////////////////////////////////////
// VkSwapchainKHR
/*

VkSwapchainKHR and VkImage need manual tracking since VkImage lifetime is tied to
the parent VkSwapchainKHR.  To manage this, a collection of VkImage IDs is mapped
to each VkSwapchainKHR.

*/
VkResult on_create_object(const GvkCommandStructureCreateSwapchainKHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Get UniqueHandlesManager
        auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
        gvk_result_assert(uniqueHandlesManager.enabled);

        // Add VkSwapchainKHR to UniqueHandlesManager
        gvk_result(uniqueHandlesManager.on_create_object(command.pSwapchain));

        // Get dispatch table
        const auto& dispatchTableItr = uniqueHandlesManager.VkDeviceDispatchTables.find(get_dispatch_key(command.device));
        gvk_result_assert(dispatchTableItr != uniqueHandlesManager.VkDeviceDispatchTables.end());
        gvk_result_assert(dispatchTableItr->second.gvkGetSwapchainImagesKHR);

        // Get VkImages
        uint32_t imageCount = 0;
        gvk_result(dispatchTableItr->second.gvkGetSwapchainImagesKHR(command.device, *command.pSwapchain, &imageCount, nullptr));
        std::vector<VkImage> images(imageCount);
        gvk_result(dispatchTableItr->second.gvkGetSwapchainImagesKHR(command.device, *command.pSwapchain, &imageCount, images.data()));

        // Use VkSwapchainKHR to look up associated VkImage ID collection
        std::vector<uint64_t>* pImageIds = nullptr;
        {
            std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
            pImageIds = &uniqueHandlesManager.swapchains[{ command.device, *command.pSwapchain }];
        }

        // Create VkImage IDs and add them to UniqueHandlesManager and ID collection
        for (const auto& image : images) {
            uint64_t id = 0;
            gvk_result(uniqueHandlesManager.on_create_object(&image, &id, false));
            pImageIds->push_back(id);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureGetSwapchainImagesKHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if application is ready to populate out parameter
        if (command.pSwapchainImageCount && command.pSwapchainImages) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // Use VkSwapchainKHR to look up associated VkImage ID collection
            std::vector<uint64_t>* pImageIds = nullptr;
            {
                std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
                pImageIds = &uniqueHandlesManager.swapchains[{ command.device, command.swapchain }];
            }

            // Mark each VkImage handle to be rewrapped with VkImage ID
            auto swapchainImageCount = std::min(*command.pSwapchainImageCount, (uint32_t)pImageIds->size());
            for (uint32_t i = 0; i < swapchainImageCount; ++i) {
                uniqueHandlesManager.mark_for_rewrap(command.pSwapchainImages + i, (*pImageIds)[i]);
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_destroy_object(const GvkCommandStructureDestroySwapchainKHR& command)
{
    gvk_result_scope_begin(VK_SUCCESS) {

        // Get UniqueHandlesManager
        auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
        gvk_result_assert(uniqueHandlesManager.enabled);

        // Unwrap VkSwapchainKHR
        VkSwapchainKHR swapchain = command.swapchain;
        gvk_result(uniqueHandlesManager.unwrap_handle(VK_OBJECT_TYPE_SWAPCHAIN_KHR, &swapchain));
        gvk_result_assert(swapchain);

        // Use VkSwapchainKHR to look up associated VkImage ID collection
        std::vector<uint64_t>* pImageIds = nullptr;
        {
            std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
            pImageIds = &uniqueHandlesManager.swapchains[{ command.device, swapchain }];
        }

        // Remove each VkImage ID from UniqueHandlesManager
        for (const auto& imageId : *pImageIds) {
            gvk_result(uniqueHandlesManager.on_destroy_object((VkImage)imageId));
        }

        // Remove VkImage ID collection from UniqueHandlesManager
        {
            std::lock_guard<std::mutex> lock(uniqueHandlesManager.mutex);
            uniqueHandlesManager.swapchains.erase({ command.device, swapchain });
        }

        // Remove VkSwapchainKHR ID from UniqueHandlesManager
        gvk_result(uniqueHandlesManager.on_destroy_object(command.swapchain));
    } gvk_result_scope_end;
    return gvkResult;
}

////////////////////////////////////////////////////////////////////////////////
VkResult on_handle_out(const GvkCommandStructureAcquirePerformanceConfigurationINTEL& command)
{
    gvk_result_scope_begin(command.result) {
        gvk_result(VK_ERROR_FEATURE_NOT_PRESENT);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureEnumeratePhysicalDeviceGroups& command)
{
    (void)command;
    // NOOP : Dispatchable handle
    return VK_SUCCESS;
}

VkResult on_handle_out(const GvkCommandStructureEnumeratePhysicalDeviceGroupsKHR& command)
{
    (void)command;
    // NOOP : Dispatchable handle
    return VK_SUCCESS;
}

VkResult on_handle_out(const GvkCommandStructureEnumeratePhysicalDevices& command)
{
    (void)command;
    // NOOP : Dispatchable handle
    return VK_SUCCESS;
}

VkResult on_handle_out(const GvkCommandStructureGetDeviceQueue& command)
{
    (void)command;
    // NOOP : Dispatchable handle
    return VK_SUCCESS;
}

VkResult on_handle_out(const GvkCommandStructureGetDeviceQueue2& command)
{
    (void)command;
    // NOOP : Dispatchable handle
    return VK_SUCCESS;
}

VkResult on_handle_out(const GvkCommandStructureGetDisplayModeProperties2KHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if application is ready to populate out parameter
        if (command.pPropertyCount && command.pProperties) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // Process out parameter
            for (uint32_t i = 0; i < *command.pPropertyCount; ++i) {
                const auto& displayModeProperties = command.pProperties[i].displayModeProperties;
                auto displayMode = displayModeProperties.displayMode;
                (void)displayMode;
                // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
                //  they'll have unique handles that persist for the life of the application anyway,
                //  but it will be better to wrap them like everything else.  Need to setup tests.
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureGetDisplayModePropertiesKHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if application is ready to populate out parameter
        if (command.pPropertyCount && command.pProperties) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // Process out parameter
            for (uint32_t i = 0; i < *command.pPropertyCount; ++i) {
                const auto& displayModeProperties = command.pProperties[i];
                auto displayMode = displayModeProperties.displayMode;
                (void)displayMode;
                // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
                //  they'll have unique handles that persist for the life of the application anyway,
                //  but it will be better to wrap them like everything else.  Need to setup tests.
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureGetDisplayPlaneSupportedDisplaysKHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if application is ready to populate out parameter
        if (command.pDisplayCount && command.pDisplays) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // Process out parameter
            for (uint32_t i = 0; i < *command.pDisplayCount; ++i) {
                auto display = command.pDisplays[i];
                (void)display;
                // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
                //  they'll have unique handles that persist for the life of the application anyway,
                //  but it will be better to wrap them like everything else.  Need to setup tests.
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureGetDrmDisplayEXT& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Process out parameter
        if (command.display && *command.display) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
            //  they'll have unique handles that persist for the life of the application anyway,
            //  but it will be better to wrap them like everything else.  Need to setup tests.
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureGetPhysicalDeviceDisplayPlaneProperties2KHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if application is ready to populate out parameter
        if (command.pPropertyCount && command.pProperties) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // Process out parameter
            for (uint32_t i = 0; i < *command.pPropertyCount; ++i) {
                const auto& displayPlaneProperties = command.pProperties[i];
                auto display = displayPlaneProperties.displayPlaneProperties.currentDisplay;
                (void)display;
                // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
                //  they'll have unique handles that persist for the life of the application anyway,
                //  but it will be better to wrap them like everything else.  Need to setup tests.
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureGetPhysicalDeviceDisplayPlanePropertiesKHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if application is ready to populate out parameter
        if (command.pPropertyCount && command.pProperties) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // Process out parameter
            for (uint32_t i = 0; i < *command.pPropertyCount; ++i) {
                const auto& displayPlaneProperties = command.pProperties[i];
                auto display = displayPlaneProperties.currentDisplay;
                (void)display;
                // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
                //  they'll have unique handles that persist for the life of the application anyway,
                //  but it will be better to wrap them like everything else.  Need to setup tests.
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureGetPhysicalDeviceDisplayProperties2KHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if application is ready to populate out parameter
        if (command.pPropertyCount && command.pProperties) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // Process out parameter
            for (uint32_t i = 0; i < *command.pPropertyCount; ++i) {
                const auto& displayProperties = command.pProperties[i];
                auto display = displayProperties.displayProperties.display;
                (void)display;
                // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
                //  they'll have unique handles that persist for the life of the application anyway,
                //  but it will be better to wrap them like everything else.  Need to setup tests.
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureGetPhysicalDeviceDisplayPropertiesKHR& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if application is ready to populate out parameter
        if (command.pPropertyCount && command.pProperties) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // Process out parameter
            for (uint32_t i = 0; i < *command.pPropertyCount; ++i) {
                const auto& displayProperties = command.pProperties[i];
                auto display = displayProperties.display;
                (void)display;
                // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
                //  they'll have unique handles that persist for the life of the application anyway,
                //  but it will be better to wrap them like everything else.  Need to setup tests.
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
VkResult on_handle_out(const GvkCommandStructureGetRandROutputDisplayEXT& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Process out parameter
        if (command.pDisplay && *command.pDisplay) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
            //  they'll have unique handles that persist for the life of the application anyway,
            //  but it will be better to wrap them like everything else.  Need to setup tests.
        }
    } gvk_result_scope_end;
    return gvkResult;
}
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult on_handle_out(const GvkCommandStructureGetWinrtDisplayNV& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Process out parameter
        if (command.pDisplay && *command.pDisplay) {

            // Get UniqueHandlesManager
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);

            // TODO : Handle VkDisplayModeKHR and VkDisplayModeKHR.  There's a very good chance
            //  they'll have unique handles that persist for the life of the application anyway,
            //  but it will be better to wrap them like everything else.  Need to setup tests.
        }
    } gvk_result_scope_end;
    return gvkResult;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

VkResult on_handle_out(const GvkCommandStructureRegisterDeviceEventEXT& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if VkFence was successfully created
        if (command.pFence && *command.pFence) {

            // Get UniqueHandlesManager and create new handle wrapper
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);
            gvk_result(uniqueHandlesManager.on_create_object(command.pFence));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult on_handle_out(const GvkCommandStructureRegisterDisplayEventEXT& command)
{
    gvk_result_scope_begin(command.result) {

        // TODO : Need to actually set result in execute_command_structure()
        gvk_result(command.result);

        // Check if VkFence was successfully created
        if (command.pFence && *command.pFence) {

            // Get UniqueHandlesManager and create new handle wrapper
            auto& uniqueHandlesManager = Registry::get().uniqueHandlesManager;
            gvk_result_assert(uniqueHandlesManager.enabled);
            gvk_result(uniqueHandlesManager.on_create_object(command.pFence));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace unique_handles
} // namespace hooks
} // namespace layer
} // namespace gvk
