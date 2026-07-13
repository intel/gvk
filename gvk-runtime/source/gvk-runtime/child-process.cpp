
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

#include "gvk-runtime/child-process.hpp"
#include "gvk-string/to-string.hpp"
#include "gvk-runtime.hpp"

#include <array>
#include <iostream>
#include <vector>

namespace gvk {

#ifdef GVK_PLATFORM_WINDOWS

ChildProcess::ChildProcess(ChildProcess&& other) noexcept
{
    *this = std::move(other);
}

ChildProcess& ChildProcess::operator=(ChildProcess&& other) noexcept
{
    if (this != &other) {
        reset();
        mCmdLine = std::exchange(other.mCmdLine, { });
        mUnicodeConversionEnabled = std::exchange(other.mUnicodeConversionEnabled, TRUE);
        mProcessInfo = std::exchange(other.mProcessInfo, { });
        mShutdownEvent = std::exchange(other.mShutdownEvent, { });
        mShutdownJob = std::exchange(other.mShutdownJob, { });
        mStdIn = std::exchange(other.mStdIn, { });
        mStdOut = std::exchange(other.mStdOut, { });
        mStdErr = std::exchange(other.mStdErr, { });
        mStdInThread = std::exchange(other.mStdInThread, { });
        mStdOutThread = std::exchange(other.mStdOutThread, { });
        mStdErrThread = std::exchange(other.mStdErrThread, { });
        mpFnOnStdOut = std::exchange(other.mpFnOnStdOut, { });
        mpFnOnStdErr = std::exchange(other.mpFnOnStdErr, { });
        mpFnOnShutdown = std::exchange(other.mpFnOnShutdown, { });
        mpUserData = std::exchange(other.mpUserData, { });
    }
    return *this;
}

ChildProcess::~ChildProcess()
{
    reset();
}

ChildProcess::operator bool() const
{
    return running();
}

BOOL ChildProcess::create(const CreateInfo* pCreateInfo, ChildProcess* pChildProcess)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(pCreateInfo);
        gvk_result_assert(pChildProcess);
        pChildProcess->reset();

        // Set cmd line
        pChildProcess->mCmdLine = pCreateInfo->pCmdLine ? pCreateInfo->pCmdLine : std::string();

        // Set unicode conversion
        pChildProcess->mUnicodeConversionEnabled = pCreateInfo->unicodeConversionEnabled;

        // Set IO callbacks
        pChildProcess->mpFnOnStdOut = pCreateInfo->pFnOnStdOut;
        pChildProcess->mpFnOnStdErr = pCreateInfo->pFnOnStdErr;

        // Create IO Pipes
        gvk::IoPipe::CreateInfo ioPipeCreateInfo{ };
        ioPipeCreateInfo.inherit = gvk::IoPipe::Read;
        if (!gvk::IoPipe::create(&ioPipeCreateInfo, &pChildProcess->mStdIn)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
        ioPipeCreateInfo.inherit = gvk::IoPipe::Write;
        if (!gvk::IoPipe::create(&ioPipeCreateInfo, &pChildProcess->mStdOut)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
        if (!gvk::IoPipe::create(&ioPipeCreateInfo, &pChildProcess->mStdErr)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }

        // Create IO threads
        gvk::IoThread::CreateInfo ioThreadCreateInfo{ };
        ioThreadCreateInfo.lpThreadProc = on_io;
        ioThreadCreateInfo.lpParameter = pChildProcess;
        if (!gvk::IoThread::create(&ioThreadCreateInfo, &pChildProcess->mStdInThread)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
        if (!gvk::IoThread::create(&ioThreadCreateInfo, &pChildProcess->mStdOutThread)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
        if (!gvk::IoThread::create(&ioThreadCreateInfo, &pChildProcess->mStdErrThread)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }

        // Setup LPSECURITY_ATTRIBUTES
        SECURITY_ATTRIBUTES securityAttributes{ };
        securityAttributes.nLength = sizeof(securityAttributes);
        securityAttributes.bInheritHandle = TRUE;

        // Setup STARTUPINFO
        STARTUPINFO startupInfo{ };
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.hStdInput = pChildProcess->mStdIn.get_read_handle();
        startupInfo.hStdOutput = pChildProcess->mStdOut.get_write_handle();
        startupInfo.hStdError = pChildProcess->mStdErr.get_write_handle();
        if (startupInfo.hStdInput || startupInfo.hStdOutput || startupInfo.hStdError) {
            startupInfo.dwFlags = STARTF_USESTDHANDLES;
        }

        // Create process
        if (!CreateProcess(
            NULL,
            pChildProcess->mCmdLine.data(),
            &securityAttributes,
            NULL,
            TRUE,
            0,
            pCreateInfo->pEnvironment ? pCreateInfo->pEnvironment : NULL,
            pCreateInfo->pDirectory ? pCreateInfo->pDirectory : NULL,
            &startupInfo,
            &pChildProcess->mProcessInfo
        )) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }

        // Enable virtual terminal input and processed input modes for the child process
        //  stdin so the child process can receive ANSI escape sequences
        DWORD mode = 0;
        if (GetConsoleMode(pChildProcess->mStdIn.get_write_handle(), &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_PROCESSED_INPUT;
            (void)SetConsoleMode(pChildProcess->mStdIn.get_write_handle(), mode);
        }

        // Close parent handle to child process pipe ends
        pChildProcess->mStdIn.close(gvk::IoPipe::Read);
        pChildProcess->mStdOut.close(gvk::IoPipe::Write);
        pChildProcess->mStdErr.close(gvk::IoPipe::Write);

        // Register callback for shutdown
        if (!RegisterWaitForSingleObject(
            &pChildProcess->mShutdownEvent,
            pChildProcess->mProcessInfo.hProcess,
            on_shutdown,
            pChildProcess,
            INFINITE,
            WT_EXECUTEONLYONCE
        )) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }

        // Create shutdown job to shutdown child process along with parent process
        pChildProcess->mShutdownJob = CreateJobObject(NULL, NULL);
        gvk_result_assert(pChildProcess->mShutdownJob);
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo{ };
        jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (!SetInformationJobObject(pChildProcess->mShutdownJob, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo))) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }
        if (!AssignProcessToJobObject(pChildProcess->mShutdownJob, pChildProcess->mProcessInfo.hProcess)) {
            gvk_result_scope_break(VK_ERROR_INITIALIZATION_FAILED);
        }

        // Set shutdown callback and user data
        pChildProcess->mpFnOnShutdown = pCreateInfo->pFnOnShutdown;
        pChildProcess->mpUserData = pCreateInfo->pUserData;
    } gvk_result_scope_end;
    if (gvkResult != VK_SUCCESS) {
        pChildProcess->reset();
    }
    return gvkResult == VK_SUCCESS;
}

void ChildProcess::reset()
{
    // Unsubscribe from shutdown event
    if (mShutdownEvent) {
        (void)UnregisterWait(mShutdownEvent);
        mShutdownEvent = NULL;
    }

    // Clear IO callbacks
    mpFnOnStdOut = nullptr;
    mpFnOnStdErr = nullptr;

    // Fire shutdown callback
    if (mpFnOnShutdown) {
        mpFnOnShutdown(*this);
        mpFnOnShutdown = nullptr;
    }

    // Terminate process and close handles
    if (mShutdownJob) {
        (void)CloseHandle(mShutdownJob);
        mShutdownJob = NULL;
    }
    if (mProcessInfo.hProcess) {
        #if 0
        // NOTE : Closing the shutdown job handle terminates the process
        (void)TerminateProcess(mProcessInfo.hProcess, 0);
        #endif
        (void)CloseHandle(mProcessInfo.hProcess);
    }
    if (mProcessInfo.hThread) {
        (void)CloseHandle(mProcessInfo.hThread);
    }
    mProcessInfo = { };

    // Close IO pipes to unblock IO threads and allow them to exit
    mStdIn.reset();
    mStdOut.reset();
    mStdErr.reset();

    // Close IO threads
    mStdInThread.reset();
    mStdOutThread.reset();
    mStdErrThread.reset();

    // Clear cmd line, user data, and reset unicode conversion flag
    mCmdLine.clear();
    mpUserData = nullptr;
    mUnicodeConversionEnabled = TRUE;
}

DWORD ChildProcess::get_pid() const
{
    return mProcessInfo.dwProcessId;
}

const std::string& ChildProcess::get_cmd_line() const
{
    return mCmdLine;
}

void* ChildProcess::get_user_data() const
{
    return mpUserData;
}

bool ChildProcess::running() const
{
    return mProcessInfo.hProcess != NULL;
}

BOOL ChildProcess::redirect_io(HANDLE read, HANDLE write, void(*pFnOnIo)(const ChildProcess& childProcess, size_t dataSize, const char* pData)) const
{
    BOOL success = read != NULL;
    while (success) {
        DWORD charsRead = 0;
        constexpr uint32_t BufferSize = 1024;
        thread_local std::vector<char> charBuffer(BufferSize);
        success = gvk::is_console(read) ?
            ReadConsole(read, charBuffer.data(), (DWORD)charBuffer.size(), &charsRead, NULL) :
            ReadFile(read, charBuffer.data(), (DWORD)charBuffer.size(), &charsRead, NULL);
        if (success && charsRead) {

            // Fire callback if set, otherwise write to output
            if (pFnOnIo) {
                pFnOnIo(*this, charsRead, charBuffer.data());
            } else if (write && mUnicodeConversionEnabled && gvk::is_console(write)) {

                // Convert narrow char buffer to wide char buffer for console output so that Unicode
                //  characters are properly displayed in a Windows console
                thread_local std::vector<wchar_t> wcharBuffer(BufferSize);
                wcharBuffer.clear();
                uint32_t wcharBufferSize = 0;
                gvk::to_wstring(charsRead, charBuffer.data(), &wcharBufferSize, nullptr);
                wcharBuffer.resize(wcharBufferSize);
                gvk::to_wstring(charsRead, charBuffer.data(), &wcharBufferSize, wcharBuffer.data());

                // Write wide character buffer to console in chunks until all data is written
                DWORD offset = 0;
                while (success && offset < wcharBuffer.size()) {
                    DWORD wcharsWritten = 0;
                    success = WriteConsoleW(write, wcharBuffer.data() + offset, (DWORD)wcharBuffer.size() - offset, &wcharsWritten, NULL);
                    if (success) {
                        offset += wcharsWritten;
                    } else {
                        break;
                    }
                }
                if (success) {
                    FlushFileBuffers(write);
                }
            } else if (write) {

                // Write character buffer to file/pipe in chunks until all data is written
                DWORD offset = 0;
                while (success && offset < charsRead) {
                    DWORD charsWritten = 0;
                    success = WriteFile(write, charBuffer.data() + offset, charsRead - offset, &charsWritten, NULL);
                    if (success) {
                        offset += charsWritten;
                    } else {
                        break;
                    }
                }
                if (success) {
                    FlushFileBuffers(write);
                }
            }
        } else if (!success) {
            break;
        }
    }
    return success;
}

DWORD CALLBACK ChildProcess::on_io(_In_ LPVOID lpParameter)
{
    assert(lpParameter);
    const auto& childProcess = *(ChildProcess*)lpParameter;
    auto currentThreadId = GetCurrentThreadId();
    if (currentThreadId == childProcess.mStdInThread.get_id()) {
        while (childProcess.redirect_io(GetStdHandle(STD_INPUT_HANDLE), childProcess.mStdIn.get_write_handle(), nullptr)) {
        }
    } else if (currentThreadId == childProcess.mStdOutThread.get_id()) {
        while (childProcess.redirect_io(childProcess.mStdOut.get_read_handle(), GetStdHandle(STD_OUTPUT_HANDLE), childProcess.mpFnOnStdOut)) {
        }
    } else if (currentThreadId == childProcess.mStdErrThread.get_id()) {
        while (childProcess.redirect_io(childProcess.mStdErr.get_read_handle(), GetStdHandle(STD_ERROR_HANDLE), childProcess.mpFnOnStdErr)) {
        }
    }
    return 0;
}

VOID CALLBACK ChildProcess::on_shutdown(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired)
{
    (void)TimerOrWaitFired;
    assert(lpParameter);
    ((ChildProcess*)lpParameter)->reset();
}

#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk
