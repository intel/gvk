
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

#include "gvk/xml/structure.hpp"
#include "tinyxml2-utilities.hpp"

namespace gvk {
namespace xml {

Structure::Structure(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    alias = get_xml_attribute(xmlElement, "alias");
    isUnion = get_xml_attribute(xmlElement, "category") == "union";
    process_xml_elements(xmlElement, "member",
        [&](const auto& memberXmlElement)
        {
            members.emplace_back(memberXmlElement);
            if (members.back().name == "sType") {
                vkStructureType = get_xml_attribute(memberXmlElement, "values");
            }
        }
    );
}

} // namespace xml
} // namespace gvk
