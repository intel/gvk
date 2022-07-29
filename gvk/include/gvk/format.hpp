
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

#include "gvk/generated/dispatch-table.hpp"
#include "gvk/generated/format-utilities.hpp"
#include "gvk/defines.hpp"

#include <array>
#include <cassert>
#include <set>
#include <string>
#include <vector>

namespace gvk {

/**
Parameters describing a specific VkFormat
*/
struct FormatInfo
{
    /**
    Parameters describing a single plane of a specific planar VkFormat
    */
    struct Plane
    {
        uint32_t index{ };
        uint32_t widthDivisor{ };
        uint32_t heightDivisor{ };
        VkFormat compatible{ VK_FORMAT_UNDEFINED };
    };

    /**
    Parameters describing a single channel component of a specific VkFormat
    */
    struct Component
    {
        ComponentName name{ ComponentName::CN_None };
        uint32_t bits{ };
        NumericFormat numericFormat{ NumericFormat::NF_None };
        uint32_t planeIndex{ };
    };

    /**
    Gets the nubmer of bits per pixel (if applicable) of a specific VkFormat
    */
    uint32_t bits_per_pixel() const;

    std::set<FormatClass> classes;
    uint32_t blockSize{ };
    uint32_t texelsPerBlock{ };
    uint32_t chroma{ };
    uint32_t packed{ };
    std::array<uint32_t, 3> blockExtent{ };
    CompressionType compressionType{ CompressionType::CT_None };
    NumericFormat numericFormat{ NumericFormat::NF_None };
    std::string spirvImageFormat;
    std::vector<Plane> planes;
    std::vector<Component> components;
};

/**
Gets the FormatInfo for a specified VkFormat
@param [in] format The VkFormat to get FormatInfo for
@return The FormatInfo for the specified VkFormat
*/
const FormatInfo& get_format_info(VkFormat format);

/**
Gets the VkImageAspectFlags for a specified VkFormat
@param [in] format The VkFormat to get VkImageAspectFlags for
@return The VkImageAspectFlags for the specified VkFormat
*/
VkImageAspectFlags get_image_aspect_flags(VkFormat format);

/**
Executes a given function for every VkFormat the fulfills the provided criteria
@typename <ProcessFormatFunctionType> The type of function to execute for each VkFormat the fulfills the provided criteria
    @note The function type must accept a single VkFormat argument and return a bool indicating whether or not to continue enumerating
@param [in] vkPhysicalDevice The VkPhysicalDevice to get VkFormat properties from
@param [in] imageTiling The VkImageTiling features to check for support
@param [in] featureFlags The VkFormatFeatureFlags2 to check for support
@param [in] processFormat The function to execute for each VkFormat that fulfills the provided criteria
*/
template <typename ProcessFormatFunctionType>
inline void enumerate_formats(
    VkPhysicalDevice vkPhysicalDevice,
    VkImageTiling imageTiling,
    VkFormatFeatureFlags2 featureFlags,
    ProcessFormatFunctionType processFormat
)
{
    if (vkPhysicalDevice) {
        enumerate_formats(
            [&](VkFormat format)
            {
                auto formatProperties3 = get_default<VkFormatProperties3>();
                auto formatProperties2 = get_default<VkFormatProperties2>();
                formatProperties2.pNext = &formatProperties3;
                assert(gDispatchTable.gvkGetPhysicalDeviceFormatProperties2);
                gDispatchTable.gvkGetPhysicalDeviceFormatProperties2(vkPhysicalDevice, format, &formatProperties2);
                switch (imageTiling) {
                case VK_IMAGE_TILING_OPTIMAL: {
                    if (formatProperties3.optimalTilingFeatures & featureFlags) {
                        return processFormat(format);
                    }
                } break;
                case VK_IMAGE_TILING_LINEAR: {
                    if (formatProperties3.linearTilingFeatures & featureFlags) {
                        return processFormat(format);
                    }
                } break;
                default: {
                } break;
                }
                return true;
            }
        );
    }
}

} // namespace gvk
