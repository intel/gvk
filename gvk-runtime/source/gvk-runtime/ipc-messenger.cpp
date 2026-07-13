
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

#include "gvk-runtime/ipc-messenger.hpp"
#include "gvk-runtime.hpp"
#include "gvk-string.hpp"

namespace gvk {

IpcMessenger::~IpcMessenger()
{
    reset();
}

void IpcMessenger::reset()
{
    mWriteBuffer.clear();
}

std::vector<IpcMessenger::Message> IpcMessenger::read(HANDLE ipcReadHandle)
{
    (void)ipcReadHandle;

#ifdef GVK_PLATFORM_WINDOWS

    assert(ipcReadHandle);

    // Peek the pipe to see if there's a message ready
    DWORD bytesAvailable = 0;
    if (PeekNamedPipe(ipcReadHandle, NULL, 0, NULL, &bytesAvailable, NULL)) {
        std::vector<Message> messages;
        while (bytesAvailable) {
            Message message;

            // Read the message header
            DWORD bytesRead = 0;
            while (message.header.empty() || message.header.back() != ']') {
                message.header += '0';
                if (!ReadFile(ipcReadHandle, &message.header.back(), 1, &bytesRead, NULL) || bytesRead != 1) {
                    std::cerr << "Failed to read to pipe : " << gvk::get_win32_error_str(GetLastError()) << std::endl;
                }
                bytesAvailable -= bytesRead;
            }

            // Parse the header for the text and data portion sizes
            auto headerSizes = gvk::string::split(gvk::string::remove(gvk::string::remove(message.header, "["), "]"), ":");
            assert(headerSizes.size() == 2);
            message.text.resize(gvk::string::to_number<size_t>(headerSizes[0]));
            message.data.resize(gvk::string::to_number<size_t>(headerSizes[1]));

            // Read the text portion
            DWORD textBytesRead = 0;
            while (bytesAvailable && textBytesRead < message.text.size()) {
                if (!ReadFile(ipcReadHandle, message.text.data(), std::min((DWORD)message.text.size(), bytesAvailable), &bytesRead, NULL)) {
                    std::cerr << "Failed to read to pipe : " << gvk::get_win32_error_str(GetLastError()) << std::endl;
                }
                bytesAvailable -= bytesRead;
                textBytesRead += bytesRead;
            }

            // Read the data portion
            DWORD dataBytesRead = 0;
            while (bytesAvailable && dataBytesRead < message.data.size()) {
                if (!ReadFile(ipcReadHandle, message.data.data(), std::min((DWORD)message.data.size(), bytesAvailable), &bytesRead, NULL)) {
                    std::cerr << "Failed to read to pipe : " << gvk::get_win32_error_str(GetLastError()) << std::endl;
                }
                bytesAvailable -= bytesRead;
                dataBytesRead += bytesRead;
            }

            // Add to the received message collection
            messages.push_back(std::move(message));
        }
        return messages;
    }

#endif // GVK_PLATFORM_WINDOWS

    return { };
}

void IpcMessenger::write(HANDLE ipcWriteHandle, const char* pText, uint32_t dataSize, const uint8_t* pData)
{
    (void)ipcWriteHandle;
    (void)pText;
    (void)dataSize;
    (void)pData;

    assert(ipcWriteHandle);

#ifdef GVK_PLATFORM_WINDOWS

    // Messages are composed of a header, text, and data.  The header is composed of
    //  the size in bytes of the text and data portions (either of which may be zero).
    //  For example...
    //
    //      [64:128]
    //
    //  ...indicates a message with 64 byte text portion and 128 byte data portion.

    // Populate message
    mWriteBuffer.clear();
    auto textSize = pText ? strlen(pText) : 0;
    auto header = "[" + std::to_string(textSize) + ":" + std::to_string(dataSize) + "]";
    mWriteBuffer.insert(mWriteBuffer.end(), header.begin(), header.end());
    if (textSize) {
        mWriteBuffer.insert(mWriteBuffer.end(), pText, pText + textSize);
    }
    if (dataSize && pData) {
        mWriteBuffer.insert(mWriteBuffer.end(), pData, pData + dataSize);
    }

    // Write message to pipe
    DWORD totalBytesWritten = 0;
    while (totalBytesWritten < mWriteBuffer.size()) {
        DWORD bytesWritten{ };
        if (!WriteFile(ipcWriteHandle, mWriteBuffer.data() + totalBytesWritten, (DWORD)mWriteBuffer.size() - totalBytesWritten, &bytesWritten, NULL)) {
            std::cerr << "Failed to write to pipe : " << gvk::get_win32_error_str(GetLastError()) << std::endl;
            break;
        }
        totalBytesWritten += bytesWritten;
    }

#endif // GVK_PLATFORM_WINDOWS

}

} // namespace gvk
