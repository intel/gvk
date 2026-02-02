
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

#include <vector>

namespace gvk {

/**
 * @brief Provides value based comparisons for stl containers of pointer types
 */
template<typename PtrType>
struct PtrComparer final
{
    /**
     * @brief Compares two objects referenced by pointer by their values
     * @param [in] lhs A pointer to the left hand side object to compare
     * @param [in] rhs A pointer to the right hand side object to compare
     * @return Whether or not the value of the left hand side object evaluates to less than the value of the right hand side object
     */
    inline bool operator()(const PtrType& lhs, const PtrType& rhs) const
    {
        if (!lhs && !rhs) {
            return false;
        }
        if (!lhs && rhs) {
            return true;
        }
        if (lhs && !rhs) {
            return false;
        }
        return *lhs < *rhs;
    }
};

/**
 * @brief Provides iterators with reference semantics for arrays of pointers
 * @param <T> The type of pointers to enumerate
 */
template<typename T>
class PtrArrayEnumerator
{
public:
    /**
     * @brief An iterator to an enumerated pointer
     */
    class Iterator final
    {
    public:
        /**
         * @brief Constructs an instance of Iterator
         */
        Iterator() = default;

        /**
         * @brief Constructs an instance of Iterator
         * @param [in] ptr The array of pointers to iterate
         */
        inline Iterator(const T** ptr)
            : mPtr{ ptr }
        {
        }

        /**
         * @brief Copies an instance of Iterator
         * @param [in] other The Iterator to copy from
         */
        Iterator(const Iterator& other) = default;

        /**
         * @brief Copies an instance of Iterator
         * @param [in] other The Iterator to copy from
         * @return This Iterator after being copied to
         */
        Iterator& operator=(const Iterator& other) = default;

        /**
         * @brief Increments this Iterator
         * @return This Iterator after being incremented
         */
        inline Iterator& operator++()
        {
            ++mPtr;
            return *this;
        }

        /**
         * @brief Increments this Iterator
         * @return A copy of this Iterator before being incremented
         */
        inline Iterator operator++(int)
        {
            Iterator temp = *this;
            operator++();
            return temp;
        }

        /**
         * @brief Gets a value indicating whether or not this Iterator is equal to a given Iterator
         * @param [in] other The Iterator to compare this Iterator against
         * @return A value indicating whether or not this Iterator is equal to a given Iterator
         */
        inline bool operator==(const Iterator& other)
        {
            return mPtr == other.mPtr;
        }

        /**
         * @brief Gets a value indicating whether or not this Iterator is inequal to a given Iterator
         * @param [in] other The Iterator to compare this Iterator against
         * @return A value indicating whether or not this Iterator is inequal to a given Iterator
         */
        inline bool operator!=(const Iterator& other)
        {
            return !(*this == other);
        }

        /**
         * @brief Dereferences this Iterator's current value
         * @return A pointer to this Iterator's current value
         */
        inline const T* operator->() const
        {
            return mPtr ? *mPtr : nullptr;
        }

        /**
         * @brief Dereferences this Iterator's current value
         * @return A reference to this Iterator's current value
         */
        inline const T& operator*() const
        {
            auto pObj = operator->();
            assert(pObj);
            return *pObj;
        }

    private:
        const T** mPtr{ };
    };

    /**
     * @brief Constructs an instance of PtrArrayEnumerator<>
     */
    PtrArrayEnumerator() = default;

    /**
     * @brief Constructs an instance of PtrArrayEnumerator<>
     * @param [in] pBegin A pointer to the beginning of the range to enumerate via this PtrArrayEnumerator
     * @param [in] pEnd A pointer to the ending of the range to enumerate via this PtrArrayEnumerator
     */
    inline PtrArrayEnumerator(const T** pBegin, const T** pEnd)
        : mBegin(pBegin)
        , mEnd(pEnd)
    {
    }

    /**
     * @brief Gets an Iterator to this PtrArrayEnumerator<>'s first element
     * @return An Iterator to this PtrArrayEnumerator<>'s first element
     */
    inline Iterator begin() const
    {
        return mBegin;
    }

    /**
     * @brief Gets an Iterator to this PtrArrayEnumerator<>'s last element
     * @return An Iterator to this PtrArrayEnumerator<>'s last element
     */
    inline Iterator end() const
    {
        return mEnd;
    }

private:
    Iterator mBegin{ };
    Iterator mEnd{ };
};

/**
 * @brief Resizes a std::vector<> if necessary for a given index
 * @param <T> The type of the std::vector<>
 * @param [in] index The index to validate the std::vector<>'s size for
 * @param [in,out] vector The std::vector<> to validate
 */
template<typename T>
inline void validate_size_for_index(size_t index, std::vector<T>& vector)
{
    if (vector.size() <= index) {
        vector.resize(index + 1);
    }
}

/**
 * @brief Sorts a std::vector<>
 * @param <T> The type of the std::vector<> to sort
 * @param <ComparisonFunctionType> The type of function to use to compare elements of the given std::vector<>
 * @param [in] vector The std::vector<> to sort
 * @param [in] comparisonFunction The functino to use to compare elements of the given std::vector<>
 */
template<typename T, typename ComparisonFunctionType>
inline void sort(std::vector<T>& vector, ComparisonFunctionType comparisonFunction)
{
    std::sort(vector.begin(), vector.end(), comparisonFunction);
}

/**
 * @brief Sorts a std::vector<>
 * @param <T> The type of the std::vector<> to sort
 * @param [in] vector The std::vector<> to sort
 */
template<typename T>
inline void sort(std::vector<T>& vector)
{
    sort(vector, [](const T& lhs, const T& rhs) { return lhs < rhs; });
}

/**
 * @brief Sorts a std::vector<> and removes all duplicates
 * @param <T> The type of the std::vector<> to sort and remove duplicates from
 * @param <ComparisonFunctionType> The type of function to use to compare elements of the given std::vector<>
 * @param [in] vector The std::vector<> to sort and remove duplicates from
 * @param [in] comparisonFunction The functino to use to compare elements of the given std::vector<>
 */
template<typename T, typename ComparisonFunctionType>
inline void sort_and_remove_duplicates(std::vector<T>& vector, ComparisonFunctionType comparisonFunction)
{
    sort(vector, comparisonFunction);
    vector.erase(std::unique(vector.begin(), vector.end()), vector.end());
}

/**
 * @brief Sorts a std::vector<> and removes all duplicates
 * @param <T> The type of the std::vector<> to sort and remove duplicates from
 * @param [in] vector The std::vector<> to sort and remove duplicates from
 */
template<typename T>
inline void sort_and_remove_duplicates(std::vector<T>& vector)
{
    sort_and_remove_duplicates(vector, [](const T& lhs, const T& rhs) { return lhs < rhs; });
}

} // namespace gvk
