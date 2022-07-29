
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

#include "gvk/xml/handle.hpp"
#include "tinyxml2-utilities.hpp"

#include <map>

namespace gvk {
namespace xml {

Handle::Handle(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_text(xmlElement.FirstChildElement("name"));
    if (!name.empty()) {
        isDispatchable = get_xml_text(xmlElement.FirstChildElement("type")) == "VK_DEFINE_HANDLE";
        vkObjectType = get_xml_attribute(xmlElement, "objtypeenum");
        for (const auto& parent : string::split(get_xml_attribute(xmlElement, "parent"), ",")) {
            parents.insert(parent);
        }
    } else {
        name = get_xml_attribute(xmlElement, "name");
        alias = get_xml_attribute(xmlElement, "alias");
    }
}

} // namespace xml
} // namespace gvk
