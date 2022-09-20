
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

#include "gvk/xml/extension.hpp"
#include "tinyxml2-utilities.hpp"

namespace gvk {
namespace xml {

Extension::Extension(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    extension = name;
    vendor = get_xml_attribute(xmlElement, "author");
    number = get_xml_attribute(xmlElement, "number");
    type = get_xml_attribute(xmlElement, "type") == "instance" ? Type::Instance : Type::Device;
    platform = get_xml_attribute(xmlElement, "platform");
    supported = get_xml_attribute(xmlElement, "supported");
    deprecatedBy = get_xml_attribute(xmlElement, "deprecatedby");
    obsoletedBy = get_xml_attribute(xmlElement, "obsoletedby");
    promotedTo = get_xml_attribute(xmlElement, "promotedto");
    process_requirements(xmlElement, *this);

    for (auto& enumerationItr : enumerations) {
        std::set<Enumerator> enumerators;
        for (auto enumerator : enumerationItr.second.enumerators) {
            if (enumerator.value.empty()) {
                auto extensionNumber = !enumerator.extensionNumber.empty() ? enumerator.extensionNumber : number;
                enumerator.value = Enumerator::get_offset_value(extensionNumber, enumerator.offset, enumerator.direction);
            }
            enumerators.insert(enumerator);
        }
        enumerationItr.second.enumerators = enumerators;
    }
}

} // namespace xml
} // namespace gvk
