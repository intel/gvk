
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

#include "gvk-xml/format.hpp"
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
    auto pSpirvImageFormatXmlElement = xmlElement.FirstChildElement("spirvimageformat");
    if (pSpirvImageFormatXmlElement) {
        spirvImageFormat = get_xml_attribute(*pSpirvImageFormatXmlElement, "name");
    }
    process_xml_elements(xmlElement, "component", [&](const auto& componentXmlElement) { components.emplace_back(componentXmlElement); });
    process_xml_elements(xmlElement, "plane", [&](const auto& planeXmlElement) { planes.emplace_back(planeXmlElement); });
}

} // namespace xml
} // namespace gvk
