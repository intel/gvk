
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

#include "gvk-xml/api-element.hpp"
#include "gvk-xml/defines.hpp"
#include "gvk-xml/feature.hpp"

#include <map>
#include <set>
#include <string>

namespace gvk {
namespace xml {

class Extension final
    : public Feature
{
public:
    enum class Type
    {
        Instance,
        Device,
    };

    Extension() = default;
    Extension(const std::string& api, const tinyxml2::XMLElement& xmlElement);

    Type type { Type::Instance };
    std::string platform;
    std::string deprecatedBy;
    std::string obsoletedBy;
    std::string promotedTo;
};

} // namespace xml
} // namespace gvk
