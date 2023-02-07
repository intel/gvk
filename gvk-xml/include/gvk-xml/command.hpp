
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
#include "gvk-xml/parameter.hpp"

#include <string>
#include <vector>

namespace gvk {
namespace xml {

class Command final
    : public ApiElement
{
public:
    enum class Type
    {
        Common,
        Cmd,
        Create,
        Destroy,
    };

    Command() = default;
    Command(const tinyxml2::XMLElement& xmlElement);

    Parameter get_target_parameter() const;

    Type type { Type::Common };
    std::string target;
    std::string returnType{ "void" };
    std::vector<std::string> successCodes;
    std::vector<std::string> errorCodes;
    std::vector<Parameter> parameters;
};

} // namespace xml
} // namespace gvk
