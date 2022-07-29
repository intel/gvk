
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

#include "gvk/xml/command.hpp"
#include "gvk/xml/defines.hpp"
#include "gvk/xml/enumeration.hpp"
#include "gvk/xml/extension.hpp"
#include "gvk/xml/enumeration.hpp"
#include "gvk/xml/feature.hpp"
#include "gvk/xml/format.hpp"
#include "gvk/xml/handle.hpp"
#include "gvk/xml/platform.hpp"
#include "gvk/xml/structure.hpp"

#include <array>
#include <map>
#include <set>
#include <string>

namespace gvk {
namespace xml {

class Manifest final
{
public:
    Manifest() = default;
    Manifest(const tinyxml2::XMLDocument& xmlDocument);

    Enumeration apiConstants;
    std::map<std::string, Platform> platforms;
    std::set<std::string> vendors;
    std::map<std::string, Handle> handles;
    std::map<std::string, Enumeration> enumerations;
    std::map<std::string, Structure> structures;
    std::map<std::string, Command> commands;
    std::map<std::string, Extension> extensions;
    std::map<std::string, Feature> features;
    std::map<std::string, Format> formats;
    std::map<std::string, std::string> vkObjectTypes;
    std::map<std::string, std::string> vkStructureTypes;
};

} // namespace xml
} // namespace gvk
