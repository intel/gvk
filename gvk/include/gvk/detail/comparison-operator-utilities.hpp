
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

#pragma once

#include "gvk/defines.hpp"

namespace gvk {

#define GVK_STUB_COMPARISON_OPERATOR_DECLARATIONS(VK_STRUCTURE_TYPE) \
bool operator==(const VK_STRUCTURE_TYPE&, const VK_STRUCTURE_TYPE&); \
bool operator!=(const VK_STRUCTURE_TYPE&, const VK_STRUCTURE_TYPE&); \
bool operator< (const VK_STRUCTURE_TYPE&, const VK_STRUCTURE_TYPE&); \
bool operator> (const VK_STRUCTURE_TYPE&, const VK_STRUCTURE_TYPE&); \
bool operator<=(const VK_STRUCTURE_TYPE&, const VK_STRUCTURE_TYPE&); \
bool operator>=(const VK_STRUCTURE_TYPE&, const VK_STRUCTURE_TYPE&);

#ifdef VK_USE_PLATFORM_WIN32_KHR
GVK_STUB_COMPARISON_OPERATOR_DECLARATIONS(SECURITY_ATTRIBUTES)
#endif // VK_USE_PLATFORM_WIN32_KHR

} // namespace gvk
