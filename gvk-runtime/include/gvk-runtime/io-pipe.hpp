
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

namespace gvk {

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

class NamedPipe final
{
public:
    struct ServerCreateInfo final
    {
        const char* pName{ };
    };

    struct ClientCreateInfo final
    {
        const char* pName{ };
        DWORD timeoutMs{ 5000 };
    };

    NamedPipe() = default;
    NamedPipe(NamedPipe&& other) noexcept;
    NamedPipe& operator=(NamedPipe&& other) noexcept;
    ~NamedPipe();
    operator bool() const;

    static BOOL create_server(const ServerCreateInfo* pCreateInfo, NamedPipe* pNamedPipe);
    static BOOL connect_client(const ClientCreateInfo* pCreateInfo, NamedPipe* pNamedPipe);

    // Server: start async wait for the next client connection
    BOOL begin_accept();
    // Server: non-blocking check — returns true once a client has connected
    bool is_accepted();
    // Server: disconnect the current client and reset for the next begin_accept()
    void disconnect();

    HANDLE get_handle() const;
    // Transfer ownership of the handle to the caller; NamedPipe no longer owns or closes it
    HANDLE release();
    void reset();

private:
    HANDLE mHandle{ NULL };
    HANDLE mEvent{ NULL };
    OVERLAPPED mOverlapped{ };
    bool mPendingAccept{ false };

    NamedPipe(const NamedPipe&) = delete;
    NamedPipe& operator=(const NamedPipe&) = delete;
};

#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk
