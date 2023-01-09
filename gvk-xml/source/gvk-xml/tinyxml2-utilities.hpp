
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

#pragma once

#include "gvk-string/utilities.hpp"
#include "gvk-xml/defines.hpp"

#include <string>

namespace gvk {
namespace xml {

inline std::string get_xml_text(const tinyxml2::XMLElement& xmlElement)
{
    auto pText = xmlElement.GetText();
    return pText ? pText : std::string();
}

inline std::string get_xml_text(const tinyxml2::XMLElement* pXmlElement)
{
    return pXmlElement ? get_xml_text(*pXmlElement) : std::string();
}

inline std::string get_xml_attribute(const tinyxml2::XMLElement& xmlElement, const std::string& attribute)
{
    auto pAttribute = xmlElement.Attribute(attribute.c_str());
    return pAttribute ? pAttribute : std::string();
}

template <typename ProcessXmlElementFunctionType>
inline void process_xml_elements(
    const tinyxml2::XMLElement& xmlCollection,
    const std::string& xmlElementName,
    ProcessXmlElementFunctionType processXmlElement
)
{
    for (auto pXmlElement = xmlCollection.FirstChildElement(xmlElementName.c_str()); pXmlElement; pXmlElement = pXmlElement->NextSiblingElement(xmlElementName.c_str())) {
        processXmlElement(*pXmlElement);
    }
}

template <typename ProcessXmlElementFunctionType>
inline void process_xml_elements(
    const tinyxml2::XMLElement& registryXml,
    const std::string& xmlCollectionName,
    const std::string& xmlElementName,
    ProcessXmlElementFunctionType processXmlElement
)
{
    auto pXmlCollection = registryXml.FirstChildElement(xmlCollectionName.c_str());
    if (pXmlCollection) {
        process_xml_elements(*pXmlCollection, xmlElementName, processXmlElement);
    }
}

template <typename ExtensionType>
inline void process_requirements(const tinyxml2::XMLElement& xmlElement, ExtensionType& extension)
{
    for (const auto& requirement : string::split(get_xml_attribute(xmlElement, "requires"), ",")) {
        extension.requirements.insert(requirement);
    }
    process_xml_elements(xmlElement, "require",
        [&](const auto& requireXmlElement)
        {
            process_xml_elements(requireXmlElement, "type",
                [&](const auto& typeXmlElement)
                {
                    auto name = get_xml_attribute(typeXmlElement, "name");
                    if (!name.empty()) {
                        extension.types.insert(name);
                    }
                }
            );
            process_xml_elements(requireXmlElement, "enum",
                [&](const auto& enumXmlElement)
                {
                    auto extends = get_xml_attribute(enumXmlElement, "extends");
                    if (!extends.empty()) {
                        auto enumerationItr = extension.enumerations.find(extends);
                        if (enumerationItr == extension.enumerations.end()) {
                            enumerationItr = extension.enumerations.insert({ extends, { } }).first;
                            enumerationItr->second.name = extends;
                        }
                        enumerationItr->second.enumerators.insert(enumXmlElement);
                    }
                }
            );
            process_xml_elements(requireXmlElement, "command",
                [&](const auto& commandXmlElement)
                {
                    auto name = get_xml_attribute(commandXmlElement, "name");
                    if (!name.empty()) {
                        extension.commands.insert(name);
                    }
                }
            );
        }
    );
}

} // namespace xml
} // namespace gvk
