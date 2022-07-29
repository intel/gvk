
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
#include "gvk/printer.hpp"

namespace gvk {

#ifdef VK_USE_PLATFORM_WIN32_KHR
template <> void print<LPCWSTR>(Printer& printer, const LPCWSTR& lpcwStr);
template <> void print<SECURITY_ATTRIBUTES>(Printer& printer, const SECURITY_ATTRIBUTES& securityAttributes);
#endif // VK_USE_PLATFORM_WIN32_KHR

} // namespace gvk
