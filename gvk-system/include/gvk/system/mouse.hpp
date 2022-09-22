
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
