
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

#include "gvk-state-tracker/dependency-enumerator.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

DependencyEnumerator::DependencyEnumerator(const GvkStateTrackedObjectEnumerateInfo& enumerateInfo)
    : mEnumerateInfo { enumerateInfo }
{
}

void DependencyEnumerator::enumerate(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData)
{
    assert(pStateTrackedObject);
    assert(pUserData);
    auto& dependencyEnuemrator = *(DependencyEnumerator*)pUserData;
    if (dependencyEnuemrator.mProcessedHandles.insert(HandleId<uint64_t, uint64_t>(pStateTrackedObject->dispatchableHandle, pStateTrackedObject->handle)).second) {
        assert(dependencyEnuemrator.mEnumerateInfo.pfnCallback);
        dependencyEnuemrator.mEnumerateInfo.pfnCallback(pStateTrackedObject, pInfo, dependencyEnuemrator.mEnumerateInfo.pUserData);
    }
}

} // namespace state_tracker
} // namespace gvk
