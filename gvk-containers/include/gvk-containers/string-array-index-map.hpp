
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

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace gvk {

class StringArrayIndexMap final
{
public:
    template <typename T, typename GetStringFunctionType>
    inline std::set<std::string> validate(uint32_t objectCount, const T* pObjs, GetStringFunctionType getString)
    {
        std::set<std::string> availableEntries;
        for (uint32_t i = 0; i < objectCount; ++i) {
            availableEntries.insert(getString(pObjs[i]));
        }
        return validate(availableEntries);
    }

    void add(const char* pEntry, bool force = false);
    void add(uint32_t entryCount, const char* const* pEntries, bool force = false);
    void erase(const char* pEntry);
    bool contains(const char* pEntry) const;
    void clear();
    uint32_t count() const;
    const char* const* data() const;

private:
    std::set<std::string> validate(std::set<std::string> const& availableEntries);

    std::map<std::string, uint32_t> mEntryIndices;
    std::vector<char const*> mEntries;
};

} // namespace gvk
