
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

#include "gvk/system/random.hpp"

#include "gtest/gtest.h"

#include <vector>

static constexpr int MaxValue { 4 };
static constexpr int MinValue { -MaxValue };
static constexpr int TestCount { 1024 };

TEST(RandomNumberGenerator, range)
{
    gvk::sys::RandomNumberGenerator rng;

    // int stays in range
    for (int i = 0; i < TestCount; ++i) {
        auto value = rng.range(MinValue, MaxValue);
        if (value < MinValue || MaxValue < value) {
            FAIL();
        }
    }

    // float stays in range
    for (int i = 0; i < TestCount; ++i) {
        auto value = rng.range((float)MinValue * 0.1f, (float)MaxValue * 0.1f);
        if (value < MinValue || MaxValue < value) {
            FAIL();
        }
    }
}

TEST(RandomNumberGenerator, probability)
{
    gvk::sys::RandomNumberGenerator rng;

    // int 0 always fails
    for (int i = 0; i < TestCount; ++i) {
        if (rng.probability(0)) {
            FAIL();
        }
    }

    // int 100 always passes
    for (int i = 0; i < TestCount; ++i) {
        if (!rng.probability(100)) {
            FAIL();
        }
    }

    // float 0.0f always fails
    for (int i = 0; i < TestCount; ++i) {
        if (rng.probability(0.0f)) {
            FAIL();
        }
    }

    // float 1.0f always passes
    for (int i = 0; i < TestCount; ++i) {
        if (!rng.probability(1.0f)) {
            FAIL();
        }
    }
}

TEST(RandomNumberGenerator, index)
{
    gvk::sys::RandomNumberGenerator rng;

    // Count 0 always gets index 0
    for (int i = 0; i < TestCount; ++i) {
        if (rng.index(0) != 0) {
            FAIL();
        }
    }

    // Count 1 always gets index 0
    for (int i = 0; i < TestCount; ++i) {
        if (rng.index(1) != 0) {
            FAIL();
        }
    }

    // Count 8 stays in range
    for (int i = 0; i < TestCount; ++i) {
        auto index = rng.index(8);
        if (index < 0 || 7 < index) {
            FAIL();
        }
    }
}

TEST(RandomNumberGenerator, die_roll)
{
    gvk::sys::RandomNumberGenerator rng;

    // D0 always rolls 0
    for (int i = 0; i < TestCount; ++i) {
        if (rng.die_roll(0) != 0) {
            FAIL();
        }
    }

    // D1 always rolls 1
    for (int i = 0; i < TestCount; ++i) {
        if (rng.die_roll(1) != 1) {
            FAIL();
        }
    }

    // D6 stays in range
    for (int i = 0; i < TestCount; ++i) {
        auto diRoll = rng.die_roll(6);
        if (diRoll < 0 || 6 < diRoll) {
            FAIL();
        }
    }
}

TEST(RandomNumberGenerator, reset)
{
    gvk::sys::RandomNumberGenerator rng;
    std::vector<int> ints(TestCount);
    std::vector<float> floats(TestCount);
    for (size_t i = 0; i < TestCount; ++i) {
        ints[i] = rng.value<int>();
        floats[i] = rng.value<float>();
    }
    rng.reset();
    for (size_t i = 0; i < TestCount; ++i) {
        if (rng.value<int>() != ints[i] || rng.value<float>() != floats[i]) {
            FAIL();
        }
    }
}
