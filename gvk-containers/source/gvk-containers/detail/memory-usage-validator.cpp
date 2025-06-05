
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

#include "gvk-containers/detail/memory-usage-validator.hpp"

#ifdef __linux__
#include <malloc.h>
#endif

#ifdef WIN32
#ifndef _CRTDBGMAP_ALLOC
#define _CRTDBGMAP_ALLOC
#endif
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <cassert>

/**
REFERENCE :
    https://stackoverflow.com/questions/2980917/c-is-it-possible-to-implement-memory-leak-testing-in-a-unit-test
*/

namespace gvk {
namespace detail {

struct MemoryUsageTracker::Snapshot
{
#ifdef __linux__
#if 0
    // TODO : Need to checks to fallback to mallinfo if mallinfo2 isn't available
    mallinfo2 mallInfo{ };
#endif
#endif
#ifdef WIN32
    _CrtMemState crtMemState{ };
#endif
};

MemoryUsageTracker::~MemoryUsageTracker()
{
    for (auto pSnapshot : mSnapshots) {
        delete pSnapshot;
    }
}

void MemoryUsageTracker::create_snapshot(Snapshot** ppSnapshot)
{
    assert(ppSnapshot);
    auto pSnapshot = new Snapshot;
    *ppSnapshot = pSnapshot;
    mSnapshots.insert(pSnapshot);
#ifdef __linux__
#if 0
    pSnapshot->mallInfo = mallinfo2();
#endif
#endif
#ifdef WIN32
    _CrtMemCheckpoint(&pSnapshot->crtMemState);
#endif
}

void MemoryUsageTracker::destroy_snapshot(const Snapshot* pSnapshot)
{
    assert(pSnapshot);
    auto itr = mSnapshots.find(pSnapshot);
    if (itr != mSnapshots.end()) {
        mSnapshots.erase(itr);
        delete *itr;
    }
}

bool MemoryUsageTracker::equal(const MemoryUsageTracker::Snapshot* pLhs, const MemoryUsageTracker::Snapshot* pRhs)
{
    assert(pLhs);
    assert(pRhs);
    return *pLhs == *pRhs;
}

bool operator==(const MemoryUsageTracker::Snapshot& lhs, const MemoryUsageTracker::Snapshot& rhs)
{
    (void)lhs;
    (void)rhs;
#ifdef __linux__
#if 0
    return lhs.mallInfo.uordblks == rhs.mallInfo.uordblks;
#else
    return true;
#endif
#endif
#ifdef WIN32
    _CrtMemState crtMemStateDiff{ };
    return (bool)_CrtMemDifference(&crtMemStateDiff, &lhs.crtMemState, &rhs.crtMemState);
#endif
}

bool operator!=(const MemoryUsageTracker::Snapshot& lhs, const MemoryUsageTracker::Snapshot& rhs)
{
    return !(lhs == rhs);
}

} // namespace detail
} // namespace gvk
