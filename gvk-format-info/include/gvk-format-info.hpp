
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

#include "gvk-defines.hpp"
#include "gvk-format-info/generated/enumerate-formats.hpp"
#include "gvk-format-info/generated/format-info.h"
#include "gvk-format-info/generated/format-info-enumerations-to-string.hpp"
#include "gvk-format-info/generated/format-info-structure-comparison-operators.hpp"
#include "gvk-format-info/generated/format-info-structure-create-copy.hpp"
#include "gvk-format-info/generated/format-info-structure-deserialization.hpp"
#include "gvk-format-info/generated/format-info-structure-destroy-copy.hpp"
#include "gvk-format-info/generated/format-info-structure-get-stype.hpp"
#include "gvk-format-info/generated/format-info-structure-make-tuple.hpp"
#include "gvk-format-info/generated/format-info-structure-serialization.hpp"
#include "gvk-format-info/generated/format-info-structure-to-string.hpp"
#include "gvk-structures/defaults.hpp"

namespace gvk {

/**
Gets the FormatInfo for a specified VkFormat
@param [in] format The VkFormat to get FormatInfo for
@param [out] pFormatInfo The FormatInfo for the specified VkFormat
*/
void get_format_info(VkFormat format, GvkFormatInfo* pFormatInfo);

/**
Gets the VkImageAspectFlags for a specified VkFormat
@param [in] format The VkFormat to get VkImageAspectFlags for
@return The VkImageAspectFlags for the specified VkFormat
*/
VkImageAspectFlags get_image_aspect_flags(VkFormat format);

/**
Gets the nubmer of bits per texel (if applicable) of a specific VkFormat
@param [in] format The VkFormat to get bits per texel for
@return The bits per texel of the specified VkFormat
*/
uint32_t get_bits_per_texel(VkFormat format);

/**
Gets the nubmer of bytes per texel (if applicable) of a specific VkFormat
@param [in] format The VkFormat to get bytes per texel for
@return The bytes per texel of the specified VkFormat
*/
uint32_t get_bytes_per_texel(VkFormat format);

/**
Executes a given function for every VkFormat the fulfills the provided criteria
@typename <ProcessFormatFunctionType> The type of function to execute for each VkFormat the fulfills the provided criteria
    @note The function type must accept a single VkFormat argument and return a bool indicating whether or not to continue enumerating
@param [in] pfnVkGetPhysicalDeviceFormatProperties2 A pointer to vkGetPhysicalDeviceFormatProperties2
@param [in] vkPhysicalDevice The VkPhysicalDevice to get VkFormat properties from
@param [in] imageTiling The VkImageTiling features to check for support
@param [in] featureFlags The VkFormatFeatureFlags2 to check for support
@param [in] processFormat The function to execute for each VkFormat that fulfills the provided criteria
*/
template <typename ProcessFormatFunctionType>
inline void enumerate_formats(
    PFN_vkGetPhysicalDeviceFormatProperties2 pfnVkGetPhysicalDeviceFormatProperties2,
    VkPhysicalDevice vkPhysicalDevice,
    VkImageTiling imageTiling,
    VkFormatFeatureFlags2 featureFlags,
    ProcessFormatFunctionType processFormat
)
{
    assert(pfnVkGetPhysicalDeviceFormatProperties2);
    assert(vkPhysicalDevice);
    enumerate_formats(
        [&](VkFormat format)
        {
            auto formatProperties3 = get_default<VkFormatProperties3>();
            auto formatProperties2 = get_default<VkFormatProperties2>();
            formatProperties2.pNext = &formatProperties3;
            pfnVkGetPhysicalDeviceFormatProperties2(vkPhysicalDevice, format, &formatProperties2);
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

} // namespace gvk
