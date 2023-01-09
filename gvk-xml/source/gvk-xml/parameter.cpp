
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

#include "gvk-xml/parameter.hpp"
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
    selector = get_xml_attribute(xmlElement, "selector");
    limitType = get_xml_attribute(xmlElement, "limittype");
    for (const auto& value : string::split(get_xml_attribute(xmlElement, "values"), ",")) {
        values.push_back(value);
    }
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
            bitField = string::to_number<int>(string::remove(value, ":"));
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
