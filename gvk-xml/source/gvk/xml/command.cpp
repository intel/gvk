
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

#include "gvk/xml/command.hpp"
#include "tinyxml2-utilities.hpp"

namespace gvk {
namespace xml {

Command::Command(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    alias = get_xml_attribute(xmlElement, "alias");
    successCodes = string::split(get_xml_attribute(xmlElement, "successcodes"), ",");
    errorCodes = string::split(get_xml_attribute(xmlElement, "errorcodes"), ",");
    auto pProtoXml = xmlElement.FirstChildElement("proto");
    if (pProtoXml) {
        returnType = get_xml_text(pProtoXml->FirstChildElement("type"));
        if (name.empty()) {
            name = get_xml_text(pProtoXml->FirstChildElement("name"));
        }
    }
    process_xml_elements(xmlElement, "param", [&](const auto& paramXmlElement) { parameters.emplace_back(paramXmlElement); });
    if (string::starts_with(name, "vkCmd")) {
        type = Type::Cmd;
        target = "VkCommandBuffer";
    } else if (string::starts_with(name, "vkCreate") || string::starts_with(name, "vkAllocate")) {
        type = Type::Create;
        if (!parameters.empty()) {
            target = parameters.back().unqualifiedType;
        }
    } else if (string::starts_with(name, "vkDestroy") || string::starts_with(name, "vkFree")) {
        type = Type::Destroy;
        for (auto ritr = parameters.rbegin(); ritr != parameters.rend(); ++ritr) {
            if (ritr->unqualifiedType != "VkAllocationCallbacks") {
                target = ritr->unqualifiedType;
                break;
            }
        }
    }
}

} // namespace xml
} // namespace gvk
