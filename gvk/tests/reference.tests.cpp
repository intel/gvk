
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

#include "gvk/detail/reference.hpp"
#include "gvk/system/random.hpp"

#include "asio.hpp"
#include "gtest/gtest.h"

constexpr size_t TestCount = 256;

struct Widget
{
    size_t value{ };
};

struct ExpectedId { size_t value{ }; };
struct ExpectedRefCount { size_t value{ }; };
struct ExpectedValue { size_t value{ }; };

void validate(
    const char* pFileLine,
    const gvk::detail::Reference<Widget>& reference,
    ExpectedId expectedId = { },
    ExpectedRefCount expectedRefCount = { },
    ExpectedValue expectedValue = { }
)
{
    EXPECT_EQ(reference.get_ref_count(), expectedRefCount.value) << pFileLine;
    if (expectedRefCount.value) {
        EXPECT_NE(reference, gvk::detail::nullref) << pFileLine;
        EXPECT_TRUE(reference) << pFileLine;
        EXPECT_EQ(reference.get_id(), expectedId.value) << pFileLine;
        EXPECT_EQ(reference.get_obj().value, expectedValue.value) << pFileLine;
        EXPECT_EQ(reference->value, expectedValue.value) << pFileLine;
        EXPECT_EQ((*reference).value, expectedValue.value) << pFileLine;
    }
    else {
        EXPECT_EQ(reference, gvk::detail::nullref) << pFileLine;
        EXPECT_FALSE(reference) << pFileLine;
    }
}

TEST(Reference, BasicCtorDtor)
{
    gvk::detail::Reference<Widget> reference;
    validate(gvk_file_line, reference);
    reference = gvk::detail::Reference<Widget>(gvk::detail::newref, 1);
    reference->value = 8;
    validate(gvk_file_line, reference, ExpectedId{ 1 }, ExpectedRefCount{ 1 }, ExpectedValue{ 8 });
    reference.reset(gvk::detail::newref, 2);
    validate(gvk_file_line, reference, ExpectedId{ 2 }, ExpectedRefCount{ 1 }, ExpectedValue{ 0 });
    reference = gvk::detail::nullref;
    validate(gvk_file_line, reference);
    reference.reset(gvk::detail::newref, 2);
    validate(gvk_file_line, reference, ExpectedId{ 2 }, ExpectedRefCount{ 1 }, ExpectedValue{ 0 });
    reference.reset();
    validate(gvk_file_line, reference);
}

TEST(Reference, RefCounting)
{
    gvk::detail::Reference<Widget> reference(gvk::detail::newref, 3);
    reference->value = 64;
    std::vector<gvk::detail::Reference<Widget>> references(TestCount);
    for (size_t i = 0; i < references.size(); ++i) {
        references[i] = reference;
        auto expectedRefCount = 1 + i + 1;
        validate(gvk_file_line, references[i], ExpectedId{ 3 }, ExpectedRefCount{ expectedRefCount }, ExpectedValue{ 64 });
        validate(gvk_file_line, reference, ExpectedId{ 3 }, ExpectedRefCount{ expectedRefCount }, ExpectedValue{ 64 });
    }
    validate(gvk_file_line, reference, ExpectedId{ 3 }, ExpectedRefCount{ references.size() + 1 }, ExpectedValue{ 64 });
    for (size_t i = 0; i < references.size(); ++i) {
        EXPECT_EQ(reference, references[i]);
    }
    references.clear();
    validate(gvk_file_line, reference, ExpectedId{ 3 }, ExpectedRefCount{ 1 }, ExpectedValue{ 64 });
    reference = gvk::detail::nullref;
    validate(gvk_file_line, reference);
}

struct Operation
{
    enum class Type
    {
        Create,
        Destroy,
        Count,
    };

    Operation() = default;

    Operation(size_t referenceCount, gvk::sys::RandomNumberGenerator& rng)
        : type { (Operation::Type)rng.die_roll((size_t)Operation::Type::Count) }
        , index { rng.index(referenceCount) }
    {
    }

    Type type { };
    size_t index { };
};

void apply_operation(
    const Operation& operation,
    const std::vector<gvk::detail::Reference<Widget>>& references,
    std::map<size_t, std::vector<gvk::detail::Reference<Widget>>>& referenceMap
)
{
    static std::mutex sMutex;
    switch (operation.type) {
    case Operation::Type::Create:
    {
        // Note that the reference is created outside of the lock used to protect the
        //  collection of references used by the test...this is because this test's
        //  ensures that gvk::detail::Reference<> is able to correctly handle creating
        //  and destroying references asynchronously so the test creates/destroys
        //  them outside of the lock...
        auto reference = references[operation.index];
        std::lock_guard<std::mutex> lock(sMutex);
        referenceMap[operation.index].push_back(reference);
    } break;
    case Operation::Type::Destroy:
    {
        std::unique_lock<std::mutex> lock(sMutex);
        assert(!referenceMap[operation.index].empty());
        auto reference = std::move(referenceMap[operation.index].back());
        referenceMap[operation.index].pop_back();
        lock.unlock();
        //  ...reference destroyed outside the lock for the reason described above...
        reference = gvk::detail::nullref;
    } break;
    }
}

std::vector<size_t> apply_operations(
    const std::vector<Operation>& operations,
    const std::vector<gvk::detail::Reference<Widget>>& references,
    asio::thread_pool* pThreadPool = nullptr
)
{
    // Creating references.size() references to each gvk::detail::Reference<Widget>.
    //  This ensures that the order/count of Create/Delete Operations won't affect
    //  the outcome of the multithreaded test by creating enough references that
    //  Destroy Operations will never run out of references to destroy...
    std::map<size_t, std::vector<gvk::detail::Reference<Widget>>> referenceMap;
    for (size_t i = 0; i < references.size(); ++i) {
        referenceMap[i].resize(references.size(), references[i]);
    }

    // Run operations, either via asio::thread_pool, or on the calling thread...
    for (const auto& operation : operations) {
        if (pThreadPool) {
            asio::post(*pThreadPool,
                [&]()
                {
                    apply_operation(operation, references, referenceMap);
                }
            );
        } else {
            apply_operation(operation, references, referenceMap);
        }
    }
    if (pThreadPool) {
        pThreadPool->wait();
    }

    // Return the resulting ref counts for each reference...
    std::vector<size_t> refCounts(references.size());
    for (size_t i = 0; i < refCounts.size(); ++i) {
        refCounts[i] = references[i].get_ref_count();
    }
    return refCounts;
}

TEST(Reference, MultithreadedCtorDtor)
{
    // Create a collection of references...
    std::vector<gvk::detail::Reference<Widget>> references(TestCount);
    for (size_t i = 0; i < references.size(); ++i) {
        references[i].reset(gvk::detail::newref);
        references[i]->value = i;
    }

    // Create a randomized sequence of operations to apply to the collection...
    gvk::sys::RandomNumberGenerator rng;
    std::vector<Operation> operations(references.size());
    for (auto& operation : operations) {
        operation = Operation(references.size(), rng);
    }

    // Compare the results of running operations on one vs many threads...
    // gvk::sys::ThreadPool threadPool;
    asio::thread_pool threadPool;
    auto singleThreadedResults = apply_operations(operations, references, nullptr);
    auto multiThreadedResults = apply_operations(operations, references, &threadPool);
    EXPECT_EQ(singleThreadedResults, multiThreadedResults);
}

TEST(Reference, LookupById)
{
    EXPECT_EQ(gvk::detail::Reference<Widget>::get({ }), gvk::detail::nullref);
    gvk::detail::Reference<Widget> reference(gvk::detail::newref, 3);
    auto id = reference.get_id();
    EXPECT_EQ(id, 3);
    EXPECT_EQ(gvk::detail::Reference<Widget>::get(id), reference);
    reference = gvk::detail::nullref;
    EXPECT_EQ(gvk::detail::Reference<Widget>::get(id), gvk::detail::nullref);
}

TEST(Reference, Enumerate)
{
    // Create a collection of references...
    std::vector<gvk::detail::Reference<Widget>> references(TestCount);
    for (size_t i = 0; i < references.size(); ++i) {
        references[i].reset(gvk::detail::newref);
        references[i]->value = i;
    }

    // Fill a std::set<> with the ids and values of each Widget...
    std::set<std::pair<size_t, size_t>> expectedValues;
    for (const auto& reference : references) {
        expectedValues.insert({ reference.get_id(), reference->value });
    }
    EXPECT_EQ(expectedValues.size(), references.size());

    // Fill another std::set<> with the ids and values of each Widget encountered
    //  as a result of calling enumerate() and compare the result...
    std::set<std::pair<size_t, size_t>> actualValues;
    gvk::detail::Reference<Widget>::enumerate(
        [&actualValues](const auto& reference)
        {
            actualValues.insert({ reference.get_id(), reference->value });
        }
    );
    EXPECT_EQ(actualValues, expectedValues);
}
