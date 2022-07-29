
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
