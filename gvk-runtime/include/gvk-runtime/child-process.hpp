
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

#include "gvk-defines.hpp"
#include "gvk-runtime/io-pipe.hpp"
#include "gvk-runtime/io-thread.hpp"

#include <string>

namespace gvk {

#ifdef GVK_PLATFORM_WINDOWS

class ChildProcess final
{
public:
    struct CreateInfo
    {
        LPSTR pCmdLine{ };
        LPCSTR pDirectory{ };
        LPVOID pEnvironment{ };
        BOOL unicodeConversionEnabled{ TRUE };
        void(*pFnOnStdOut)(const ChildProcess& childProcess, size_t dataSize, const char* pData);
        void(*pFnOnStdErr)(const ChildProcess& childProcess, size_t dataSize, const char* pData);
        void(*pFnOnShutdown)(const ChildProcess& childProcess){ };
        void* pUserData{ };
    };

    ChildProcess() = default;
    ChildProcess(ChildProcess&& other) noexcept;
    ChildProcess& operator=(ChildProcess&& other) noexcept;
    ~ChildProcess();
    operator bool() const;
    static BOOL create(const CreateInfo* pCreateInfo, ChildProcess* pChildProcess);
    void reset();
    DWORD get_pid() const;
    const std::string& get_cmd_line() const;
    void* get_user_data() const;
    bool running() const;

private:
    BOOL redirect_io(HANDLE read, HANDLE write, void(*pFnOnIo)(const ChildProcess& childProcess, size_t dataSize, const char* pData)) const;
    static DWORD CALLBACK on_io(_In_ LPVOID lpParameter);
    static VOID CALLBACK on_shutdown(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired);

    std::string mCmdLine;
    BOOL mUnicodeConversionEnabled{ TRUE };
    PROCESS_INFORMATION mProcessInfo{ };
    HANDLE mShutdownEvent{ };
    HANDLE mShutdownJob{ };
    gvk::IoPipe mStdIn;
    gvk::IoPipe mStdOut;
    gvk::IoPipe mStdErr;
    gvk::IoThread mStdInThread;
    gvk::IoThread mStdOutThread;
    gvk::IoThread mStdErrThread;
    void(*mpFnOnStdOut)(const ChildProcess& childProcess, size_t dataSize, const char* pData) { };
    void(*mpFnOnStdErr)(const ChildProcess& childProcess, size_t dataSize, const char* pData) { };
    void(*mpFnOnShutdown)(const ChildProcess& childProcess) { };
    void* mpUserData{ };

    ChildProcess(const ChildProcess&) = delete;
    ChildProcess& operator=(const ChildProcess&) = delete;
};

#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk
