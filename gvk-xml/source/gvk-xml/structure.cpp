
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

#include "gvk-xml/structure.hpp"
#include "tinyxml2-utilities.hpp"

namespace gvk {
namespace xml {

Structure::Structure(const tinyxml2::XMLElement& xmlElement)
{
    apis = get_apis(get_xml_attribute(xmlElement, "api"));
    name = get_xml_attribute(xmlElement, "name");
    alias = get_xml_attribute(xmlElement, "alias");
    isUnion = get_xml_attribute(xmlElement, "category") == "union";
    process_xml_elements(xmlElement, "member",
        [&](const auto& memberXmlElement)
        {
            Parameter member(memberXmlElement);
            if (apis_compatible(apis, member.apis)) {
                members.push_back(member);
                if (members.back().name == "sType") {
                    vkStructureType = get_xml_attribute(memberXmlElement, "values");
                }
            }
        }
    );
}

} // namespace xml
} // namespace gvk
