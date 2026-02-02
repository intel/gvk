
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

#include "gvk-containers/streambuf.hpp"

#include <type_traits>

namespace gvk {

void VectorOstreambuf::clear()
{
    mData.clear();
}

const uint8_t* VectorOstreambuf::data() const
{
    return mData.data();
}

size_t VectorOstreambuf::size() const
{
    return mData.size();
}

std::streambuf::int_type VectorOstreambuf::overflow(std::streambuf::int_type ch)
{
    if (ch != traits_type::eof()) {
        xsputn((const char*)&ch, 1);
    }
    return ch;
}

std::streamsize VectorOstreambuf::xsputn(const char* pStr, std::streamsize n)
{
    mData.insert(mData.end(), pStr, pStr + n);
    return n;
}

VectorIstreambuf::VectorIstreambuf(std::vector<uint8_t>& data)
{
    setg((char*)data.data(), (char*)data.data(), (char*)data.data() + data.size());
}

} // namespace gvk
