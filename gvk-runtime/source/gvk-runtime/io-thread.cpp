
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

#include "gvk-runtime/io-thread.hpp"

#include <utility>

namespace gvk {

#ifdef GVK_PLATFORM_WINDOWS

IoThread::IoThread(IoThread&& other) noexcept
{
    *this = std::move(other);
}

IoThread& IoThread::operator=(IoThread&& other) noexcept
{
    if (this != &other) {
        reset();
        mHandle = std::exchange(other.mHandle, { });
        mId = std::exchange(other.mId, 0);
    }
    return *this;
}

IoThread::~IoThread()
{
    reset();
}

IoThread::operator bool() const
{
    assert(!mHandle == !mId);
    return mHandle && mId;
}

BOOL IoThread::create(const CreateInfo* pCreateInfo, IoThread* pIoThread)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(pCreateInfo);
        gvk_result_assert(pCreateInfo->lpThreadProc);
        gvk_result_assert(pCreateInfo->lpParameter);
        gvk_result_assert(pIoThread);
        pIoThread->reset();
        pIoThread->mHandle = CreateThread(0, 0, pCreateInfo->lpThreadProc, pCreateInfo->lpParameter, 0, &pIoThread->mId);
        if (!pIoThread->mHandle) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
    } gvk_result_scope_end;
    if (gvkResult != VK_SUCCESS) {
        pIoThread->reset();
    }
    return gvkResult == VK_SUCCESS;
}

void IoThread::reset()
{
    if (mHandle) {
        (void)CloseHandle(mHandle);
    }
    mHandle = NULL;
    mId = 0;
}

HANDLE IoThread::get_handle() const
{
    return mHandle;
}

DWORD IoThread::get_id() const
{
    return mId;
}

#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk
