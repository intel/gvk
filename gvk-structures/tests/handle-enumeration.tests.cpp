
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

#define _CRT_SECURE_NO_WARNINGS

#include "gvk-structures/generated/core-structure-enumerate-handles.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

TEST(enumerate_structure_handles, Basic)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo { };
    commandBufferAllocateInfo.commandPool = (VkCommandPool)64;
    gvk::detail::enumerate_structure_handles(
        commandBufferAllocateInfo,
        [](VkObjectType objectType, const uint64_t& handle)
        {
            EXPECT_EQ(objectType, VK_OBJECT_TYPE_COMMAND_POOL);
            EXPECT_EQ(handle, 64);
            const_cast<uint64_t&>(handle) = 128;
        }
    );
    EXPECT_EQ(commandBufferAllocateInfo.commandPool, (VkCommandPool)128);
}
