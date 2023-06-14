

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

#define _CRT_SECURE_NO_WARNINGS

#include "gvk-structures/serialization.hpp"
#include "gvk-structures/to-string.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

#include <sstream>

namespace gvk {
namespace validation {

template <typename VulkanStructureType>
inline void validate_structure_serialization(const VulkanStructureType& obj, bool printSerializedBytes = false)
{
    std::stringstream strStrm(std::ios::binary | std::ios::in | std::ios::out);
    gvk::serialize(strStrm, obj);
    if (printSerializedBytes) {
        std::vector<uint8_t> serialized((std::istream_iterator<uint8_t>(strStrm)), std::istream_iterator<uint8_t>());
        std::stringstream message;
        message << serialized.size() << " bytes { ";
        for (auto byte : serialized) {
            message << byte << " ";
        }
        message << "}";
        FAIL() << message.str();
    } else {
        // TODO : VkAllocationCallbacks need to be hooked up in gvk::Auto<>, then hook
        //  up gvk::validation::Allocator here
        gvk::Auto<VulkanStructureType> deserialized;
        gvk::deserialize(strStrm, nullptr, deserialized);
        if (deserialized != obj) {
            FAIL()
                << "===============================================================================" << std::endl
                << gvk::to_string(obj) << std::endl
                << "-------------------------------------------------------------------------------" << std::endl
                << gvk::to_string(deserialized) << std::endl
                << "===============================================================================" << std::endl
                << std::endl;
        }
    }
}

} // namespace validation
} // namespace gvk
