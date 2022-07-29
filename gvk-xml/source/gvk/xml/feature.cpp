
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

#include "gvk/xml/feature.hpp"
#include "tinyxml2-utilities.hpp"

namespace gvk {
namespace xml {

Feature::Feature(const tinyxml2::XMLElement& xmlElement)
{
    api = get_xml_attribute(xmlElement, "api");
    name = get_xml_attribute(xmlElement, "name");
    number = get_xml_attribute(xmlElement, "number");
    process_requirements(xmlElement, *this);
}

} // namespace xml
} // namespace gvk
