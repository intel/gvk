
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

#include <unordered_set>

namespace gvk {
namespace detail {

/**
Utility object for tracking allocations in the current process
    @note The information provided by this object is platform dependent, configuration dependent, very coarse, and mostly intended for debugging
*/
class MemoryUsageTracker final
{
public:
    struct Snapshot;
    MemoryUsageTracker() = default;
    ~MemoryUsageTracker();
    void create_snapshot(Snapshot** ppSnapshot);
    void destroy_snapshot(const Snapshot* pSnapshot);
    bool equal(const MemoryUsageTracker::Snapshot* pLhs, const MemoryUsageTracker::Snapshot* pRhs);
private:
    std::unordered_set<const Snapshot*> mSnapshots;
    MemoryUsageTracker(const MemoryUsageTracker&) = delete;
    MemoryUsageTracker& operator=(const MemoryUsageTracker&) = delete;
};

bool operator==(const MemoryUsageTracker::Snapshot& lhs, const MemoryUsageTracker::Snapshot& rhs);
bool operator!=(const MemoryUsageTracker::Snapshot& lhs, const MemoryUsageTracker::Snapshot& rhs);

} // namespace detail
} // namespace gvk
