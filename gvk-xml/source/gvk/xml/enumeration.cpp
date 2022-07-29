
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

namespace gvk {
namespace xml {

Enumerator::Enumerator(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    value = get_xml_attribute(xmlElement, "value");
    if (value.empty()) {
        value = get_xml_attribute(xmlElement, "bitpos");
    }
    alias = get_xml_attribute(xmlElement, "alias");
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
    isBitmask = get_xml_attribute(xmlElement, "type") == "bitmask";
    process_xml_elements(xmlElement, "enum", [&](const auto& enumXmlElement) { enumerators.insert(enumXmlElement); });
}

} // namespace xml
} // namespace gvk
