
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

#include "gvk-xml/api-element.hpp"
#include "gvk-xml/defines.hpp"

#include <set>
#include <string>

namespace gvk {
namespace xml {

class Enumerator final
    : public ApiElement
{
public:
    static constexpr int64_t ExtensionBaseValue { 1000000000 };
    static constexpr int64_t ExtensionRangeSize { 1000 };

    Enumerator() = default;
    Enumerator(const tinyxml2::XMLElement& xmlElement);

    static std::string get_offset_value(
        const std::string& extensionNumber,
        const std::string& offset,
        const std::string& direction
    );

    std::string value;
    std::string bitPos;
    std::string offset;
    std::string direction;
    std::string extends;
    std::string extensionNumber;
};

bool operator==(const Enumerator& lhs, const Enumerator& rhs);
bool operator!=(const Enumerator& lhs, const Enumerator& rhs);
bool operator<(const Enumerator& lhs, const Enumerator& rhs);
bool operator>(const Enumerator& lhs, const Enumerator& rhs);
bool operator<=(const Enumerator& lhs, const Enumerator& rhs);
bool operator>=(const Enumerator& lhs, const Enumerator& rhs);

class Enumeration final
    : public ApiElement
{
public:
    Enumeration() = default;
    Enumeration(const tinyxml2::XMLElement& xmlElement);

    bool isBitmask { false };
    std::set<Enumerator> enumerators;
};

} // namespace xml
} // namespace gvk
