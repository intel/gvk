
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

#include "gvk-containers/thread-safe-unordered-map.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-defines.hpp"
#include "gvk-environment.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-reference.hpp"
#include "gvk-structures.hpp"

#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace std {

template <>
struct hash<std::pair<VkObjectType, uint64_t>>
{
    inline size_t operator()(const std::pair<VkObjectType, uint64_t>& vkObject) const
    {
        std::hash<uint64_t> hasher;
        auto h0 = hasher((uint64_t)vkObject.first);
        auto h1 = hasher(vkObject.second);
        return h0 ^ (h1 << 1);
    }
};

} // namespace std

namespace gvk {
namespace layer {

inline bool is_dispatchable(VkObjectType oType)
{
    return
        oType == VK_OBJECT_TYPE_INSTANCE ||
        oType == VK_OBJECT_TYPE_PHYSICAL_DEVICE ||
        oType == VK_OBJECT_TYPE_DEVICE ||
        oType == VK_OBJECT_TYPE_QUEUE ||
        oType == VK_OBJECT_TYPE_COMMAND_BUFFER;
}

class UniqueHandlesManager final
{
public:
    static constexpr gvk::Printer::Flags PrinterFlags{ gvk::Printer::Default & ~gvk::Printer::EnumValue };

    UniqueHandlesManager()
    {
        logUnwrappedHandles = gvk::get_env_var_true("GVK_LOG_UNWRAPPED_HANDLES");
    }

    template <typename CommandType>
    inline void set_current_command(const CommandType& currentCommand)
    {
        currentCommandLogged = false;
        pCurrentCommand = (const GvkCommandBaseStructure*)&currentCommand;
    }

    inline void clear_current_command()
    {
        currentCommandLogged = false;
        pCurrentCommand = nullptr;
    }

    template <typename VkHandleType>
    inline VkResult mark_for_rewrap(VkHandleType* pHandle, uint64_t id)
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result_assert(pHandle);
            get_thread_local_transient_handles().push_back({ (uint64_t*)pHandle, id });
        } gvk_result_scope_end;
        return gvkResult;
    }

    template <typename VkHandleType>
    inline VkResult on_create_object(const VkHandleType* pHandle, uint64_t* pId = nullptr, bool markForRewrap = true)
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result_assert(enabled);
            auto oType = gvk::detail::get_object_type<VkHandleType>();
            gvk_result_assert(oType);
            gvk_result_assert(pHandle);
            if (*pHandle) {

                // If VkHandleType is dispatchable, just use the handle, otherwise get next ID
                uint64_t id = is_dispatchable(oType) ? (uint64_t)*pHandle : get_next_id();

                // Map type + ID to handle
                // NOTE : That this mapping could handle duplicate IDs across different types,
                //  but that would mean any consuming code would have to do so as well to use
                //  handles as map keys and whatnot.  What it really does is ensure that things
                //  still work in the unlikely scenario of a collision between a dispatchable
                //  handle and generated ID.
                auto inserted = ids.insert({ { oType, id }, (uint64_t)*pHandle }).second;
                gvk_result_assert(inserted);

                // Set the ID out parameter
                if (pId) {
                    *pId = id;
                }

                // If marked for rewrap, mark the handle's address to be rewritten with the ID
                if (markForRewrap) {
                    gvk_result(mark_for_rewrap((uint64_t*)pHandle, id));
                }
            }
        } gvk_result_scope_end;
        return gvkResult;
    }

    template <typename VkHandleType>
    inline VkResult on_destroy_object(VkHandleType handle)
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result_assert(enabled);
            auto oType = gvk::detail::get_object_type<VkHandleType>();
            gvk_result_assert(oType);

            // Remove object from ID map
            auto erased = ids.erase({ oType, (uint64_t)handle });
            if (!erased && logUnwrappedHandles && !is_dispatchable(oType)) {
                log_unwrapped_handle(oType, handle);
            }
        } gvk_result_scope_end;
        return gvkResult;
    }

    template <typename VkHandleType>
    inline VkResult unwrap_handle(VkObjectType oType, VkHandleType* pHandle)
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result_assert(enabled);
            gvk_result_assert(oType);

            // Get handle from ID map
            if (!is_dispatchable(oType) && pHandle && *pHandle) {
                auto handle = ids.get({ oType, (uint64_t)*pHandle });
                if (handle) {
                    *pHandle = (VkHandleType)handle;
                } else if (logUnwrappedHandles) {
                    log_unwrapped_handle(oType, *pHandle);
                }
            }
        } gvk_result_scope_end;
        return gvkResult;
    }

    template <typename StructureType>
    inline VkResult unwrap_handles(StructureType* pStrcuture)
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result_assert(enabled);
            gvk_result_assert(pStrcuture);

            // Clear storage used for rewrapping handles
            get_thread_local_transient_handles().clear();

            // Enumerate handles in given structure
            gvk::detail::enumerate_structure_handles(
                *pStrcuture,
                [&](VkObjectType oType, const uint64_t& id)
                {
                    // NOTE : Can't gvk_result_assert() in lambda, so use gvkResult directly
                    if (oType == VK_OBJECT_TYPE_UNKNOWN) {
                        gvkResult = VK_ERROR_UNKNOWN;
                    }

                    // If object is valid and not dispatchable, unwrap it
                    if (oType && id && !is_dispatchable(oType)) {

                        // Take address of handle to overwrite
                        auto pMutableHandle = const_cast<uint64_t*>(&id);

                        // Mark for rewrap so that handle will be reset after command execution
                        auto markForRewrapResult = mark_for_rewrap(pMutableHandle, id);
                        if (markForRewrapResult != VK_SUCCESS) {
                            gvkResult = markForRewrapResult;
                        }

                        // Overwrite the incoming wrapped handle with the actual unwrapped handle
                        auto unwrapResult = unwrap_handle(oType, pMutableHandle);
                        if (unwrapResult != VK_SUCCESS) {
                            gvkResult = unwrapResult;
                        }
                    }
                }
            );
        } gvk_result_scope_end;
        return gvkResult;
    }

    inline VkResult rewrap_handles()
    {
        gvk_result_scope_begin(VK_SUCCESS) {
            gvk_result_assert(enabled);

            // Loop over all the handles that were marked for rewrap and rewrap them
            for (const auto& handle : get_thread_local_transient_handles()) {
                gvk_result_assert(handle.first);
                gvk_result_assert(handle.second);
                *handle.first = handle.second;
            }

            // Clear storage used for rewrapping handles
            get_thread_local_transient_handles().clear();
        } gvk_result_scope_end;
        return gvkResult;
    }

    VkBool32 enabled{ };
    VkBool32 logUnwrappedHandles{ };
    VkBool32 currentCommandLogged{ };
    const GvkCommandBaseStructure* pCurrentCommand{ };
    std::unordered_map<void*, DispatchTable> VkInstanceDispatchTables;
    std::unordered_map<void*, DispatchTable> VkDeviceDispatchTables;
    gvk::ThreadSafeUnorderedMap<std::pair<VkObjectType, uint64_t>, uint64_t> ids;
    std::unordered_map<gvk::HandleId<VkDevice, VkDescriptorPool>, std::unordered_set<uint64_t>> descriptorPools;
    std::unordered_map<gvk::HandleId<VkDevice, VkSwapchainKHR>, std::vector<uint64_t>> swapchains;
    std::unordered_map<gvk::HandleId<VkPhysicalDevice, VkDisplayKHR>, uint64_t> displays;
    std::mutex mutex;

private:
    inline uint64_t get_next_id()
    {
        static std::atomic<uint64_t> sId;
        return ++sId;
    }

    inline std::vector<std::pair<uint64_t*, uint64_t>>& get_thread_local_transient_handles()
    {
        thread_local std::vector<std::pair<uint64_t*, uint64_t>> tlHandles;
        return tlHandles;
    }

    inline void log_current_command()
    {
        if (pCurrentCommand && !currentCommandLogged) {
            currentCommandLogged = true;
            std::cout << gvk::to_string(*pCurrentCommand, PrinterFlags) << std::endl;
        }
    }

    template <typename VkHandleType>
    inline void log_unwrapped_handle(VkObjectType oType, VkHandleType handle)
    {
        log_current_command();
        std::cout << "Encountered unwrapped handle " << gvk::to_hex_string(handle) << " of type " << gvk::string::remove(gvk::to_string(oType, PrinterFlags), "\"") << "; note that this may not indicate an error" << std::endl;
    }

    UniqueHandlesManager(const UniqueHandlesManager&) = delete;
    UniqueHandlesManager& operator=(const UniqueHandlesManager&) = delete;
};

namespace hooks {
namespace unique_handles {

////////////////////////////////////////////////////////////////////////////////
// VkDescriptorPool/VkDescriptorSet
VkResult on_destroy_object(const GvkCommandStructureResetDescriptorPool& command);
VkResult on_destroy_object(const GvkCommandStructureDestroyDescriptorPool& command);
VkResult on_create_object(const GvkCommandStructureAllocateDescriptorSets& command);
VkResult on_destroy_object(const GvkCommandStructureFreeDescriptorSets& command);

////////////////////////////////////////////////////////////////////////////////
// VkPipelineBinaryKHR
VkResult on_create_object(const GvkCommandStructureCreatePipelineBinariesKHR& command);

////////////////////////////////////////////////////////////////////////////////
// VkSwapchainKHR
VkResult on_create_object(const GvkCommandStructureCreateSwapchainKHR& command);
VkResult on_handle_out(const GvkCommandStructureGetSwapchainImagesKHR& command);
VkResult on_destroy_object(const GvkCommandStructureDestroySwapchainKHR& command);

////////////////////////////////////////////////////////////////////////////////
VkResult on_handle_out(const GvkCommandStructureAcquirePerformanceConfigurationINTEL& command);
VkResult on_handle_out(const GvkCommandStructureEnumeratePhysicalDeviceGroups& command);
VkResult on_handle_out(const GvkCommandStructureEnumeratePhysicalDeviceGroupsKHR& command);
VkResult on_handle_out(const GvkCommandStructureEnumeratePhysicalDevices& command);
VkResult on_handle_out(const GvkCommandStructureGetDeviceQueue& command);
VkResult on_handle_out(const GvkCommandStructureGetDeviceQueue2& command);
VkResult on_handle_out(const GvkCommandStructureGetDisplayModeProperties2KHR& command);
VkResult on_handle_out(const GvkCommandStructureGetDisplayModePropertiesKHR& command);
VkResult on_handle_out(const GvkCommandStructureGetDisplayPlaneSupportedDisplaysKHR& command);
VkResult on_handle_out(const GvkCommandStructureGetDrmDisplayEXT& command);
VkResult on_handle_out(const GvkCommandStructureGetPhysicalDeviceDisplayPlaneProperties2KHR& command);
VkResult on_handle_out(const GvkCommandStructureGetPhysicalDeviceDisplayPlanePropertiesKHR& command);
VkResult on_handle_out(const GvkCommandStructureGetPhysicalDeviceDisplayProperties2KHR& command);
VkResult on_handle_out(const GvkCommandStructureGetPhysicalDeviceDisplayPropertiesKHR& command);
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
VkResult on_handle_out(const GvkCommandStructureGetRandROutputDisplayEXT& command);
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult on_handle_out(const GvkCommandStructureGetWinrtDisplayNV& command);
#endif // VK_USE_PLATFORM_WIN32_KHR
VkResult on_handle_out(const GvkCommandStructureRegisterDeviceEventEXT& command);
VkResult on_handle_out(const GvkCommandStructureRegisterDisplayEventEXT& command);

} // namespace unique_handles
} // namespace hooks
} // namespace layer
} // namespace gvk
