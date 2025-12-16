
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

#include "gvk-pipeline-explorer/backend/utilities.hpp"

#include /* spirv_cross/ */ "spirv_common.hpp"

#include "stb/stb_image_write.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <codecvt>
#include <locale>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

#include <map>

namespace gvk {
namespace pipeline_explorer {

UUID get_256_bit_hash(const std::string& str)
{
    // FROM : https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    static const UUID sFnvPrime("0x0000000000000000000001000000000000000000000000000000000000000163");
    static const UUID sFnvOffsetBasis("0xdd268dbcaac550362d98c384c4e576ccc8b1536847b6bbb31023b4c8caee0535");
    auto hash = sFnvOffsetBasis;
    for (char c : str) {
        hash = (hash * sFnvPrime) ^ c;
    }
    return hash;
}

std::string uuid_to_string(const UUID& uuid, uint32_t count)
{
    std::stringstream strStrm;
    strStrm << std::hex << std::showbase << uuid;
    return count ? strStrm.str().substr(0, count) : strStrm.str();
}

std::string uuid_to_string(const uint8_t bytes[GVK_PIPELINE_EXPLORER_UUID_SIZE], uint32_t count)
{
    UUID uuid{ };
    boost::multiprecision::import_bits(uuid, bytes, bytes + GVK_PIPELINE_EXPLORER_UUID_SIZE);
    return uuid_to_string(uuid, count);
}

std::string get_shader_stage_file_extension(VkShaderStageFlagBits shaderStage)
{
    static const std::map<VkShaderStageFlagBits, std::string> scShaderStageFileExtensions{
        { VK_SHADER_STAGE_VERTEX_BIT,                  "vert"  },
        { VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,    "tesc"  },
        { VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tese"  },
        { VK_SHADER_STAGE_GEOMETRY_BIT,                "geom"  },
        { VK_SHADER_STAGE_FRAGMENT_BIT,                "frag"  },
        { VK_SHADER_STAGE_COMPUTE_BIT,                 "comp"  },
        { VK_SHADER_STAGE_RAYGEN_BIT_KHR,              "rgen"  },
        { VK_SHADER_STAGE_ANY_HIT_BIT_KHR,             "rahit" },
        { VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,         "rchit" },
        { VK_SHADER_STAGE_MISS_BIT_KHR,                "rmiss" },
        { VK_SHADER_STAGE_INTERSECTION_BIT_KHR,        "rint"  },
        { VK_SHADER_STAGE_CALLABLE_BIT_KHR,            "rcall" },
        { VK_SHADER_STAGE_TASK_BIT_EXT,                "task"  },
        { VK_SHADER_STAGE_MESH_BIT_EXT,                "mesh"  },
    };
    auto itr = scShaderStageFileExtensions.find(shaderStage);
    return itr != scShaderStageFileExtensions.end() ? itr->second : std::string();
}

std::string get_spirv_base_type_str(const spirv_cross::SPIRType& type)
{
    switch (type.basetype) {
    case spirv_cross::SPIRType::SByte: { return "int8"; } break;
    case spirv_cross::SPIRType::UByte: { return "uint8"; } break;
    case spirv_cross::SPIRType::Short: { return "int16"; } break;
    case spirv_cross::SPIRType::UShort: { return "uint16"; } break;
    case spirv_cross::SPIRType::Int: { return "int"; } break;
    case spirv_cross::SPIRType::UInt: { return "uint"; } break;
    case spirv_cross::SPIRType::Int64: { return "int64"; } break;
    case spirv_cross::SPIRType::UInt64: { return "uint64"; } break;
    case spirv_cross::SPIRType::Half: { return "half"; } break;
    case spirv_cross::SPIRType::Float: { return "float"; } break;
    case spirv_cross::SPIRType::Double: { return "double"; } break;
    default: {
    } break;
    }
    return { };
}

std::string get_spirv_type_str(const spirv_cross::SPIRType& type)
{
    if (1 < type.vecsize) {
        std::string typeStr;
        switch (type.basetype) {
        case spirv_cross::SPIRType::SByte: { return "i8vec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::UByte: { return "u8vec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::Short: { return "i16vec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::UShort: { return "u16vec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::Int: { return "ivec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::UInt: { return "uvec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::Int64: { return "i64vec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::UInt64: { return "u64vec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::Half: { return "hvec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::Float: { return "vec" + std::to_string(type.vecsize); } break;
        case spirv_cross::SPIRType::Double: { return "dvec" + std::to_string(type.vecsize); } break;
        default: {
        } break;
        }
    }
    return get_spirv_base_type_str(type);
}

VkResult read_shader_binding_table(const gvk::Device& gvkDevice, const gvk::Buffer& gvkBuffer, VkDeviceSize stride, VkDeviceSize count, ShaderBindingTable* pShaderBindingTable)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(gvkBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(stride ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(count ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pShaderBindingTable ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        *pShaderBindingTable = { };

        auto physicalDeviceRayTracingProperties = gvk::get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
        auto physicalDeviceProperties2 = gvk::get_default<VkPhysicalDeviceProperties2>();
        physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
        gvkDevice.get<gvk::PhysicalDevice>().GetPhysicalDeviceProperties2(&physicalDeviceProperties2);

        uint8_t* pMappedData = nullptr;
        gvk_result(vmaMapMemory(gvkDevice.get<VmaAllocator>(), gvkBuffer.get<VmaAllocation>(), (void**)&pMappedData));
        auto pItr = pMappedData;

        pShaderBindingTable->stride = stride;
        pShaderBindingTable->entries.resize(count);
        auto shaderBindingTableSize = stride * count;
        auto entryDataSize = stride - physicalDeviceRayTracingProperties.shaderGroupHandleSize;
        for (auto& entry : pShaderBindingTable->entries) {
            gvk_result((VkDeviceSize)(pItr - pMappedData) <= shaderBindingTableSize ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            entry.shaderGroupHandle.insert(entry.shaderGroupHandle.end(), pItr, pItr + physicalDeviceRayTracingProperties.shaderGroupHandleSize);
            pItr += physicalDeviceRayTracingProperties.shaderGroupHandleSize;
            gvk_result((VkDeviceSize)(pItr - pMappedData) <= shaderBindingTableSize ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            entry.data.insert(entry.data.end(), pItr, pItr + entryDataSize);
            pItr += entryDataSize;
        }
        gvk_result((VkDeviceSize)(pItr - pMappedData) == shaderBindingTableSize ? VK_SUCCESS : VK_ERROR_UNKNOWN);

        vmaUnmapMemory(gvkDevice.get<VmaAllocator>(), gvkBuffer.get<VmaAllocation>());
    } gvk_result_scope_end;
    return gvkResult;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
BOOL get_this_module_handle(HMODULE* phModule)
{
    assert(phModule);
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&get_this_module_handle, phModule);
}

DWORD get_module_path(HMODULE hModule, std::filesystem::path* pPath)
{
    assert(pPath);
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

BOOL CALLBACK enumerate_windows_processes(HWND hwnd, LPARAM lParam)
{
    DWORD windowPID = 0;
    GetWindowThreadProcessId(hwnd, &windowPID);

    ProcessWindow* pData = reinterpret_cast<ProcessWindow*>(lParam);
    if (windowPID == pData->processId && IsWindowVisible(hwnd)) {
        pData->hWnd = hwnd;
        return FALSE; // Stop enumeration after finding the first match
    }
    return TRUE;
}

HWND find_window_by_pid(DWORD pid)
{
    ProcessWindow data = { pid, nullptr };
    EnumWindows(enumerate_windows_processes, reinterpret_cast<LPARAM>(&data));
    return data.hWnd;
}

//Will return pixels in BGRA format
ImageData capture_window_pixels(HWND hwnd)
{
    HBITMAP hBitmap = HBITMAP(0);
    HDC hMemDC = HDC(0);
    HDC hWindowDC = HDC(0);
    std::vector<unsigned char> pixels = { 0 };
    int width = 0;
    int height = 0;

    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(hwnd && "invalid hwnd given to capture_window_pixels");

        RECT rc = { 0,0,0,0 };
        GetClientRect(hwnd, &rc);
        width = rc.right - rc.left;
        height = rc.bottom - rc.top;

        hWindowDC = GetDC(hwnd);
        hMemDC = CreateCompatibleDC(hWindowDC);

        // Create a 32-bit bitmap (BGRA). We specify this so we can convert it easily into a pixel array after
        BITMAPINFO bmi = { 0,0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // Negative for top-down bitmap
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32; // 4 bytes per pixel
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pPixels = nullptr;

        hBitmap = CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, &pPixels, nullptr, 0);
        gvk_result_assert(hBitmap && "bitmap window capture unsuccesfull"); //we should configure this bitmap gvk_result_assert to not assert on failure here

        SelectObject(hMemDC, hBitmap);

        // Copy window content into the bitmap
        gvk_result_assert(BitBlt(hMemDC, 0, 0, width, height, hWindowDC, 0, 0, SRCCOPY) && "BitBlt failed");

        // Copy pixels from DIB section into a vector
        size_t dataSize = width * height * 4; // 4 bytes per pixel BGRA
        pixels.resize(dataSize);
        auto pBytes = static_cast<unsigned char*>(pPixels);
        for (int i = 0; i < width * height; ++i) {
            pixels[i * 4 + 0] = pBytes[i * 4 + 2]; //B -> R
            pixels[i * 4 + 1] = pBytes[i * 4 + 1]; //G
            pixels[i * 4 + 2] = pBytes[i * 4 + 0]; //R -> B
            pixels[i * 4 + 3] = pBytes[i * 4 + 3]; //A
        }

    } gvk_result_scope_end;
        
    if (gvkResult != VK_SUCCESS) {
        pixels.clear();
        height = 0;
        width = 0;
    }

    // Cleanup and return empty struct
    if (hBitmap) {
        DeleteObject(hBitmap);
    }
    if (hMemDC) {
        DeleteDC(hMemDC);
    }
    if (hWindowDC) {
        ReleaseDC(hwnd, hWindowDC);
    }

    return { std::move(pixels), width, height };
}

bool save_pixels_to_png(const ImageData &img, std::string filename)
{
    //Create screens directory
    return stbi_write_png(filename.c_str(), img.width, img.height, 4, img.pixels.data(), img.width * 4);
}

std::string get_window_title(HWND hwnd)
{
    std::string title(GetWindowTextLength(hwnd) + 1, '\0');
    title.resize(GetWindowText(hwnd, title.data(), static_cast<int>(title.size())));
    return title;
}

#endif // VK_USE_PLATFORM_WIN32_KHR

} // namespace pipeline_explorer
} // namespace gvk
