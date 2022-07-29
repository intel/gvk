
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

#include "gvk/xml/format.hpp"
#include "tinyxml2-utilities.hpp"

#include <cassert>

namespace gvk {
namespace xml {

Plane::Plane(const tinyxml2::XMLElement& xmlElement)
{
    index = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "index"));
    widthDivisor = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "widthDivisor"));
    heightDivisor = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "heightDivisor"));
    compatible = get_xml_attribute(xmlElement, "compatible");
}

Component::Component(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    bits = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "bits"));
    numericFormat = get_xml_attribute(xmlElement, "numericFormat");
    planeIndex = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "planeIndex"));
}

Format::Format(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    classes = string::split(get_xml_attribute(xmlElement, "class"), " ");
    blockSize = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "blockSize"));
    texelsPerBlock = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "texelsPerBlock"));
    chroma = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "chroma"));
    packed = string::to_number<uint32_t>(get_xml_attribute(xmlElement, "packed"));
    uint32_t dimension_i = 0;
    for (const auto& dimension : string::split(get_xml_attribute(xmlElement, "blockExtent"), ",")) {
        assert(dimension_i < blockExtent.size());
        blockExtent[dimension_i++] = string::to_number<uint32_t>(dimension);
    }
    compressionType = get_xml_attribute(xmlElement, "compressed");
    spirvImageFormat = get_xml_attribute(xmlElement, "spirvImageFormat");
    process_xml_elements(xmlElement, "component", [&](const auto& componentXmlElement) { components.emplace_back(componentXmlElement); });
    process_xml_elements(xmlElement, "plane", [&](const auto& planeXmlElement) { planes.emplace_back(planeXmlElement); });
}

} // namespace xml
} // namespace gvk
