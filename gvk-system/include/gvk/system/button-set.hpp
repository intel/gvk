
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

#include "gvk/system/updateable.hpp"

#include <bitset>

namespace gvk {
namespace sys {

template <size_t Count>
class ButtonSet
    : public Updateable<std::bitset<Count>>
{
public:
    template <typename ButtonIdType>
    inline bool up(ButtonIdType buttonId) const
    {
        return !this->current[(size_t)buttonId];
    }

    template <typename ButtonIdType>
    inline bool down(ButtonIdType buttonId) const
    {
        return this->current[(size_t)buttonId];
    }

    template <typename ButtonIdType>
    inline bool held(ButtonIdType buttonId) const
    {
        return this->previous[(size_t)buttonId] && this->current[(size_t)buttonId];
    }

    template <typename ButtonIdType>
    inline bool pressed(ButtonIdType buttonId) const
    {
        return !this->previous[(size_t)buttonId] && this->current[(size_t)buttonId];
    }

    template <typename ButtonIdType>
    inline bool released(ButtonIdType buttonId) const
    {
        return this->previous[(size_t)buttonId] && !this->current[(size_t)buttonId];
    }
};

} // namespace sys
} // namespace gvk
