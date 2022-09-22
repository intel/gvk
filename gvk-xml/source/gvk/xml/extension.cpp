
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

#include "gvk/xml/extension.hpp"
#include "tinyxml2-utilities.hpp"

namespace gvk {
namespace xml {

Extension::Extension(const tinyxml2::XMLElement& xmlElement)
{
    name = get_xml_attribute(xmlElement, "name");
    extension = name;
    vendor = get_xml_attribute(xmlElement, "author");
    number = get_xml_attribute(xmlElement, "number");
    type = get_xml_attribute(xmlElement, "type") == "instance" ? Type::Instance : Type::Device;
    platform = get_xml_attribute(xmlElement, "platform");
    supported = get_xml_attribute(xmlElement, "supported");
    deprecatedBy = get_xml_attribute(xmlElement, "deprecatedby");
    obsoletedBy = get_xml_attribute(xmlElement, "obsoletedby");
    promotedTo = get_xml_attribute(xmlElement, "promotedto");
    process_requirements(xmlElement, *this);

    for (auto& enumerationItr : enumerations) {
        std::set<Enumerator> enumerators;
        for (auto enumerator : enumerationItr.second.enumerators) {
            if (enumerator.value.empty()) {
                auto extensionNumber = !enumerator.extensionNumber.empty() ? enumerator.extensionNumber : number;
                enumerator.value = Enumerator::get_offset_value(extensionNumber, enumerator.offset, enumerator.direction);
            }
            enumerators.insert(enumerator);
        }
        enumerationItr.second.enumerators = enumerators;
    }
}

} // namespace xml
} // namespace gvk
