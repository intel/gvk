
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
