
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

#include "gvk/xml/parameter.hpp"
#include "tinyxml2-utilities.hpp"

#include <cassert>

namespace gvk {
namespace xml {

Parameter::Parameter(const tinyxml2::XMLElement& xmlElement)
{
    type = get_xml_text(xmlElement.FirstChildElement("type"));
    unqualifiedType = string::remove(string::remove(string::remove(type, "const"),  "*"), " ");
    name = get_xml_text(xmlElement.FirstChildElement("name"));
    length = string::remove(get_xml_attribute(xmlElement, "len"), ",null-terminated");
    altLength = get_xml_attribute(xmlElement, "altlen");
    for (auto pNode = xmlElement.FirstChild(); pNode; pNode = pNode->NextSibling()) {
        auto value = string::trim_whitespace(pNode->Value() ? pNode->Value() : "");
        if (value == "const" || value == "const struct") {
            flags |= Const;
            type = string::contains(type, "const") ? type + " const" : "const " + type;
        } else if (value == "*") {
            flags |= Pointer;
            type += value;
            dimensionCount = 1;
        } else if (value == "**") {
            flags |= Pointer | Array;
            type += value;
            dimensionCount = 2;
        } else if (value == "* const*" || value == "* const *") {
            flags |= Dynamic | Const | Pointer | Array;
            type += value;
            dimensionCount = 2;
        } else if (value == "enum") {
            flags |= Static | Array;
            length = "[" + get_xml_text(xmlElement.FirstChildElement("enum")) + "]";
            dimensionCount = (int)std::count(length.begin(), length.end(), ']');
        } else if (string::starts_with(value, "[") && string::ends_with(value, "]")) {
            flags |= Static | Array;
            length = value;
            dimensionCount = (int)std::count(length.begin(), length.end(), ']');
        } else if (value == "[") {
        } else if (value == "]") {
        } else if (value == ":24" || value == ":8") {
        } else if (value == "struct") {
        } else if (value == "type") {
        } else if (value == "name") {
        } else if (value == "comment") {
        } else {
            assert(false);
        }
    }
    if (!length.empty()) {
        flags |= Array;
        if (!(flags & Static)) {
            flags |= Dynamic;
        }
        if (unqualifiedType == "char") {
            flags |= String;
        }
    }
    if (unqualifiedType == "void") {
        flags |= Void;
    }
    if (string::starts_with(unqualifiedType, "PFN_")) {
        flags |= Function | Pointer;
    }
    if (get_xml_attribute(xmlElement, "optional") == "true") {
        flags |= Optional;
    }
    type = string::replace(type, " *", "*");
}

} // namespace xml
} // namespace gvk
