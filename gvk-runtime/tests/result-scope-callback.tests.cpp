
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

#include "gvk-defines.hpp"
#include "gvk-string.hpp"

#include "asio.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

struct ErrorInfo
{
    operator bool() const
    {
        assert((vkResult == VK_SUCCESS) == !pFileLine);
        assert((vkResult == VK_SUCCESS) == !pGvkCall);
        return vkResult != VK_SUCCESS;
    }

    VkResult vkResult{ };
    const char* pFileLine{ };
    const char* pGvkCall{ };
    std::thread::id threadId;
};

thread_local ErrorInfo tlErrorInfo;
static VkBool32 process_gvk_error(VkResult vkResult, const char* pFileLine, const char* pGvkCall)
{
    tlErrorInfo.vkResult = vkResult;
    tlErrorInfo.pFileLine = pFileLine;
    tlErrorInfo.pGvkCall = pGvkCall;
    tlErrorInfo.threadId = { };
    return VK_TRUE;
}

static VkBool32 process_gvk_error_multithreaded(VkResult vkResult, const char* pFileLine, const char* pGvkCall)
{
    tlErrorInfo.vkResult = vkResult;
    tlErrorInfo.pFileLine = pFileLine;
    tlErrorInfo.pGvkCall = pGvkCall;
    tlErrorInfo.threadId = std::this_thread::get_id();
    return VK_TRUE;
}

static VkResult test_function_call(VkResult vkResult)
{
    return vkResult;
}

TEST(gvk_result_scope, GlobalCallback)
{
    // Validate that ErrorInfo is empty
    EXPECT_FALSE(tlErrorInfo);

    // Set global gvk_result_scope callback
    gvk::gPfnGvkResultScopeCallback = process_gvk_error;

    // Open gvk_result_scope
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // After a successful call ErrorInfo should remain empty
        gvk_result(test_function_call(VK_SUCCESS));
        EXPECT_FALSE(tlErrorInfo);

        // After a failed call ErrorInfo should be populated
        gvk_result(test_function_call(VK_ERROR_DEVICE_LOST));

        // Shouldn't be here, previous failure should cause gvk_result_scope to bail
        FAIL();
    } gvk_result_scope_end;

    // Validate ErrorInfo, threadId should be default, then reset it
    EXPECT_EQ(gvkResult, VK_ERROR_DEVICE_LOST);
    EXPECT_EQ(tlErrorInfo.vkResult, gvkResult);
    ASSERT_TRUE(tlErrorInfo.pFileLine);
    EXPECT_TRUE(gvk::string::ends_with(gvk::string::scrub_path(tlErrorInfo.pFileLine), "gvk-runtime/tests/result-scope-callback.tests.cpp(93)"));
    ASSERT_TRUE(tlErrorInfo.pGvkCall);
    EXPECT_FALSE(strcmp(tlErrorInfo.pGvkCall, "test_function_call(VK_ERROR_DEVICE_LOST)"));
    EXPECT_EQ(tlErrorInfo.threadId, std::thread::id{ });
    tlErrorInfo = { };
}

TEST(gvk_result_scope, ThreadCallback)
{
    // Validate that ErrorInfo is empty
    EXPECT_FALSE(tlErrorInfo);

    // Set global gvk_result_scope callback
    gvk::gPfnGvkResultScopeCallback = process_gvk_error;

    // Create a thread_pool and run test on multiple threads
    // NOTE : This isn't guarnteed to actually use every available hardware thread
    asio::thread_pool threadPool;
    for (uint32_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
        asio::post(threadPool,
            []()
            {
                // Validate that ErrorInfo is empty
                EXPECT_FALSE(tlErrorInfo);

                // Set thread_local gvk_result_scope callback
                gvk::tlPfnGvkResultScopeCallback = process_gvk_error_multithreaded;

                // Open gvk_result_scope
                gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

                    // After a successful call ErrorInfo should remain empty
                    gvk_result(VK_SUCCESS);
                    EXPECT_FALSE(tlErrorInfo);

                    // After a failed call ErrorInfo should be populated
                    gvk_result(test_function_call(VK_ERROR_DEVICE_LOST));

                    // Shouldn't be here, previous failure should cause gvk_result_scope to bail
                    FAIL();
                } gvk_result_scope_end;

                // Validate ErrorInfo including threadId, then reset it
                EXPECT_EQ(gvkResult, VK_ERROR_DEVICE_LOST);
                EXPECT_EQ(tlErrorInfo.vkResult, gvkResult);
                ASSERT_TRUE(tlErrorInfo.pFileLine);
                EXPECT_TRUE(gvk::string::ends_with(gvk::string::scrub_path(tlErrorInfo.pFileLine), "gvk-runtime/tests/result-scope-callback.tests.cpp(139)"));
                ASSERT_TRUE(tlErrorInfo.pGvkCall);
                EXPECT_FALSE(strcmp(tlErrorInfo.pGvkCall, "test_function_call(VK_ERROR_DEVICE_LOST)"));
                EXPECT_EQ(tlErrorInfo.threadId, std::this_thread::get_id());
                tlErrorInfo = { };
            }
        );
    }

    // Open gvk_result_scope
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // After a successful call ErrorInfo should remain empty
        gvk_result(VK_SUCCESS);
        EXPECT_FALSE(tlErrorInfo);

        // After a failed call ErrorInfo should be populated
        gvk_result(test_function_call(VK_ERROR_DEVICE_LOST));

        // Shouldn't be here, previous failure should cause gvk_result_scope to bail
        FAIL();
    } gvk_result_scope_end;

    // Validate ErrorInfo, threadId should be default, then reset it
    EXPECT_EQ(gvkResult, VK_ERROR_DEVICE_LOST);
    EXPECT_EQ(tlErrorInfo.vkResult, gvkResult);
    ASSERT_TRUE(tlErrorInfo.pFileLine);
    EXPECT_TRUE(gvk::string::ends_with(gvk::string::scrub_path(tlErrorInfo.pFileLine), "gvk-runtime/tests/result-scope-callback.tests.cpp(166)"));
    ASSERT_TRUE(tlErrorInfo.pGvkCall);
    EXPECT_FALSE(strcmp(tlErrorInfo.pGvkCall, "test_function_call(VK_ERROR_DEVICE_LOST)"));
    tlErrorInfo = { };
}
