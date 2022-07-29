
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

#include "gvk/format.hpp"

#include <cassert>

namespace gvk {

uint32_t FormatInfo::bits_per_pixel() const
{
    uint32_t bitsPerPixel = 0;
    for (const auto& component : components) {
        bitsPerPixel += component.bits;
    }
    return bitsPerPixel;
}

VkImageAspectFlags get_image_aspect_flags(VkFormat format)
{
    VkImageAspectFlags imageAspectFlags = VK_IMAGE_ASPECT_NONE;
    for (const auto& component : get_format_info(format).components) {
        switch (component.name) {
        case ComponentName::CN_A:
        case ComponentName::CN_R:
        case ComponentName::CN_G:
        case ComponentName::CN_B: {
            imageAspectFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
        } break;
        case ComponentName::CN_D: {
            imageAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        } break;
        case ComponentName::CN_S: {
            imageAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        } break;
        default: {
        } break;
        }
    }
    return imageAspectFlags;
}

} // namespace gvk
