
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

#include "gvk/system/button-set.hpp"
#include "gvk/system/updateable.hpp"

#include <array>

namespace gvk {
namespace sys {

class Mouse final
{
public:
    enum class Button
    {
        // NOTE : This enum contains the symbolic constant names, hexadecimal values,
        //  and mouse or keyboard equivalent for the virtual key codes used by Windows.
        //  The codes are listed in numeric order.
        //  http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx

        Unknown      = 0,

        Left         = 0x01,
        Right        = 0x02,

        #if 0
        // NOTE : Defined in gvk/system/keyboard.hpp
        ControlBreak = 0x03,
        #endif

        Middle       = 0x04,
        X1           = 0x05,
        X2           = 0x06,

        Count,
        Any,
    };

    inline void update()
    {
        scroll.update();
        position.update();
        buttons.update();
    }

    class : public Updateable<std::array<float, 2>>
    {
    public:
        inline std::array<float, 2> delta() const
        {
            return { current[0] - previous[0], current[1] - previous[1] };
        }
    } scroll { }, position { };

    ButtonSet<(size_t)Button::Count> buttons { };
};

} // namespace sys
} // namespace gvk
