
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

#include "gvk-containers/streambuf.hpp"
#include "gvk-runtime/ipc-messenger.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-structures.hpp"

#include <mutex>
#include <type_traits>

namespace gvk {
namespace pipeline_explorer {

class IpcMessenger final
    : public gvk::IpcMessenger
{
public:
    ~IpcMessenger();
    void reset() override final;
    HANDLE get_read_pipe() const;
    void set_read_pipe(HANDLE hPipe);
    HANDLE get_write_pipe() const;
    void set_write_pipe(HANDLE hPipe);
    std::vector<Message> read();
    void write(const char* pText, uint32_t dataSize = 0, const uint8_t* pData = nullptr);

    template <typename StructureType>
    void write(const std::string& text, const StructureType& structure)
    {
        if constexpr (std::is_same_v<StructureType, std::string>) {
            write(text.c_str(), (uint32_t)structure.size(), (const uint8_t*)structure.data());
        } else {
            thread_local gvk::VectorOstreambuf tlSerializer;
            tlSerializer.clear();
            std::ostream ostrm(&tlSerializer);
            gvk::serialize(ostrm, structure);
            write(text.c_str(), (uint32_t)tlSerializer.size(), tlSerializer.data());
        }
    }

private:
    std::mutex mMutex;
    HANDLE mhReadPipe{ };
    HANDLE mhWritePipe{ };
};

template <typename StructureType>
inline void create_ipc_message(const std::string& text, const StructureType& structure, IpcMessenger::Message* pMessage)
{
    // TODO : DRY
    if (pMessage) {
        gvk::VectorOstreambuf serializer;
        std::ostream ostrm(&serializer);
        gvk::serialize(ostrm, structure);
        pMessage->text = text;
        pMessage->data.insert(pMessage->data.end(), serializer.data(), serializer.data() + serializer.size());
    }
}

} // namespace pipeline_explorer
} // namespace gvk
