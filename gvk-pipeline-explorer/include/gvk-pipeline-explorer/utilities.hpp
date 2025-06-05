
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
#include "gvk-pipeline-explorer.hpp"
#include "gvk-handles.hpp"

#include "boost/multiprecision/integer.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace spirv_cross {

struct SPIRType;

} // namespace spirv_cross

namespace gvk {
namespace pipeline_explorer {

static constexpr gvk::Printer::Flags PrinterFlags{ gvk::Printer::Default & ~gvk::Printer::EnumValue };

using UUID = boost::multiprecision::uint256_t;
UUID get_256_bit_hash(const std::string& str);
std::string uuid_to_string(const UUID& uuid, uint32_t count = 0);
std::string uuid_to_string(const uint8_t bytes[GVK_PIPELINE_EXPLORER_UUID_SIZE], uint32_t count = 0);

std::string get_shader_stage_file_extension(VkShaderStageFlagBits shaderStage);
std::string get_spirv_base_type_str(const spirv_cross::SPIRType& type);
std::string get_spirv_type_str(const spirv_cross::SPIRType& type);

class ShaderBindingTable final
{
public:
    class Entry final
    {
    public:
        std::vector<uint8_t> shaderGroupHandle;
        std::vector<uint8_t> data;
    };

    std::vector<Entry> entries;
    VkDeviceSize stride{ };
};

VkResult read_shader_binding_table(const gvk::Device& gvkDevice, const gvk::Buffer& gvkBuffer, VkDeviceSize stride, VkDeviceSize count, ShaderBindingTable* pShaderBindingTable);

#ifdef VK_USE_PLATFORM_WIN32_KHR
BOOL get_this_module_handle(HMODULE* phModule);
DWORD get_module_path(HMODULE hModule, std::filesystem::path* pPath);
DWORD get_this_module_path(std::filesystem::path* pPath);
std::string get_win32_error_str(DWORD errorCode);
#endif // VK_USE_PLATFORM_WIN32_KHR

} // namespace pipeline_explorer
} // namespace gvk
