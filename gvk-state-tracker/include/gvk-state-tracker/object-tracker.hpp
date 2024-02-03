
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

#include "gvk-state-tracker/thread-safe-unordered-map.hpp"
#include "VK_LAYER_INTEL_gvk_state_tracker.h"

#include <cassert>
#include <tuple>
#include <utility>

namespace gvk {
namespace state_tracker {

template <typename GvkHandleType>
class ObjectTracker final
{
public:
    template <typename ProcessHandleFunctionType>
    inline bool enumerate(ProcessHandleFunctionType processHandle) const
    {
        return mHandles.enumerate(
            [processHandle](const auto& handleItr)
            {
                return processHandle(handleItr.second);
            }
        );
    }

    inline bool enumerate(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
    {
        return mHandles.enumerate(
            [pfnCallback, pUserData](const auto& handleItr)
            {
                handleItr.second.enumerate(pfnCallback, pUserData);
                return true;
            }
        );
    }

    inline void insert(const GvkHandleType& handle)
    {
        auto inserted = mHandles.insert({ (typename GvkHandleType::VkHandleType)handle, handle }).second;
        (void)inserted;
        assert(inserted && "Attempting to insert duplicate handle; is an unhooked/unserviced entrypoint/extension in use?");
    }

    inline GvkHandleType get(typename GvkHandleType::VkHandleType vkHandle) const
    {
        auto handle = mHandles.get(vkHandle);
        assert(handle && "Attempting to get non existant handle; is an unhooked/unserviced entrypoint/extension in use?");
        return handle;
    }

    inline void erase(typename GvkHandleType::VkHandleType vkHandle)
    {
        auto erased = mHandles.erase(vkHandle);
        (void)erased;
        assert(erased && "Attempting to erase non existant handle; is an unhooked/unserviced entrypoint/extension in use?");
    }

    inline void clear()
    {
        mHandles.clear();
    }

private:
    ThreadSafeUnorderedMap<typename GvkHandleType::VkHandleType, GvkHandleType> mHandles;
};

template <typename GvkHandleType>
using BindingTracker = ObjectTracker<GvkHandleType>;

template <typename GvkHandleType>
using DependencyTracker = ObjectTracker<GvkHandleType>;

} // namespace state_tracker
} // namespace gvk
