
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
