
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

#include "tinyxml2.h"

#include <cstdint>

namespace gvk {
namespace xml {

enum FlagBits
{
    Optional = 1,
    Dynamic  = 1 << 1,
    Static   = 1 << 2,
    Const    = 1 << 3,
    Pointer  = 1 << 4,
    Array    = 1 << 5,
    String   = 1 << 6,
    Void     = 1 << 7,
    Function = 1 << 8,
};

using Flags = uint32_t;

} // namespace xml
} // namespace gvk
