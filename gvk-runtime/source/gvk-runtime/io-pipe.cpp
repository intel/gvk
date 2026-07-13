
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

#include "gvk-runtime/io-pipe.hpp"

#include <utility>

namespace gvk {

#ifdef GVK_PLATFORM_WINDOWS

IoPipe::IoPipe(IoPipe&& other) noexcept
{
    *this = std::move(other);
}

IoPipe& IoPipe::operator=(IoPipe&& other) noexcept
{
    if (this != &other) {
        reset();
        mReadHandle = std::exchange(other.mReadHandle, { });
        mWriteHandle = std::exchange(other.mWriteHandle, { });
    }
    return *this;
}

IoPipe::~IoPipe()
{
    reset();
}

IoPipe::operator bool() const
{
    return mReadHandle || mWriteHandle;
}

BOOL IoPipe::create(const CreateInfo* pCreateInfo, IoPipe* pIoPipe)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(pCreateInfo);
        gvk_result_assert(pCreateInfo->inherit);
        gvk_result_assert(pIoPipe);
        pIoPipe->reset();

        // Create anonymous pipe pair
        SECURITY_ATTRIBUTES securityAtributes{ };
        securityAtributes.nLength = sizeof(securityAtributes);
        securityAtributes.bInheritHandle = FALSE;
        if (!CreatePipe(&pIoPipe->mReadHandle, &pIoPipe->mWriteHandle, &securityAtributes, 0)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
        if (!pIoPipe->mReadHandle) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
        if (!pIoPipe->mWriteHandle) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }

        // Set inheritance
        if (!SetHandleInformation(pIoPipe->mReadHandle, HANDLE_FLAG_INHERIT, (pCreateInfo->inherit & Read) ? HANDLE_FLAG_INHERIT : 0)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
        if (!SetHandleInformation(pIoPipe->mWriteHandle, HANDLE_FLAG_INHERIT, (pCreateInfo->inherit & Write) ? HANDLE_FLAG_INHERIT : 0)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
    } gvk_result_scope_end;
    if (gvkResult != VK_SUCCESS) {
        pIoPipe->reset();
    }
    return gvkResult == VK_SUCCESS;
}

void IoPipe::reset()
{
    close(Read | Write);
}

HANDLE IoPipe::get_read_handle() const
{
    return mReadHandle;
}

HANDLE IoPipe::get_write_handle() const
{
    return mWriteHandle;
}

void IoPipe::close(Flags flags)
{
    if ((flags & Read) && mReadHandle) {
        (void)CloseHandle(mReadHandle);
        mReadHandle = NULL;
    }
    if ((flags & Write) && mWriteHandle) {
        (void)CloseHandle(mWriteHandle);
        mWriteHandle = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
// NamedPipe

NamedPipe::NamedPipe(NamedPipe&& other) noexcept
{
    *this = std::move(other);
}

NamedPipe& NamedPipe::operator=(NamedPipe&& other) noexcept
{
    if (this != &other) {
        reset();
        mHandle = std::exchange(other.mHandle, (HANDLE)NULL);
        mEvent = std::exchange(other.mEvent, (HANDLE)NULL);
        mOverlapped = other.mOverlapped;
        other.mOverlapped = { };
        mPendingAccept = std::exchange(other.mPendingAccept, false);
    }
    return *this;
}

NamedPipe::~NamedPipe()
{
    reset();
}

NamedPipe::operator bool() const
{
    return mHandle != NULL;
}

BOOL NamedPipe::create_server(const ServerCreateInfo* pCreateInfo, NamedPipe* pNamedPipe)
{
    assert(pCreateInfo && pCreateInfo->pName && pNamedPipe);
    pNamedPipe->reset();
    pNamedPipe->mHandle = CreateNamedPipeA(
        pCreateInfo->pName,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        65536, 65536, 0, nullptr
    );
    if (pNamedPipe->mHandle == INVALID_HANDLE_VALUE) {
        pNamedPipe->mHandle = NULL;
        return FALSE;
    }
    pNamedPipe->mEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    if (!pNamedPipe->mEvent) {
        pNamedPipe->reset();
        return FALSE;
    }
    return TRUE;
}

BOOL NamedPipe::connect_client(const ClientCreateInfo* pCreateInfo, NamedPipe* pNamedPipe)
{
    assert(pCreateInfo && pCreateInfo->pName && pNamedPipe);
    pNamedPipe->reset();
    auto deadline = GetTickCount64() + pCreateInfo->timeoutMs;
    while (GetTickCount64() < deadline) {
        pNamedPipe->mHandle = CreateFileA(
            pCreateInfo->pName,
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr
        );
        if (pNamedPipe->mHandle != INVALID_HANDLE_VALUE) {
            return TRUE;
        }
        auto error = GetLastError();
        if (error == ERROR_PIPE_BUSY) {
            WaitNamedPipeA(pCreateInfo->pName, 1000);
        } else if (error == ERROR_FILE_NOT_FOUND) {
            Sleep(100);
        } else {
            break;
        }
    }
    pNamedPipe->mHandle = NULL;
    return FALSE;
}

BOOL NamedPipe::begin_accept()
{
    assert(mHandle && mEvent);
    ResetEvent(mEvent);
    mOverlapped = { };
    mOverlapped.hEvent = mEvent;
    mPendingAccept = true;
    if (!ConnectNamedPipe(mHandle, &mOverlapped)) {
        auto error = GetLastError();
        if (error == ERROR_IO_PENDING) {
            return TRUE;
        }
        if (error == ERROR_PIPE_CONNECTED) {
            SetEvent(mEvent);
            return TRUE;
        }
        mPendingAccept = false;
        return FALSE;
    }
    SetEvent(mEvent);
    return TRUE;
}

bool NamedPipe::is_accepted()
{
    if (!mPendingAccept) {
        return false;
    }
    if (WaitForSingleObject(mEvent, 0) == WAIT_OBJECT_0) {
        mPendingAccept = false;
        return true;
    }
    return false;
}

void NamedPipe::disconnect()
{
    if (mHandle) {
        DisconnectNamedPipe(mHandle);
    }
    mPendingAccept = false;
}

HANDLE NamedPipe::get_handle() const
{
    return mHandle;
}

HANDLE NamedPipe::release()
{
    auto h = mHandle;
    mHandle = NULL;
    return h;
}

void NamedPipe::reset()
{
    if (mHandle) {
        CloseHandle(mHandle);
        mHandle = NULL;
    }
    if (mEvent) {
        CloseHandle(mEvent);
        mEvent = NULL;
    }
    mOverlapped = { };
    mPendingAccept = false;
}

#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk
