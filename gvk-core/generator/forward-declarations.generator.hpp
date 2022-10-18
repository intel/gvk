
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

#include "cppgen-utilities.hpp"

namespace gvk {
namespace cppgen {

class ForwardDeclarationsGenerator final
{
public:
    static void generate(const gvk::xml::Manifest& manifest)
    {
        File file("forward-declarations.hpp");
        file << std::endl;
        NamespaceGenerator gvkNamespaceGenerator(file, "gvk");
        file << std::endl;
        for (const auto& handleItr : manifest.handles) {
            CompileGuardGenerator compileGuardGenerator(file, handleItr.second.compileGuards);
            file << "class " << gvk::string::strip_vk(handleItr.second.name) << ";" << std::endl;
        }
        file << std::endl;
        NamespaceGenerator detailNamespaceGenerator(file, "detail");
        file << std::endl;
        for (const auto& handleItr : manifest.handles) {
            CompileGuardGenerator compileGuardGenerator(file, handleItr.second.compileGuards);
            file << "class " << gvk::string::strip_vk(handleItr.second.name) << "ControlBlock;" << std::endl;
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
