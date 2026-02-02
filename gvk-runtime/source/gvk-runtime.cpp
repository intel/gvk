
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

#include "gvk-runtime.hpp"

#include <array>

namespace gvk {

static void* sVulkanRuntime;

VkResult load_vulkan_runtime()
{
#ifdef GVK_PLATFORM_LINUX
    if (!sVulkanRuntime) {
        sVulkanRuntime = gvk_dlopen("libvulkan.so.1");
    }
    if (!sVulkanRuntime) {
        sVulkanRuntime = gvk_dlopen("libvulkan.so");
    }
#endif
#ifdef GVK_PLATFORM_WINDOWS
    if (!sVulkanRuntime) {
        sVulkanRuntime = gvk_dlopen("vulkan-1.dll");
    }
#endif
    return sVulkanRuntime ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT;
}

void unload_vulkan_runtime()
{
    if (sVulkanRuntime) {
        gvk_dlclose(sVulkanRuntime);
        sVulkanRuntime = NULL;
    }
}

PFN_vkGetInstanceProcAddr load_vkGetInstanceProcAddr()
{
    return (load_vulkan_runtime() == VK_SUCCESS) ? (PFN_vkGetInstanceProcAddr)gvk_dlsym(sVulkanRuntime, "vkGetInstanceProcAddr") : nullptr;
}

const std::vector<std::string>& get_validation_layer_settings()
{
    // TODO : Settings should be parsed from layer JSON for all layers

    // FROM : https://vulkan.lunarg.com/doc/view/1.4.304.0/windows/khronos_validation_layer.html
    static const std::vector<std::string> scLayerSettings{
        /*
        Name                                                                  Type        Default Value
        */
        "VK_LAYER_FINE_GRAINED_LOCKING",                                   // BOOL      : true
        "VK_KHRONOS_VALIDATION_VALIDATE_CORE",                             // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_IMAGE_LAYOUT",                        // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_COMMAND_BUFFER",                      // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_OBJECT_IN_USE",                       // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_QUERY",                               // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_SHADERS",                             // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_SHADERS_CACHING",                     // BOOL      : true
        "VK_KHRONOS_VALIDATION_DEBUG_DISABLE_SPIRV_VAL",                   // BOOL      : false
        "VK_KHRONOS_VALIDATION_UNIQUE_HANDLES",                            // BOOL      : true
        "VK_KHRONOS_VALIDATION_OBJECT_LIFETIME",                           // BOOL      : true
        "VK_KHRONOS_VALIDATION_STATELESS_PARAM",                           // BOOL      : true
        "VK_KHRONOS_VALIDATION_THREAD_SAFETY",                             // BOOL      : true
        "VK_KHRONOS_VALIDATION_VALIDATE_SYNC",                             // BOOL      : false
        "VK_KHRONOS_VALIDATION_SYNCVAL_SUBMIT_TIME_VALIDATION",            // BOOL      : true
        "VK_KHRONOS_VALIDATION_SYNCVAL_SHADER_ACCESSES_HEURISTIC",         // BOOL      : false
        "VK_KHRONOS_VALIDATION_SYNCVAL_MESSAGE_EXTRA_PROPERTIES",          // BOOL      : false
        "VK_KHRONOS_VALIDATION_PRINTF_ENABLE",                             // BOOL      : false
        "VK_KHRONOS_VALIDATION_PRINTF_TO_STDOUT",                          // BOOL      : true
        "VK_KHRONOS_VALIDATION_PRINTF_VERBOSE",                            // BOOL      : false
        // "VK_KHRONOS_VALIDATION_PRINTF_BUFFER_SIZE",                     // INT       : 1024
        "VK_KHRONOS_VALIDATION_GPUAV_ENABLE",                              // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_SHADER_INSTRUMENTATION",              // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_DESCRIPTOR_CHECKS",                   // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_WARN_ON_ROBUST_OOB",                  // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_BUFFER_ADDRESS_OOB",                  // BOOL      : true
        // "VK_KHRONOS_VALIDATION_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES",      // INT       : 10000
        "VK_KHRONOS_VALIDATION_GPUAV_VALIDATE_RAY_QUERY",                  // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_POST_PROCESS_DESCRIPTOR_INDEXING",    // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_SELECT_INSTRUMENTED_SHADERS",         // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_BUFFERS_VALIDATION",                  // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_INDIRECT_DRAWS_BUFFERS",              // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_INDIRECT_DISPATCHES_BUFFERS",         // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_INDIRECT_TRACE_RAYS_BUFFERS",         // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_BUFFER_COPIES",                       // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_INDEX_BUFFERS",                       // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_RESERVE_BINDING_SLOT",                // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_VMA_LINEAR_OUTPUT",                   // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_DEBUG_VALIDATE_INSTRUMENTED_SHADERS", // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_DEBUG_DUMP_INSTRUMENTED_SHADERS",     // BOOL      : false
        // "VK_KHRONOS_VALIDATION_GPUAV_DEBUG_MAX_INSTRUMENTATIONS_COUNT", // INT       : 0
        "VK_KHRONOS_VALIDATION_GPUAV_DEBUG_PRINT_INSTRUMENTATION_INFO",    // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES",                   // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_ARM",               // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_AMD",               // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_IMG",               // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_NVIDIA",            // BOOL      : false
        // "VK_KHRONOS_VALIDATION_DEBUG_ACTION",                           // FLAGS     : VK_DBG_LAYER_ACTION_LOG_MSG
        // "VK_KHRONOS_VALIDATION_LOG_FILENAME",                           // SAVE_FILE : stdout
        // "VK_KHRONOS_VALIDATION_REPORT_FLAGS",                           // FLAGS     : error
        "VK_KHRONOS_VALIDATION_ENABLE_MESSAGE_LIMIT",                      // BOOL      : true
        // "VK_LAYER_DUPLICATE_MESSAGE_LIMIT",                             // INT       : 10
        // "VK_LAYER_MESSAGE_ID_FILTER",                                   // LIST      :
        "VK_KHRONOS_VALIDATION_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME",   // BOOL      : false
    };
    return scLayerSettings;
}

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

ChildProcess::ChildProcess(ChildProcess&& other) noexcept
{
    *this = std::move(other);
}

ChildProcess& ChildProcess::operator=(ChildProcess&& other) noexcept
{
    if (this != &other) {
        reset();
        mCmdLine = std::exchange(other.mCmdLine, { });
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

    // Close IO pipes
    mStdIn.reset();
    mStdOut.reset();
    mStdErr.reset();

    // Close IO threads
    mStdInThread.reset();
    mStdOutThread.reset();
    mStdErrThread.reset();

    // Clear cmd line and user data
    mCmdLine.clear();
    mpUserData = nullptr;
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
    DWORD count = 0;
    std::array<char, 1024> buffer{ };
    auto success = ReadFile(read, buffer.data(), (DWORD)buffer.size() - 1, &count, NULL);
    if (success && count) {
        if (pFnOnIo) {
            pFnOnIo(*this, count, buffer.data());
        } else {
            success = WriteFile(write, buffer.data(), count, NULL, NULL);
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

BOOL get_this_module_handle(HMODULE* phModule)
{
    gvk_assert(phModule);
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&get_this_module_handle, phModule);
}

DWORD get_module_path(HMODULE hModule, std::filesystem::path* pPath)
{
    gvk_assert(pPath);
    const size_t CharBufferSize = 16384;
    std::vector<wchar_t> wcharBuffer(CharBufferSize);
    auto result = GetModuleFileNameW(hModule, wcharBuffer.data(), (DWORD)wcharBuffer.size());
    if (result && wcharBuffer[0]) {
        *pPath = wcharBuffer.data();
    }
    return result;
}

DWORD get_this_module_path(std::filesystem::path* pPath)
{
    HMODULE hModule = NULL;
    return get_this_module_handle(&hModule) ? get_module_path(hModule, pPath) : 0;
}

std::string get_win32_error_str(DWORD errorCode)
{
    std::string errorStr = "Win32 [" + std::to_string(errorCode) + "]";
    LPSTR pErrorStr = NULL;
    auto dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    if (FormatMessage(dwFlags, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&pErrorStr, 0, NULL)) {
        errorStr += " " + std::string(pErrorStr);
    }
    LocalFree(pErrorStr);
    return errorStr;
}
#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk
