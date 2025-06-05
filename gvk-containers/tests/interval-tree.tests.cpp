
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

#include "gvk-containers/interval-tree.hpp"
#include "gvk-containers/detail/memory-usage-validator.hpp"

#include "gtest/gtest.h"

/**
This test validates that Overlap<>() works correctly
*/
TEST(Interval, overlap)
{
    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
      -
      -
    */
    gvk::Interval<uint64_t> interval0{ };
    gvk::Interval<uint64_t> interval1{ };
    EXPECT_TRUE(gvk::overlap(interval0, interval1));

    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
                    -
                    -
    */
    interval0 = { 7, 7 };
    interval1 = { 7, 7 };
    EXPECT_TRUE(gvk::overlap(interval0, interval1));

    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
              ---
              -
    */
    interval0 = { 4, 5 };
    interval1 = { 4, 4 };
    EXPECT_TRUE(gvk::overlap(interval0, interval1));

    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
      -----------
          ---------
    */
    interval0 = { 0, 5 };
    interval1 = { 2, 6 };
    EXPECT_TRUE(gvk::overlap(interval0, interval1));

    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
              ---------
          -------
    */
    interval0 = { 4, 8 };
    interval1 = { 2, 5 };
    EXPECT_TRUE(gvk::overlap(interval0, interval1));

    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
      -----------
                ---------
    */
    interval0 = { 0, 5 };
    interval1 = { 5, 9 };
    EXPECT_TRUE(gvk::overlap(interval0, interval1));

    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
                -------
          -------
    */
    interval0 = { 5, 8 };
    interval1 = { 2, 5 };
    EXPECT_TRUE(gvk::overlap(interval0, interval1));

    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
          -------
                    -----
    */
    interval0 = { 2, 5 };
    interval1 = { 7, 9 };
    EXPECT_FALSE(gvk::overlap(interval0, interval1));

    /*
    [ 0 1 2 3 4 5 6 7 8 9 ]
                  -------
        -------
    */
    interval0 = { 6, 9 };
    interval1 = { 1, 4 };
    EXPECT_FALSE(gvk::overlap(interval0, interval1));
}

class MemoryUsageValidator final
{
public:
    MemoryUsageValidator()
    {
        memoryUsageTracker.create_snapshot(&pSnapshot);
    }

    ~MemoryUsageValidator()
    {
        gvk::detail::MemoryUsageTracker::Snapshot* pCurrentSnapshot = nullptr;
        memoryUsageTracker.create_snapshot(&pCurrentSnapshot);
        EXPECT_TRUE(memoryUsageTracker.equal(pSnapshot, pCurrentSnapshot));
    }

    gvk::detail::MemoryUsageTracker memoryUsageTracker;
    gvk::detail::MemoryUsageTracker::Snapshot* pSnapshot{ };
};

/**
This test validates that enumerating Interval<> objects works correctly
*/
TEST(IntervalTree, AddAndEnumerate)
{
#if 0
#ifdef WIN32
#ifdef _DEBUG
    // NOTE : This test has a false positive for outstanding allocations
    //  according to MemoryUsageValidator on Linux.  Running this test with
    //  Valgrind outputs...
    //      "All heap blocks were freed -- no leaks are possible"
    //  Interestingly, the fuzz test below passes fine and it makes many more
    //  allocations than this one.  Maybe this test scenario is causing some
    //  allocation in Google test that isn't cleaned up til sometime after the
    //  validator goes out of scope.
    MemoryUsageValidator memoryUsageValidator;
#endif
#endif
#endif

    /*
       Test0       Test4                   Test12   Test15   Test18   Test21
       .           .                       .        .        .        .
    [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 ]
       .     ------o--------- [2,7](0)     .        .        .        .
       .           o-----------------------o [4,12](1)       .        .
       o--- [0,1](2)                       .        .        .        .
       .  ---------o--- [1,5](3)           .        .        .        .
       .           .                       .        o--------o--- [15,19](4)
       .           .                    ---o--------o [11,15](5)      .
    */
    gvk::IntervalTree<uint64_t, int> intervalTree;
    intervalTree[{ 2,   7 }] = 0;
    intervalTree[{ 4,  12 }] = 1;
    intervalTree[{ 0,   1 }] = 2;
    intervalTree[{ 1,   5 }] = 3;
    intervalTree[{ 15, 19 }] = 4;
    intervalTree[{ 11, 15 }] = 5;
    /*
              [2,7]
             /     \
        [0,1]       [11,15]
             \     /       \
           [1,5] [4,12] [15,19]
    */

    auto validateIntervalValues = [&intervalTree](uint64_t point, const std::vector<std::pair<gvk::Interval<uint64_t>, int>>& expected) {
        std::vector<std::pair<gvk::Interval<uint64_t>, int>> intervalValues;
        intervalTree.enumerate(
            point,
            [&intervalValues](const gvk::Interval<uint64_t>& interval, int value) {
                intervalValues.push_back({ interval, value });
            }
        );
        EXPECT_EQ(expected, intervalValues);
    };

    // Test0
    validateIntervalValues(
        0,
        {
            {{ 0, 1 }, 2 },
        }
    );

    // Test4
    validateIntervalValues(
        4,
        {
            {{ 1,  5 }, 3 },
            {{ 2,  7 }, 0 },
            {{ 4, 12 }, 1 },
        }
    );

    // Test12
    validateIntervalValues(
        12,
        {
            {{ 4,  12 }, 1 },
            {{ 11, 15 }, 5 },
        }
    );

    // Test15
    validateIntervalValues(
        15,
        {
            {{ 11, 15 }, 5 },
            {{ 15, 19 }, 4 },
        }
    );

    // Test18
    validateIntervalValues(
        18,
        {
            {{ 15, 19 }, 4 },
        }
    );

    // Test21
    validateIntervalValues(
        21,
        {
        }
    );

    /*
       Test0       Test4                   Test12   Test15   Test18   Test21
       .           .                       .        .        .        .
    [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 ]
       .     ------o--------- [2,7](0)     .        .        .        .
       .           .                       .        .        .        .
       o--- [0,1](2)                       .        .        .        .
       .           .                       .        .        .        .
       .           .                       .        o--------o--- [15,19](4)
       .           .                       .        .        .        .
    */
    intervalTree.erase({  1,  5 });
    intervalTree.erase({  4, 12 });
    intervalTree.erase({ 11, 15 });
    /*
              [2,7]
             /     \
        [0,1]       [15,9]
    */

    // Test0
    validateIntervalValues(
        0,
        {
            {{ 0, 1 }, 2 },
        }
    );

    // Test4
    validateIntervalValues(
        4,
        {
            {{ 2,  7 }, 0 },
        }
    );

    // Test12
    validateIntervalValues(
        12,
        {
        }
    );

    // Test15
    validateIntervalValues(
        15,
        {
            {{ 15, 19 }, 4 },
        }
    );

    // Test18
    validateIntervalValues(
        18,
        {
            {{ 15, 19 }, 4 },
        }
    );

    // Test21
    validateIntervalValues(
        21,
        {
        }
    );

#if 0
    /*
       Test0       Test4                   Test12   Test15   Test18   Test21
       .           .                       .        .        .        .
    [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 ]
       .     ------o--------- [2,7](0)     .        .        .        .
       .           .                       .        .        .        .
       o--- [0,1](2)                       .        .        .        .
       .           .                       .        .        .        .
       .           .                       .        o--------o--- [15,19](4)
       .           .                       .        .        .        .
    */
    intervalTree.erase({  1,  5 });
    intervalTree.erase({  4, 12 });
    intervalTree.erase({ 11, 15 });
    /*
              [2,7]
             /     \
        [0,1]       [15,9]
    */

    // Test0
    validateIntervalValues(
        0,
        {
            {{ 0, 1 }, 2 },
        }
    );

    // Test4
    validateIntervalValues(
        4,
        {
            {{ 1,  5 }, 3 },
            {{ 2,  7 }, 0 },
            {{ 4, 12 }, 1 },
        }
    );

    // Test12
    validateIntervalValues(
        12,
        {
            {{ 4,  12 }, 1 },
            {{ 11, 15 }, 5 },
        }
    );

    // Test15
    validateIntervalValues(
        15,
        {
            {{ 11, 15 }, 5 },
            {{ 15, 19 }, 4 },
        }
    );

    // Test18
    validateIntervalValues(
        18,
        {
            {{ 15, 19 }, 4 },
        }
    );

    // Test21
    validateIntervalValues(
        21,
        {
        }
    );
#endif
}

/**
@brief This test validates that enumerating Interval<>s works correctly
    @note This test is similar to the one above except that it uses randomized data
*/
TEST(IntervalTree, AddAndEnumerateFuzz)
{
#if 0
#ifdef WIN32
#ifdef _DEBUG
    // NOTE : This test actually passes the MemoryUsageValidator, but it's
    //  disabled for consistency with the test above.
    MemoryUsageValidator memoryUsageValidator;
#endif
#endif
#endif

    const size_t IntervalCount = 1024;
    const uint64_t IntervalMax = 256;
    gvk::IntervalTree<uint64_t, int> intervalTree;
    std::vector<std::map<gvk::Interval<uint64_t>, int>> intervalValues(IntervalMax);

    // We generate IntervalCount Intervals<> with randomized end points...
    for (size_t i = 0; i < IntervalCount; ++i) {
        gvk::Interval<uint64_t> interval{std::minmax((uint64_t)rand() % IntervalMax, (uint64_t)rand() % IntervalMax)};
        // ...then each Interval<> is added to the IntervalTree<>...
        intervalTree[interval] = (int)i;
        // ...then we loop over every point in the Interval<> and add the
        //  Interval<> to the std::map<> at that point...
        for (size_t point = interval.first; point <= interval.second; ++point) {
            intervalValues[point][interval] = (int)i;
        }
    }

    for (size_t i = 0; i < intervalValues.size(); ++i) {
        auto& intervalMap = intervalValues[i];
        // ...then we call EnumerateOverlappingIntervals() for each point and
        //  validate that each enumerated Interval<> and value are present in
        //  the std::map<> at that point and erase the std::map<> entry...
        intervalTree.enumerate(
            i,
            [&intervalMap](const gvk::Interval<uint64_t>& interval, int value) {
                auto itr = intervalMap.find(interval);
                EXPECT_TRUE(itr != intervalMap.end());
                EXPECT_EQ(itr->second, value);
                intervalMap.erase(itr);
            });
        // ...finally, we check that the std::map<> has been completely emptied
        //  indicating that every Interval<> and value were enumerated.
        EXPECT_TRUE(intervalMap.empty());
    }
}
