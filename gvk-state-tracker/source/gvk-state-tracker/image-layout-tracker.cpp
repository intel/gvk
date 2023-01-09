
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

#include "gvk-state-tracker/image-layout-tracker.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

/*

    The following layout is used when indexing individual VkImageSubresources.
    To calculate a particular VkImageSubresource index:

        imageSubresource.arrayLayer * mipLevelCount + imageSubresource.mipLevel

    The following diagram illustrates the layout of an ImageLayoutTracker with 4
    mip levels and 3 array layers:

        0           1       2     3

    0   +---------+ +-----+ +---+ +-+
        |0        | |1    | |2  | +-+
        |         | |     | +---+
        |         | +-----+
        |         |
        +---------+
    1   +---------+ +-----+ +---+ +-+
        |4        | |5    | |6  | +-+
        |         | |     | +---+
        |         | +-----+
        |         |
        +---------+
    2   +---------+ +-----+ +---+ +-+
        |8        | |9    | |10 | +-+
        |         | |     | +---+
        |         | +-----+
        |         |
        +---------+

*/

ImageLayoutTracker::ImageLayoutTracker(uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageLayout initialLayout)
    : mMipLevelCount { mipLevelCount }
    , mArrayLayerCount { arrayLayerCount }
    , mSubresourceImageLayouts(mipLevelCount * arrayLayerCount, initialLayout)
{
    assert(mMipLevelCount);
    assert(mArrayLayerCount);
}

const VkImageLayout& ImageLayoutTracker::operator[](const VkImageSubresource& imageSubresource) const
{
    assert(imageSubresource.mipLevel < mMipLevelCount);
    assert(imageSubresource.arrayLayer < mArrayLayerCount);
    auto index = imageSubresource.arrayLayer * mMipLevelCount + imageSubresource.mipLevel;
    assert(index < mSubresourceImageLayouts.size());
    return mSubresourceImageLayouts[index];
}

VkImageLayout& ImageLayoutTracker::operator[](const VkImageSubresource& imageSubresource)
{
    auto constThis = const_cast<const ImageLayoutTracker*>(this);
    return const_cast<VkImageLayout&>(constThis->operator[](imageSubresource));
}

uint32_t ImageLayoutTracker::get_mip_level_count() const
{
    return mMipLevelCount;
}

uint32_t ImageLayoutTracker::get_array_layer_count() const
{
    return mArrayLayerCount;
}

uint32_t ImageLayoutTracker::get_subresource_count() const
{
    return (uint32_t)mSubresourceImageLayouts.size();
}

void ImageLayoutTracker::get_image_layouts(const VkImageSubresourceRange& imageSubresourceRange, VkImageLayout* pImageLayout)
{
    enumerate(
        imageSubresourceRange,
        [&](const VkImageSubresource&, VkImageLayout& subresourceImageLayout)
        {
            *pImageLayout = subresourceImageLayout;
            ++pImageLayout;
        }
    );
}

void ImageLayoutTracker::set_image_layouts(const VkImageSubresourceRange& imageSubresourceRange, VkImageLayout imageLayout)
{
    enumerate(
        imageSubresourceRange,
        [&](const VkImageSubresource&, VkImageLayout& subresourceImageLayout)
        {
            subresourceImageLayout = imageLayout;
        }
    );
}

} // namespace state_tracker
} // namespace gvk
