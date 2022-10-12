
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

#include <limits>
#include <random>
#include <type_traits>

namespace gvk {
namespace sys {

/**
Provides high level control over random number generation
*/
class RandomNumberGenerator final
{
public:
    /**
    Constructs an instance of RandomNumberGenerator
    */
    inline RandomNumberGenerator()
        : RandomNumberGenerator(std::random_device()())
    {
    }

    /**
    Constructs an instance of RandomNumberGenerator
    @param [in] seed This RandomNumberGenerator object's seed
    */
    inline RandomNumberGenerator(uint32_t seed)
    {
        set_seed(seed);
    }

    /**
    Gets this RandomNumberGenerator object's seed
    @return This RandomNumberGenerator object's seed
    */
    inline uint32_t get_seed() const
    {
        return mSeed;
    }

    /**
    Sets this RandomNumberGenerator object's seed
    @param [in] seed This RandomNumberGenerator object's seed
        @note This method resets this RandomNumberGenerator
    */
    inline void set_seed(uint32_t seed)
    {
        mSeed = seed;
        reset();
    }

    /**
    Resets this RandomNumberGenerator<> with its current seed
    */
    inline void reset()
    {
        mEngine.seed(mSeed);
    }

    /**
    Generates a random number in a specified range
    @param <T> The type of random number to generate
    @param [in] min The lower bound of the range [inclusive]
    @param [in] max The upper bound of the range [inclusive]
    @return The generated number
    */
    template <typename T>
    inline typename std::enable_if<std::is_integral<T>::value, T>::type range(T min, T max)
    {
        return std::uniform_int_distribution<T>(min, max)(mEngine);
    }

    /**
    Generates a random number in a specified range
    @param <T> The type of random number to generate
    @param [in] min The lower bound of the range [inclusive]
    @param [in] max The upper bound of the range [inclusive]
    @return The generated number
    */
    template <typename T>
    inline typename std::enable_if<std::is_floating_point<T>::value, T>::type range(T min, T max)
    {
        return std::uniform_real_distribution<T>(min, max)(mEngine);
    }

    /**
    Generates a random value in a specified type's range
    @param <T> The type of random number to generate
    @return The generated number
    */
    template <typename T>
    inline T value()
    {
        return range<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }

    /**
    Gets a value indicating whether or not a specified value was greater than or equal to a random number in the range [1 - 100]
    @param <T> The type of value to test
    @param [in] value The value to test
    @return Whether or not the value passed
    */
    template <typename T>
    inline bool probability(T value, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr)
    {
        return value >= range<T>(T { 1 }, T { 100 });
    }

    /**
    Gets a value indicating whether or not a specified value was greater than or equal to a random number in the range (0.0 - 1.0]
    @param <T> The type of value to test
    @param [in] value The value to test
    @return Whether or not the value passed
    */
    template <typename T>
    inline bool probability(T value, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr)
    {
        return value >= range<T>(std::numeric_limits<T>::epsilon(), T { 1.0 });
    }

    /**
    Generates a random index for a collection with a specified count
    @param <T> The type of random number to generate
    @param [in] count The number of elements in the collection
    @return The generated index
        @note This method can only be used with integral types
    */
    template <typename T>
    inline T index(T count)
    {
        static_assert(std::is_integral<T>::value, "gvk::sys::RandomNumberGenerator::index<>() can only be used with integer types");
        return count > T { 0 } ? range<T>(T { 0 }, count - T { 1 }) : T { 0 };
    }

    /**
    Generates a roll from a die with a specified number of sides
    @param <T> The type of random number to generate
    @param [in] D The number of sides on the die
    @return The result of the die roll
        @note This method can only be used with integral types
    */
    template <typename T>
    inline T die_roll(T D)
    {
        static_assert(std::is_integral<T>::value, "gvk::sys::RandomNumberGenerator::die_roll<>() can only be used with integer types");
        return D > T { 0 } ? range<T>(T { 1 }, D) : T { 0 };
    }

private:
    uint32_t mSeed { };
    std::mt19937 mEngine;
};

} // namespace sys
} // namespace gvk
