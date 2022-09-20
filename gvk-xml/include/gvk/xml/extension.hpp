
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
#include "gvk/xml/enumeration.hpp"

#include <map>
#include <set>
#include <string>

namespace gvk {
namespace xml {

class Extension final
    : public ApiElement
{
public:
    enum class Type
    {
        Instance,
        Device,
    };

    Extension() = default;
    Extension(const tinyxml2::XMLElement& xmlElement);

    std::string number;
    Type type { Type::Instance };
    std::string platform;
    std::string supported;
    std::string deprecatedBy;
    std::string obsoletedBy;
    std::string promotedTo;
    std::set<std::string> requirements;
    std::set<std::string> types;
    std::map<std::string, Enumeration> enumerations;
    std::set<std::string> commands;
};

} // namespace xml
} // namespace gvk
