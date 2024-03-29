
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

#include "gvk-cppgen/file-generator.hpp"

#include <cassert>
#include <fstream>

namespace gvk {
namespace cppgen {

FileGenerator::FileGenerator(const std::filesystem::path& filePath, const std::string& licenseHeader)
    : mFilePath { filePath }
{
    *this << licenseHeader << "\n// NOTE : This file contains generated code\n";
    if (mFilePath.extension() == ".hpp") {
        *this << "\n#pragma once\n\n";
    }
}

FileGenerator::~FileGenerator()
{
    assert(!mFilePath.empty());
    std::filesystem::create_directories(mFilePath.parent_path());
    auto content = str();
    std::ifstream file(mFilePath);
    if (content != std::string(std::istreambuf_iterator<char>(file), { })) {
        file.close();
        std::ofstream(mFilePath) << content;
    }
}

} // namespace cppgen
} // namespace gvk
