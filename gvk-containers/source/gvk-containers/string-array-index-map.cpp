
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

#include "gvk-containers/string-array-index-map.hpp"

#include <cassert>
#include <cstring>

namespace gvk {

void StringArrayIndexMap::add(const char* pEntry, bool force)
{
    assert(pEntry);
    if (force) {
        erase(pEntry);
    }
    if (mEntryIndices.insert({pEntry, (uint32_t)mEntries.size()}).second) {
        mEntries.push_back(pEntry);
    }
}

void StringArrayIndexMap::add(uint32_t entryCount, const char* const* pEntries, bool force)
{
    if (pEntries) {
        for (uint32_t i = 0; i < entryCount; ++i) {
            add(pEntries[i], force);
        }
    }
}

void StringArrayIndexMap::erase(const char* pEntry)
{
    if (pEntry) {
        auto itr = mEntryIndices.find(pEntry);
        if (itr != mEntryIndices.end()) {
            auto index = itr->second;
            assert(index < mEntries.size());
            assert(mEntries[index]);
            assert(!strcmp(mEntries[index], pEntry));
            mEntryIndices.erase(itr);
            mEntries.erase(mEntries.begin() + index);
            for (; index < mEntries.size(); ++index) {
                itr = mEntryIndices.find(mEntries[index]);
                assert(itr != mEntryIndices.end());
                itr->second = index;
            }
        }
    }
}

bool StringArrayIndexMap::contains(const char* pEntry) const
{
    return pEntry ? mEntryIndices.count(pEntry) : 0;
}

void StringArrayIndexMap::clear()
{
    mEntryIndices.clear();
    mEntries.clear();
}

uint32_t StringArrayIndexMap::count() const
{
    return (uint32_t)mEntries.size();
}

const char* const* StringArrayIndexMap::data() const
{
    return !mEntries.empty() ? mEntries.data() : nullptr;
}

std::set<std::string> StringArrayIndexMap::validate(std::set<std::string> const& availableEntries)
{
    std::set<std::string> invalidEntries;
    for (uint32_t i = 0; i < mEntries.size();) {
        assert(mEntries[i]);
        if (!availableEntries.count(mEntries[i])) {
            invalidEntries.insert(mEntries[i]);
            erase(mEntries[i]);
        } else {
            ++i;
        }
    }
    return invalidEntries;
}

} // namespace gvk
