
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

#include "gvk/detail/comparison-operator-utilities.hpp"
#include "gvk/generated/comparison-operators.hpp"

namespace gvk {

#define GVK_STUB_COMPARISON_OPERATOR_DEFINITIONS(VK_STRUCTURE_TYPE) \
bool operator==(const VK_STRUCTURE_TYPE&,     const VK_STRUCTURE_TYPE&    ) { return false; }; \
bool operator!=(const VK_STRUCTURE_TYPE& lhs, const VK_STRUCTURE_TYPE& rhs) { return !(lhs == rhs); }; \
bool operator< (const VK_STRUCTURE_TYPE&,     const VK_STRUCTURE_TYPE&    ) { return false; }; \
bool operator> (const VK_STRUCTURE_TYPE& lhs, const VK_STRUCTURE_TYPE& rhs) { return rhs < lhs; }; \
bool operator<=(const VK_STRUCTURE_TYPE& lhs, const VK_STRUCTURE_TYPE& rhs) { return !(rhs < lhs); }; \
bool operator>=(const VK_STRUCTURE_TYPE& lhs, const VK_STRUCTURE_TYPE& rhs) { return !(rhs < lhs); };

GVK_STUB_COMPARISON_OPERATOR_DEFINITIONS(SECURITY_ATTRIBUTES)

} // namespace gvk
