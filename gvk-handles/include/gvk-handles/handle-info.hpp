
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

#include "gvk-reference.hpp"

namespace gvk {

#if 0
template<typename HandleIdType>
class HandleInfo final
{
public:
    class ControlBlock;
    gvk::Reference<ControlBlock, HandleIdType> reference;
    using HandleId = HandleIdType;
    HandleInfo() = default;
    inline HandleInfo(std::nullptr_t) { };
    inline HandleInfo(gvk::nullref_t) { };
    inline HandleInfo(gvk::newref_t, const HandleIdType& handleId) { reference.reset(gvk::newref, handleId); }
    inline HandleInfo(const HandleIdType& handleId) { reference = gvk::Reference<ControlBlock, HandleIdType>::get(handleId); }
    HandleInfo(const HandleInfo&) = default;
    HandleInfo& operator=(const HandleInfo&) = default;
    HandleInfo(HandleInfo&&) = default;
    HandleInfo& operator=(HandleInfo&&) = default;
    inline operator bool() const { return reference; }
    inline ControlBlock& operator*() { return reference.get_obj(); }
    inline ControlBlock* operator->() { return &reference.get_obj(); }
    inline const ControlBlock& operator*() const { return reference.get_obj(); }
    inline const ControlBlock* operator->() const { return &reference.get_obj(); }
    inline friend bool operator==(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference == rhs.reference; }
    inline friend bool operator!=(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference != rhs.reference; }
    inline friend bool operator<(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference < rhs.reference; }
    inline friend bool operator>(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference > rhs.reference; }
    inline friend bool operator<=(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference <= rhs.reference; }
    inline friend bool operator>=(const HandleInfo& lhs, const HandleInfo& rhs) { return lhs.reference >= rhs.reference; }
};
#endif

} // namespace gvk
