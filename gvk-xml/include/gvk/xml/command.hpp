
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

#pragma once

#include "gvk/xml/api-element.hpp"
#include "gvk/xml/defines.hpp"
#include "gvk/xml/parameter.hpp"

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

    Type type { Type::Common };
    std::string target;
    std::string returnType{ "void" };
    std::vector<std::string> successCodes;
    std::vector<std::string> errorCodes;
    std::vector<Parameter> parameters;
};

} // namespace xml
} // namespace gvk
