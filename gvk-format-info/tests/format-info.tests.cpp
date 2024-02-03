
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
#include "gvk-structures.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

#include <iostream>
#include <vector>

TEST(GvkFormatInfo, get_bits_per_texel_linear)
{
    std::vector<VkFormat> mismatches;
    gvk::enumerate_formats(
        [&](VkFormat format)
        {
            GvkFormatInfo formatInfo { };
            gvk::get_format_info(format, &formatInfo);
            if (!formatInfo.packed && !formatInfo.compressionType) {
                uint32_t bitsPerTexel = 0;
                for (uint32_t i = 0; i < formatInfo.componentCount; ++i) {
                    bitsPerTexel += formatInfo.pComponents[i].bits;
                }
                if (bitsPerTexel != (format ? formatInfo.blockSize * 8 / formatInfo.texelsPerBlock : 0) ||
                    bitsPerTexel != gvk::get_bits_per_texel(format)) {
                    mismatches.push_back(format);
                }
            }
            return true;
        }
    );
    EXPECT_TRUE(mismatches.empty());
}

TEST(GvkFormatInfo, get_bits_per_texel_packed)
{
    std::vector<VkFormat> mismatches;
    gvk::enumerate_formats(
        [&](VkFormat format)
        {
            GvkFormatInfo formatInfo { };
            gvk::get_format_info(format, &formatInfo);
            if (formatInfo.packed && (formatInfo.compressionType || gvk::get_bits_per_texel(format) != formatInfo.packed)) {
                mismatches.push_back(format);
            }
            return true;
        }
    );
    EXPECT_TRUE(mismatches.empty());
}

TEST(GvkFormatInfo, get_bits_per_texel_compressed)
{
    std::vector<VkFormat> mismatches;
    gvk::enumerate_formats(
        [&](VkFormat format)
        {
            GvkFormatInfo formatInfo { };
            gvk::get_format_info(format, &formatInfo);
            auto bitsPerTexel = format ? formatInfo.blockSize * 8 / formatInfo.texelsPerBlock : 0;
            if (formatInfo.compressionType && (formatInfo.packed || gvk::get_bits_per_texel(format) != bitsPerTexel)) {
                mismatches.push_back(format);
            }
            return true;
        }
    );
    EXPECT_TRUE(mismatches.empty());
}
