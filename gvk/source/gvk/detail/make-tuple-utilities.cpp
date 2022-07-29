
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#include "gvk/detail/make-tuple-utilities.hpp"
#include "gvk/generated/make-tuple.hpp"

#include <string_view>

namespace gvk {
namespace detail {

// NOTE : This function is implemented in gvk/generated/make-tuple.cpp
#if 0
bool operator==(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)
{
    return false;
}
#endif

bool operator!=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)
{
    return !(lhs == rhs);
}

// NOTE : This function is implemented in gvk/generated/make-tuple.cpp
#if 0
bool operator<(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)
{
    return false;
}
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

bool operator==(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs)
{
    static_assert(sizeof(StringTupleElementWrapper) == sizeof(const char*));
    auto pLhsWrapper = (const StringTupleElementWrapper*)lhs.ppStrs;
    auto pRhsWrapper = (const StringTupleElementWrapper*)rhs.ppStrs;
    return ArrayTupleElementWrapper { lhs.count, pLhsWrapper } == ArrayTupleElementWrapper { rhs.count, pRhsWrapper };
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
    return ArrayTupleElementWrapper { lhs.count, pLhsWrapper } < ArrayTupleElementWrapper { rhs.count, pRhsWrapper };
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

} // namespace detail
} // namespace gvk
