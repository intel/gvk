
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

#include "gvk-reference/handle-id.hpp"
#include "gvk-reference/reference.hpp"

#include <cstddef>
#include <cstdint>

#define gvk_reference_type(GVK_REFERENCE_TYPE)                           \
public:                                                                  \
    GVK_REFERENCE_TYPE() = default;                                      \
    inline GVK_REFERENCE_TYPE(std::nullptr_t) { };                       \
    inline GVK_REFERENCE_TYPE(gvk::nullref_t) { };                       \
    GVK_REFERENCE_TYPE(const GVK_REFERENCE_TYPE&) = default;             \
    GVK_REFERENCE_TYPE(GVK_REFERENCE_TYPE&&) = default;                  \
    GVK_REFERENCE_TYPE& operator=(const GVK_REFERENCE_TYPE&) = default;  \
    GVK_REFERENCE_TYPE& operator=(GVK_REFERENCE_TYPE&&) = default;       \
    inline operator bool() const { return mReference; }                  \
private:                                                                 \
    gvk::Reference<ControlBlock> mReference;
