
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

#ifdef GVK_COMPILER_MSVC
#pragma warning(push, 0)
#endif // GVK_COMPILER_MSVC
#ifdef GVK_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif // GVK_COMPILER_GCC
#ifdef GVK_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wnullability-extension"
#endif // GVK_COMPILER_CLANG
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"
#ifdef GVK_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // GVK_COMPILER_CLANG
#ifdef GVK_COMPILER_GCC
#pragma GCC diagnostic pop
#endif // GVK_COMPILER_GCC
#ifdef GVK_COMPILER_MSVC
#pragma warning(pop)
#endif // GVK_COMPILER_MSVC
