
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
#include "gvk-structures.hpp"
#include "gvk-binding-info/generated/binding-info.h"
#include "gvk-binding-info/generated/binding-info-enumerations-to-string.hpp"
#include "gvk-binding-info/generated/binding-info-structure-comparison-operators.hpp"
#include "gvk-binding-info/generated/binding-info-structure-create-copy.hpp"
#include "gvk-binding-info/generated/binding-info-structure-deserialization.hpp"
#include "gvk-binding-info/generated/binding-info-structure-destroy-copy.hpp"
#include "gvk-binding-info/generated/binding-info-structure-get-stype.hpp"
#include "gvk-binding-info/generated/binding-info-structure-serialization.hpp"
#include "gvk-binding-info/generated/binding-info-structure-to-string.hpp"

#include <memory>
#include <set>
#include <utility>

namespace gvk {
namespace detail {

template <template <typename, typename...> class ManagerType, typename InfoObjectType>
class InfoObjectRegistry final
{
public:
    InfoObjectRegistry() = default;

    InfoObjectRegistry(InfoObjectRegistry&& other) noexcept
    {
        *this = std::move(other);
    }

    InfoObjectRegistry& operator=(InfoObjectRegistry&& other) noexcept
    {
        if (this != &other) {
            mInfoObjects = std::exchange(other.mInfoObjects, { });
        }
        return *this;
    }

    ~InfoObjectRegistry()
    {
        reset();
    }

    void reset()
    {
        mInfoObjects.clear();
    }

    const InfoObjectType* register_info_object(const InfoObjectType& infoObject)
    {
        auto pInfoObject = get_registered_info_object(infoObject);
        if (!pInfoObject) {
            pInfoObject = &**mInfoObjects.insert(infoObject).first;
        }
        return pInfoObject;
    }

    const InfoObjectType* get_registered_info_object(const InfoObjectType& infoObject) const
    {
        auto itr = mInfoObjects.find(infoObject);
        return itr != mInfoObjects.end() ? &**itr : nullptr;
    }

private:
    std::set<ManagerType<InfoObjectType>> mInfoObjects;

    InfoObjectRegistry(const InfoObjectRegistry&) = delete;
    InfoObjectRegistry& operator=(InfoObjectRegistry&) = delete;
};

} // namespace detail
} // namespace gvk
