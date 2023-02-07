
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

#include "gvk-xml/enumeration.hpp"
#include "tinyxml2-utilities.hpp"

#include <cassert>

namespace gvk {
namespace xml {

Enumerator::Enumerator(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    alias = get_xml_attribute(xmlElement, "alias");
    value = get_xml_attribute(xmlElement, "value");
    bitPos = get_xml_attribute(xmlElement, "bitpos");
    extensionNumber = get_xml_attribute(xmlElement, "extnumber");
    offset = get_xml_attribute(xmlElement, "offset");
    direction = get_xml_attribute(xmlElement, "dir");
    extends = get_xml_attribute(xmlElement, "extends");
    if (!bitPos.empty()) {
        assert(value.empty());
        value = std::to_string(1 << string::to_number<uint32_t>(bitPos));
    }
    if (value.empty()) {
        value = get_offset_value(extensionNumber, offset, direction);
    }
}

std::string Enumerator::get_offset_value(
    const std::string& extensionNumber,
    const std::string& offset,
    const std::string& direction
)
{
    if (!extensionNumber.empty() && !offset.empty()) {
        static const int64_t BaseValue = 1000000000;
        static const int64_t RangeSize = 1000;
        auto extensionNumberValue = string::to_number<int64_t>(extensionNumber);
        auto offsetValue = string::to_number<int64_t>(offset);
        auto value = BaseValue + (extensionNumberValue - 1) * RangeSize + offsetValue;
        return std::to_string(value * (direction == "-" ? -1 : 1));
    }
    return { };
}

inline auto make_tuple(const Enumerator& enumerator)
{
    return std::tie(
        enumerator.value,
        enumerator.name,
        enumerator.alias
    );
}

bool operator==(const Enumerator& lhs, const Enumerator& rhs)
{
    return make_tuple(lhs) == make_tuple(rhs);
}

bool operator!=(const Enumerator& lhs, const Enumerator& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const Enumerator& lhs, const Enumerator& rhs)
{
    return make_tuple(lhs) < make_tuple(rhs);
}

bool operator>(const Enumerator& lhs, const Enumerator& rhs)
{
    return rhs < lhs;
}

bool operator<=(const Enumerator& lhs, const Enumerator& rhs)
{
    return !(rhs < lhs);
}

bool operator>=(const Enumerator& lhs, const Enumerator& rhs)
{
    return !(lhs < rhs);
}

Enumeration::Enumeration(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    alias = get_xml_attribute(xmlElement, "alias");
    isBitmask = get_xml_attribute(xmlElement, "type") == "bitmask";
    process_xml_elements(xmlElement, "enum", [&](const auto& enumXmlElement) { enumerators.insert(enumXmlElement); });
}

} // namespace xml
} // namespace gvk
