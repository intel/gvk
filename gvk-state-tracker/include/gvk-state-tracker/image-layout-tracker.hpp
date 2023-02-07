
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

#include <vector>

namespace gvk {
namespace state_tracker {

class ImageLayoutTracker final
{
public:
    ImageLayoutTracker() = default;
    ImageLayoutTracker(const ImageLayoutTracker& other) = default;
    ImageLayoutTracker& operator=(const ImageLayoutTracker& other) = default;
    ImageLayoutTracker(uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageLayout initialLayout);
    const VkImageLayout& operator[](const VkImageSubresource& imageSubresource) const;
    VkImageLayout& operator[](const VkImageSubresource& imageSubresource);

    uint32_t get_mip_level_count() const;
    uint32_t get_array_layer_count() const;
    uint32_t get_subresource_count() const;
    void get_image_layouts(const VkImageSubresourceRange& imageSubresourceRange, VkImageLayout* pImageLayout);
    void set_image_layouts(const VkImageSubresourceRange& imageSubresourceRange, VkImageLayout imageLayout);

    template <typename ProcessSubresourceImageLayoutFunctionType>
    inline void enumerate(const VkImageSubresourceRange& imageSubresourceRange, ProcessSubresourceImageLayoutFunctionType processSubresourceImageLayout) const
    {
        if (imageSubresourceRange.levelCount && imageSubresourceRange.layerCount) {
            uint32_t lastMipLevel = imageSubresourceRange.baseMipLevel + imageSubresourceRange.levelCount - 1;
            uint32_t lastArrayLAyer = imageSubresourceRange.baseArrayLayer + imageSubresourceRange.layerCount - 1;
            for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer <= lastArrayLAyer && arrayLayer < mArrayLayerCount; ++arrayLayer) {
                for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel <= lastMipLevel && mipLevel < mMipLevelCount; ++mipLevel) {
                    VkImageSubresource imageSubresource { };
                    imageSubresource.mipLevel = mipLevel;
                    imageSubresource.arrayLayer = arrayLayer;
                    processSubresourceImageLayout(imageSubresource, operator[](imageSubresource));
                }
            }
        }
    }

    template <typename ProcessSubresourceImageLayoutFunctionType>
    inline void enumerate(const VkImageSubresourceRange& imageSubresourceRange, ProcessSubresourceImageLayoutFunctionType processSubresourceImageLayout)
    {
        const_cast<const ImageLayoutTracker*>(this)->enumerate(
            imageSubresourceRange,
            [&processSubresourceImageLayout](const VkImageSubresource& imageSubresource, const VkImageLayout& imageLayout)
            {
                processSubresourceImageLayout(imageSubresource, const_cast<VkImageLayout&>(imageLayout));
            }
        );
    }

private:
    uint32_t mMipLevelCount { 0 };
    uint32_t mArrayLayerCount { 0 };
    std::vector<VkImageLayout> mSubresourceImageLayouts;
};

} // namespace state_tracker
} // namespace gvk
