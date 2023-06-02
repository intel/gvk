
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

#include "gvk-structures/detail/make-tuple-utilities.hpp"

#include <string_view>

namespace gvk {
namespace detail {

bool operator==(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs)
{
    return lhs.pStr && rhs.pStr ? std::string_view(lhs.pStr) == std::string_view(rhs.pStr) : !lhs.pStr && !rhs.pStr;
}

bool operator!=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs)
{
    return lhs.pStr && rhs.pStr ? std::string_view(lhs.pStr) < std::string_view(rhs.pStr) : !lhs.pStr && rhs.pStr;
}

bool operator>(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs)
{
    return rhs < lhs;
}

bool operator<=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs)
{
    return !(rhs < lhs);
}

bool operator>=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs)
{
    return !(lhs < rhs);
}

bool operator==(const WStringTupleElementWrapper& lhs, const WStringTupleElementWrapper& rhs)
{
    return lhs.pwStr && rhs.pwStr ? std::wstring_view(lhs.pwStr) == std::wstring_view(rhs.pwStr) : !lhs.pwStr && !rhs.pwStr;
}

bool operator!=(const WStringTupleElementWrapper& lhs, const WStringTupleElementWrapper& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const WStringTupleElementWrapper& lhs, const WStringTupleElementWrapper& rhs)
{
    return lhs.pwStr && rhs.pwStr ? std::wstring_view(lhs.pwStr) < std::wstring_view(rhs.pwStr) : !lhs.pwStr && rhs.pwStr;
}

bool operator>(const WStringTupleElementWrapper& lhs, const WStringTupleElementWrapper& rhs)
{
    return rhs < lhs;
}

bool operator<=(const WStringTupleElementWrapper& lhs, const WStringTupleElementWrapper& rhs)
{
    return !(rhs < lhs);
}

bool operator>=(const WStringTupleElementWrapper& lhs, const WStringTupleElementWrapper& rhs)
{
    return !(lhs < rhs);
}

bool operator==(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs)
{
    static_assert(sizeof(StringTupleElementWrapper) == sizeof(const char*));
    auto pLhsWrapper = (const StringTupleElementWrapper*)lhs.ppStrs;
    auto pRhsWrapper = (const StringTupleElementWrapper*)rhs.ppStrs;
    return ArrayTupleElementWrapper<StringTupleElementWrapper> { lhs.count, pLhsWrapper } == ArrayTupleElementWrapper<StringTupleElementWrapper> { rhs.count, pRhsWrapper };
}

bool operator!=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs)
{
    static_assert(sizeof(StringTupleElementWrapper) == sizeof(const char*));
    auto pLhsWrapper = (const StringTupleElementWrapper*)lhs.ppStrs;
    auto pRhsWrapper = (const StringTupleElementWrapper*)rhs.ppStrs;
    return ArrayTupleElementWrapper<StringTupleElementWrapper> { lhs.count, pLhsWrapper } < ArrayTupleElementWrapper<StringTupleElementWrapper> { rhs.count, pRhsWrapper };
}

bool operator>(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs)
{
    return rhs < lhs;
}

bool operator<=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs)
{
    return !(rhs < lhs);
}

bool operator>=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs)
{
    return !(lhs < rhs);
}

#if 0
// NOTE : Defined in gvk/structures/generated/pnext-tuple-element-wrapper.cpp
bool operator==(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
#endif

bool operator!=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)
{
    return !(lhs == rhs);
}

#if 0
// NOTE : Defined in gvk/structures/generated/pnext-tuple-element-wrapper.cpp
bool operator<(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
#endif

bool operator>(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)
{
    return rhs < lhs;
}

bool operator<=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)
{
    return !(rhs < lhs);
}

bool operator>=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)
{
    return !(lhs < rhs);
}

} // namespace detail
} // namespace gvk
