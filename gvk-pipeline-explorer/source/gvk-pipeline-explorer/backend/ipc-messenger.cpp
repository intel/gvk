
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

#include "gvk-pipeline-explorer/backend/ipc-messenger.hpp"

#ifdef GVK_PLATFORM_WINDOWS

namespace gvk {
namespace pipeline_explorer {

IpcMessenger::~IpcMessenger()
{
}

HANDLE IpcMessenger::get_read_pipe() const
{
    return mhReadPipe;
}

void IpcMessenger::set_read_pipe(HANDLE hPipe)
{
    mhReadPipe = hPipe;
}

HANDLE IpcMessenger::get_write_pipe() const
{
    return mhWritePipe;
}

void IpcMessenger::set_write_pipe(HANDLE hPipe)
{
    mhWritePipe = hPipe;
}

void IpcMessenger::reset()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mhReadPipe = NULL;
    mhWritePipe = NULL;
    gvk::IpcMessenger::reset();
}

std::vector<gvk::IpcMessenger::Message> IpcMessenger::read()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mhReadPipe ? gvk::IpcMessenger::read(mhReadPipe) : std::vector<gvk::IpcMessenger::Message>{ };
}

void IpcMessenger::write(const char* pText, uint32_t dataSize, const uint8_t* pData)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mhWritePipe) {
        gvk::IpcMessenger::write(mhWritePipe, pText, dataSize, pData);
    }
}

} // namespace pipeline_explorer
} // namespace gvk

#endif // GVK_PLATFORM_WINDOWS
