
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

#include <algorithm>

namespace gvk {
namespace detail {

template <typename T>
struct ArrayTupleElementWrapper final
{
    inline const T* begin() const
    {
        return count && ptr ? ptr : nullptr;
    }

    inline const T* end() const
    {
        return count && ptr ? ptr + count : nullptr;
    }

    size_t count { };
    const T* ptr { };
};

template <typename T>
inline bool operator==(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
inline bool operator!=(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
inline bool operator<(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
inline bool operator>(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return rhs < lhs;
}

template <typename T>
inline bool operator<=(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return !(rhs < lhs);
}

template <typename T>
inline bool operator>=(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return !(lhs < rhs);
}

template <typename T>
struct PointerArrayTupleElementWrapper final
{
    inline const T* const* begin() const
    {
        return count && ptr ? ptr : nullptr;
    }

    inline const T* const* end() const
    {
        return count && ptr ? ptr + count : nullptr;
    }

    size_t count{ };
    const T* const* ptr{ };
};

template <typename T>
inline bool operator==(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return std::equal(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](auto lhsPtr, auto rhsPtr)
        {
            return ArrayTupleElementWrapper<T> { 1, lhsPtr } == ArrayTupleElementWrapper<T> { 1, rhsPtr };
        }
    );
}

template <typename T>
inline bool operator!=(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
inline bool operator<(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](auto lhsPtr, auto rhsPtr)
        {
            return ArrayTupleElementWrapper<T> { 1, lhsPtr } < ArrayTupleElementWrapper<T> { 1, rhsPtr };
        }
    );
}

template <typename T>
inline bool operator>(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return rhs < lhs;
}

template <typename T>
inline bool operator<=(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return !(rhs < lhs);
}

template <typename T>
inline bool operator>=(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return !(lhs < rhs);
}

struct StringTupleElementWrapper final
{
    const char* pStr { nullptr };
};

bool operator==(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator!=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator<(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator>(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator<=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator>=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);

struct StringArrayTupleElementWrapper final
{
    size_t count { };
    const char* const* ppStrs { nullptr };
};

bool operator==(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator!=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator<(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator>(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator<=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator>=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);

struct PNextTupleElementWrapper final
{
    const void* pNext { nullptr };
};

bool operator==(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator!=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator<(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator>(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator<=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator>=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);

} // namespace detail
} // namespace gvk
