
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

#include "gvk-xml/command.hpp"
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

Parameter Command::get_target_parameter() const
{
    for (const auto& parameter : parameters) {
        if (parameter.unqualifiedType == target) {
            return parameter;
        }
    }
    return { };
}

} // namespace xml
} // namespace gvk
