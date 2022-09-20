
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

#include "gvk/xml/enumeration.hpp"
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
