
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

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
