
/******************************************************************************

© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.

 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

#include <vector>

namespace gvk {

/**
A sorted container of unique elements with contiguous storage
@param <T> The type of eleemnts to store
*/
template<typename T>
class ContiguousSet final
{
public:
    /**
    Constructs an instance of ContiguousSet<>
    */
    ContiguousSet() = default;

    /**
    Gets an iterator to this ContiguousSet<>'s first element
    @return An iterator to this ContiguousSet<>'s first element
    */
    typename std::vector<T>::const_iterator begin() const
    {
        return mElements.begin();
    }

    /**
    Gets an iterator to this ContiguousSet<>'s last element
    @return An iterator to this ContiguousSet<>'s last element
    */
    typename std::vector<T>::const_iterator end() const
    {
        return mElements.end();
    }

    /**
    Gets a value indicating whether or not this ContiguousSet<> is empty
    @return Whether or not this ContiguousSet<> is empty
    */
    bool empty() const
    {
        return mElements.empty();
    }

    /**
    Gets the number of elements in this ContiguousSet<>
    @return The number of elements in this ContiguousSet<>
    */
    size_t size() const
    {
        return mElements.size();
    }

    /**
    Clears all elements from this ContiguousSet<>
    */
    void clear()
    {
        mElements.clear();
    }

    /**
    Inserts an element into this ContiguousSet<> if the element is not already present
    @param [in] element The element to insert
    @return An std::pair<> consisting of an iterator to the inserted element and a value indicating whether or not an insertion occured
    */
    std::pair<typename std::vector<T>::const_iterator, bool> insert(const T& element)
    {
        auto itr = std::lower_bound(mElements.begin(), mElements.end(), element);
        if (itr == mElements.end() || *itr != element) {
            itr = mElements.insert(itr, element);
            return { itr, true };
        }
        return { itr, false };
    }

    /**
    TODO : Documentation
    Inserts a range of elements into this ContiguousSet<>
    @param [in] element The element to insert
    @return An std::pair<> consisting of an iterator to the inserted element and a value indicating whether or not an insertion occured
    */
    template<typename InputIt>
    void insert(InputIt first, InputIt last)
    {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    /**
    Finds the given element if it is present in this ContiguousSet<>
    @param [in] element The element to find
    @return An iterator to the element or end() if the element isn't found
    */
    typename std::vector<T>::const_iterator find(const T& element) const
    {
        auto itr = std::lower_bound(mElements.begin(), mElements.end(), element);
        return (itr != mElements.end() && *itr == element) ? itr : mElements.end();
    }

    /**
    Gets a pointer to this ContiguousSet<> object's underlying storage
    @return A pointer to this ContiguousSet<> object's underlying storage
    */
    const T* data() const
    {
        return mElements.data();
    }

private:
    std::vector<T> mElements;
};

} // namespace gvk
