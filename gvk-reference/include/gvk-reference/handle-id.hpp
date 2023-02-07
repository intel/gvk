
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

#include <bitset>
#include <cstring>
#include <utility>

namespace gvk {

template <typename VkDispatchableHandleType, typename VkHandleType>
class HandleId final
{
public:
    HandleId() = default;

    inline HandleId(VkDispatchableHandleType dispatchableHandle, VkHandleType handle)
        : mId { dispatchableHandle, handle }
    {
    }

    inline const VkDispatchableHandleType& get_dispatchable_handle() const
    {
        return mId.first;
    }

    inline const VkHandleType& get_handle() const
    {
        return mId.second;
    }

    inline friend bool operator==(const HandleId<VkDispatchableHandleType, VkHandleType>& lhs, const HandleId<VkDispatchableHandleType, VkHandleType>& rhs)
    {
        return lhs.mId == rhs.mId;
    }

    inline friend bool operator!=(const HandleId<VkDispatchableHandleType, VkHandleType>& lhs, const HandleId<VkDispatchableHandleType, VkHandleType>& rhs)
    {
        return lhs.mId != rhs.mId;
    }

    inline friend bool operator<(const HandleId<VkDispatchableHandleType, VkHandleType>& lhs, const HandleId<VkDispatchableHandleType, VkHandleType>& rhs)
    {
        return lhs.mId < rhs.mId;
    }

    inline friend bool operator>(const HandleId<VkDispatchableHandleType, VkHandleType>& lhs, const HandleId<VkDispatchableHandleType, VkHandleType>& rhs)
    {
        return lhs.mId > rhs.mId;
    }

    inline friend bool operator<=(const HandleId<VkDispatchableHandleType, VkHandleType>& lhs, const HandleId<VkDispatchableHandleType, VkHandleType>& rhs)
    {
        return lhs.mId <= rhs.mId;
    }

    inline friend bool operator>=(const HandleId<VkDispatchableHandleType, VkHandleType>& lhs, const HandleId<VkDispatchableHandleType, VkHandleType>& rhs)
    {
        return lhs.mId >= rhs.mId;
    }

private:
    std::pair<VkDispatchableHandleType, VkHandleType> mId;
};

} // namespace gvk

namespace std {

template <typename VkDispatchableHandleType, typename VkHandleType>
struct hash<gvk::HandleId<VkDispatchableHandleType, VkHandleType>>
{
    inline size_t operator()(const gvk::HandleId<VkDispatchableHandleType, VkHandleType>& handleId) const
    {
        return std::hash<uint64_t> { }((uint64_t)handleId.get_dispatchable_handle()) ^ (std::hash<uint64_t> { }((uint64_t)handleId.get_handle()) << 1);
    }
};

} // namespace std
