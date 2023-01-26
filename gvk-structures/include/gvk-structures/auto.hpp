
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

#include "gvk-defines.hpp"
#include "gvk-structures/comparison-operators.hpp"
#include "gvk-structures/copy.hpp"

#include <utility>

namespace gvk {

template <typename StructureType>
class Auto final
{
public:
    Auto() = default;

    inline Auto(const StructureType& other)
        : mStructure { detail::create_structure_copy(other, nullptr) }
    {
    }

    inline Auto(const Auto<StructureType>& other)
    {
        *this = other;
    }

    inline Auto(Auto<StructureType>&& other)
    {
        *this = std::move(other);
    }

    inline ~Auto()
    {
        reset();
    }

    inline Auto<StructureType>& operator=(const Auto<StructureType>& other)
    {
        if (this != &other) {
            reset();
            mStructure = detail::create_structure_copy(other.mStructure, nullptr);
        }
        return *this;
    }

    inline Auto<StructureType>& operator=(Auto<StructureType>&& other)
    {
        if (this != &other) {
            mStructure = other.mStructure;
            other.mStructure = { };
        }
        return *this;
    }

    inline operator const StructureType&() const
    {
        return mStructure;
    }

    inline const StructureType& operator*() const
    {
        return mStructure;
    }

    inline const StructureType* operator->() const
    {
        return &mStructure;
    }

    inline void reset()
    {
        detail::destroy_structure_copy(mStructure, nullptr);
        mStructure = { };
    }

private:
    StructureType mStructure { };
};

} // namespace gvk
