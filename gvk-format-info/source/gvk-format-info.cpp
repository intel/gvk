
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

#include "gvk-format-info.hpp"

namespace gvk {

VkImageAspectFlags get_image_aspect_flags(VkFormat format)
{
    GvkFormatInfo formatInfo = { };
    get_format_info(format, &formatInfo);
    VkImageAspectFlags imageAspectFlags = VK_IMAGE_ASPECT_NONE;
    for (uint32_t i = 0; i < formatInfo.componentCount; ++i) {
        switch (formatInfo.pComponents[i].name) {
        case GVK_FORMAT_COMPONENT_NAME_A:
        case GVK_FORMAT_COMPONENT_NAME_R:
        case GVK_FORMAT_COMPONENT_NAME_G:
        case GVK_FORMAT_COMPONENT_NAME_B: {
            imageAspectFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
        } break;
        case GVK_FORMAT_COMPONENT_NAME_D: {
            imageAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        } break;
        case GVK_FORMAT_COMPONENT_NAME_S: {
            imageAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        } break;
        default: {
        } break;
        }
    }
    return imageAspectFlags;
}

uint32_t get_bits_per_texel(VkFormat format)
{
    GvkFormatInfo formatInfo = { };
    get_format_info(format, &formatInfo);
    uint32_t bitPerTexel = 0;
    for (uint32_t i = 0; i < formatInfo.componentCount; ++i) {
        bitPerTexel += formatInfo.pComponents[i].bits;
    }
    return bitPerTexel;
}

uint32_t get_bytes_per_texel(VkFormat format)
{
    return get_bits_per_texel(format) / 8;
}

} // namespace gvk
