
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

#include <unordered_map>
#include <mutex>
#include <utility>

namespace gvk {

template <typename KeyType, typename ValueType>
class ThreadSafeUnorderedMap final
{
public:
    using map_type = std::unordered_map<KeyType, ValueType>;

    template <typename ProcessIteratorFunctionType>
    bool enumerate(ProcessIteratorFunctionType processIterator) const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        for (const auto& itr : mMap) {
            if (!processIterator(itr)) {
                return false;
            }
        }
        return true;
    }

    std::pair<typename map_type::iterator, bool> insert(const typename map_type::value_type& value)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mMap.insert(value);
    }

    ValueType get(const KeyType& key) const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        const auto& itr = mMap.find(key);
        return itr != mMap.end() ? itr->second : ValueType{ };
    }

    typename map_type::size_type erase(const KeyType& key)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mMap.erase(key);
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mMap.clear();
    }

private:
    map_type mMap;
    mutable std::mutex mMutex;
};

} // namespace gvk
