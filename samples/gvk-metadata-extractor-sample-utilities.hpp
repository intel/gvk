
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

#include "gvk-runtime.hpp"
#include "gvk-binding-info.hpp"
#include "gvk-containers/streambuf.hpp"
#include "gvk-string.hpp"
#include "gvk-structures.hpp"

#include <Windows.h>

#include <iostream>
#include <queue>
#include <string>
#include <vector>

// NOTE : The IPC message setup here aims to be "good enough" for this sample.
//  For a "real" application, a more robust IPC strategy is recommended.

class GvkMetadataExtractorSampleIpcMessage final
{
public:
    std::string header;
    std::string text;
    std::vector<uint8_t> data;
};

class GvkMetadataExtractorSampleIpcMessenger final
{
public:
    template <typename StructureType>
    void write(HANDLE ipcWriteHandle, const std::string& text, const StructureType& structure)
    {
        mSerializer.clear();
        std::ostream ostrm(&mSerializer);
        gvk::serialize(ostrm, structure);
        write(ipcWriteHandle, text, (uint32_t)mSerializer.size(), mSerializer.data());
    }

    void write(HANDLE ipcWriteHandle, const std::string& text, uint32_t dataSize = 0, const uint8_t* pData = nullptr)
    {
        assert(ipcWriteHandle);
        assert(!text.empty());

        // Messages are composed of a header, text, and data.  The header is composed of
        //  the size in bytes of the text and data portions (either of which may be zero).
        //  For example...
        //
        //      [64:128]
        // 
        //  ...indicates a message with a text portion of 64 bytes and a data portion of
        //  128 bytes.

        // Populate message
        mWriteBuffer.clear();
        auto header = "[" + std::to_string(text.size()) + ":" + std::to_string(dataSize) + "]";
        mWriteBuffer.insert(mWriteBuffer.end(), header.begin(), header.end());
        if (!text.empty()) {
            mWriteBuffer.insert(mWriteBuffer.end(), text.begin(), text.end());
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
            }
            totalBytesWritten += bytesWritten;
        }
    }

    std::vector<GvkMetadataExtractorSampleIpcMessage> read(HANDLE ipcReadHandle)
    {
        // TODO : Shutting down app can trigger this
        assert(ipcReadHandle);

        // Peek the pipe to see if there's a message ready
        DWORD bytesAvailable = 0;
        if (PeekNamedPipe(ipcReadHandle, NULL, 0, NULL, &bytesAvailable, NULL)) {
            std::vector<GvkMetadataExtractorSampleIpcMessage> messages;
            while (bytesAvailable) {
                GvkMetadataExtractorSampleIpcMessage message;

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

                // Add to the recieved message collection
                messages.push_back(std::move(message));
            }
            return messages;
        }
        return { };
    }

private:
    std::vector<uint8_t> mWriteBuffer;
    gvk::VectorOstreambuf mSerializer;
};
