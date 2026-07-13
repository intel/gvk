
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

#include "gvk-string/to-string.hpp"

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace gvk {

std::wstring to_wstring(const std::string& str)
{
    uint32_t wstrLength = 0;
    to_wstring((uint32_t)str.length(), str.c_str(), &wstrLength, nullptr);
    std::wstring wstr;
    wstr.resize(wstrLength);
    to_wstring((uint32_t)str.length(), str.c_str(), &wstrLength, wstr.data());
    return wstr;
}

void to_wstring(uint32_t strLength, const char* pStr, uint32_t* pWstrLength, wchar_t* pWstr)
{
#if defined(_WIN32) || defined(_WIN64)
    if (strLength && pStr && pWstrLength) {
        if (*pWstrLength && pWstr) {
            (void)MultiByteToWideChar(CP_UTF8, 0, pStr, strLength, pWstr, *pWstrLength);
        } else {
            *pWstrLength = MultiByteToWideChar(CP_UTF8, 0, pStr, (int)strLength, NULL, 0);
        }
    }
#else
    (void)strLength;
    (void)pStr;
    (void)pWstrLength;
    (void)pWstr;
    assert(false && "Not yet implemented");
#endif
}

} // namespace gvk
