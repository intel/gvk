
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

#include "gvk-xml/command.hpp"
#include "gvk-xml/defines.hpp"
#include "gvk-xml/enumeration.hpp"
#include "gvk-xml/extension.hpp"
#include "gvk-xml/enumeration.hpp"
#include "gvk-xml/feature.hpp"
#include "gvk-xml/format.hpp"
#include "gvk-xml/handle.hpp"
#include "gvk-xml/platform.hpp"
#include "gvk-xml/structure.hpp"

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
    Manifest(const tinyxml2::XMLDocument& xmlDocument, const std::string& api = "vulkan");

    Enumeration constants;
    std::map<std::string, Platform> platforms;
    std::set<std::string> vendors;
    std::map<std::string, Handle> handles;
    std::map<std::string, Enumeration> enumerations;
    std::map<std::string, Structure> structures;
    std::map<std::string, Command> commands;
    std::map<std::string, Format> formats;
    std::map<std::string, Feature> features;
    std::map<std::string, Extension> extensions;
    std::map<std::string, std::string> vkObjectTypes;
    std::map<std::string, std::string> vkStructureTypes;
};

} // namespace xml
} // namespace gvk
