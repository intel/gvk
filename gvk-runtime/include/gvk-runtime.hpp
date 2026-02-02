
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

#include <filesystem>
#include <string>
#include <vector>

namespace gvk {

VkResult load_vulkan_runtime();
void unload_vulkan_runtime();
PFN_vkGetInstanceProcAddr load_vkGetInstanceProcAddr();
const std::vector<std::string>& get_validation_layer_settings();

#ifdef GVK_PLATFORM_WINDOWS

class IoPipe final
{
public:
    class Message;
    class Writer;
    class Reader;

    enum FlagBits
    {
        Read  = 1 << 0,
        Write = 1 << 1,
    };

    using Flags = DWORD;

    struct CreateInfo final
    {
        Flags inherit{ };
    };

    IoPipe() = default;
    IoPipe(IoPipe&& other) noexcept;
    IoPipe& operator=(IoPipe&& other) noexcept;
    ~IoPipe();
    operator bool() const;
    static BOOL create(const CreateInfo* pCreateInfo, IoPipe* pIoPipe);
    void reset();
    HANDLE get_read_handle() const;
    HANDLE get_write_handle() const;
    void close(Flags flags = Read | Write);

private:
    HANDLE mReadHandle{ };
    HANDLE mWriteHandle{ };

    IoPipe(const IoPipe&) = delete;
    IoPipe& operator=(const IoPipe&) = delete;
};

class IoThread final
{
public:
    struct CreateInfo final
    {
        LPTHREAD_START_ROUTINE lpThreadProc{ };
        LPVOID lpParameter{ };
    };

    IoThread() = default;
    IoThread(IoThread&& other) noexcept;
    IoThread& operator=(IoThread&& other) noexcept;
    ~IoThread();
    operator bool() const;
    static BOOL create(const CreateInfo* pCreateInfo, IoThread* pIoThread);
    void reset();
    HANDLE get_handle() const;
    DWORD get_id() const;

private:
    HANDLE mHandle{ };
    DWORD mId{ };

    IoThread(const IoThread&) = delete;
    IoThread& operator=(const IoThread&) = delete;
};

class ChildProcess final
{
public:
    struct CreateInfo
    {
        LPSTR pCmdLine{ };
        LPCSTR pDirectory{ };
        LPVOID pEnvironment{ };
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
    const std::string& get_cmd_line() const;
    void* get_user_data() const;
    bool running() const;

private:
    BOOL redirect_io(HANDLE read, HANDLE write, void(*pFnOnIo)(const ChildProcess& childProcess, size_t dataSize, const char* pData)) const;
    static DWORD CALLBACK on_io(_In_ LPVOID lpParameter);
    static VOID CALLBACK on_shutdown(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired);

    std::string mCmdLine;
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

BOOL get_this_module_handle(HMODULE* phModule);
DWORD get_module_path(HMODULE hModule, std::filesystem::path* pPath);
DWORD get_this_module_path(std::filesystem::path* pPath);
std::string get_win32_error_str(DWORD errorCode);

#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk
