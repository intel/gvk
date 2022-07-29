
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

#include <array>
#include <string>
#include <vector>

namespace gvk {
namespace xml {

class Plane final
{
public:
    Plane() = default;
    Plane(const tinyxml2::XMLElement& xmlElement);

    uint32_t index{ };
    uint32_t widthDivisor{ };
    uint32_t heightDivisor{ };
    std::string compatible;
};

class Component final
{
public:
    Component() = default;
    Component(const tinyxml2::XMLElement& xmlElement);

    std::string name;
    uint32_t bits{ };
    std::string numericFormat;
    uint32_t planeIndex{ };
};

class Format final
    : public ApiElement
{
public:
    Format() = default;
    Format(const tinyxml2::XMLElement& xmlElement);

    std::vector<std::string> classes;
    uint32_t blockSize{ };
    uint32_t texelsPerBlock{ };
    uint32_t chroma{ };
    uint32_t packed{ };
    std::array<uint32_t, 3> blockExtent{ };
    std::string compressionType;
    std::string spirvImageFormat;
    std::vector<Component> components;
    std::vector<Plane> planes;
};

} // namespace xml
} // namespace gvk
