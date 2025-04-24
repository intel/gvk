
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

#include "gvk-structures/generated/core-structure-deserialization.hpp"
#include "gvk-structures/generated/core-structure-get-stype.hpp"
#include "gvk-structures/generated/core-structure-serialization.hpp"
#include "gvk-structures/generated/core-enumerations-to-string.hpp"
#include "gvk-defines.hpp"
#include "gvk-string.hpp"

#include <filesystem>
#include <fstream>

namespace gvk {

/*
TODO : Documentation
*/
template <typename StructureType>
inline const std::string& get_default_serialized_structure_file_name()
{
    static constexpr gvk::Printer::Flags scPrinterFlags{ gvk::Printer::Default & ~gvk::Printer::EnumValue };
    static const auto scFileName = gvk::string::remove(gvk::to_string(gvk::get_stype<StructureType>(), scPrinterFlags), "\"");
    return scFileName;
}

/*
TODO : Documentation
*/
template <typename StructureType>
inline VkResult write_serialized_structure(const std::filesystem::path& path, const std::string& name, const StructureType& obj, bool forceOverwrite = false)
{
    std::filesystem::create_directories(path);
    auto infoFilePath = std::filesystem::path(path / name).replace_extension(".info");
    std::error_code errorCode;
    if (!std::filesystem::exists(infoFilePath, errorCode) || forceOverwrite) {
        if (!errorCode) {
            auto tempFilePath = std::filesystem::path(path / name).replace_extension(".temp");
            std::ofstream tempFile(tempFilePath, std::ios::binary);
            if (tempFile.is_open()) {
                gvk::serialize(tempFile, obj);
                tempFile.close();
                std::filesystem::rename(tempFilePath, infoFilePath);
                return VK_SUCCESS;
            } else {
                return VK_INCOMPLETE;
            }
        } else {
            return VK_ERROR_UNKNOWN;
        }
    }
    return VK_NOT_READY;
}

/*
TODO : Documentation
*/
template <typename StructureType>
inline VkResult write_serialized_structure(const std::filesystem::path& path, const StructureType& obj, bool forceOverwrite = false)
{
    return write_serialized_structure<StructureType>(path, get_default_serialized_structure_file_name<StructureType>(), obj, forceOverwrite);
}

/*
TODO : Documentation
*/
template <typename StructureType>
inline VkResult read_serialized_structure(const std::filesystem::path& path, const std::string& name, Auto<StructureType>& obj, bool removeFileOnRead = true)
{
    obj.reset();
    auto infoFilePath = std::filesystem::path(path / name).replace_extension(".info");
    if (std::filesystem::exists(infoFilePath)) {
        std::ifstream infoFile(infoFilePath, std::ios::binary);
        if (infoFile.is_open()) {
            gvk::deserialize(infoFile, nullptr, obj);
            infoFile.close();
            if (removeFileOnRead) {
                std::filesystem::remove(infoFilePath);
            }
            return VK_SUCCESS;
        } else {
            return VK_INCOMPLETE;
        }
    }
    return VK_NOT_READY;
}

/*
TODO : Documentation
*/
template <typename StructureType>
inline VkResult read_serialized_structure(const std::filesystem::path& path, Auto<StructureType>& obj, bool removeFileOnRead = true)
{
    return read_serialized_structure<StructureType>(path, get_default_serialized_structure_file_name<StructureType>(), obj, removeFileOnRead);
}

} // namespace gvk
