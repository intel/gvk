
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

class Structure final
    : public ApiElement
{
public:
    Structure() = default;
    Structure(const tinyxml2::XMLElement& xmlElement);

    bool isUnion { false };
    std::string vkStructureType;
    std::vector<Parameter> members;
};

} // namespace xml
} // namespace gvk
