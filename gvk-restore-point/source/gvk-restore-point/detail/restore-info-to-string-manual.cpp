
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

#include "gvk-defines.hpp"
#include "gvk-restore-point/generated/restore-info.h"
#include "gvk-restore-point/generated/restore-info-enumerations-to-string.hpp"
#include "gvk-restore-point/generated/restore-info-structure-to-string.hpp"
#include "gvk-structures.hpp"

namespace gvk {

template <>
void print<GvkRestorePointObject>(Printer& printer, const GvkRestorePointObject& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("objectType", obj.objectType);
            // NOTE : Casting handles to VkInstance so they print hex values
            printer.print_field("handle", (VkInstance)obj.handle);
            printer.print_field("dispatchableHandle", (VkInstance)obj.dispatchableHandle);
        }
    );
}

} // namespace gvk
