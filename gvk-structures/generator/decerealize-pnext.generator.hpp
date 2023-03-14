
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

#include "gvk-cppgen.hpp"

namespace gvk {
namespace cppgen {

class DecerealizePNextGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_STRUCTURES_GENERATED_INCLUDE_PATH "/decerealize-pnext.hpp");
        file << std::endl;
        file << "#include \"gvk-structures/detail/cerealization-manual.hpp\"" << std::endl;
        file << "#include \"gvk-structures/detail/cerealization-utilities.hpp\"" << std::endl;
        file << "#include \"gvk-structures/generated/core-structure-decerealization.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;
        file << "template <typename ArchiveType>" << std::endl;
        file << "inline void* decerealize_pnext(ArchiveType& archive)" << std::endl;
        file << "{" << std::endl;
        file << "    void* pNext = nullptr;" << std::endl;
        file << "    bool serialized = false;" << std::endl;
        file << "    archive(serialized);" << std::endl;
        file << "    if (serialized) {" << std::endl;
        file << "        VkStructureType sType { };" << std::endl;
        file << "        archive(sType);" << std::endl;
        file << "        assert(tlpDecerealizationAllocator);" << std::endl;
        file << "        auto pAllocator = tlpDecerealizationAllocator;" << std::endl;
        generate_pnext_switch(
            file,
            manifest,
            "        ",
            "sType",
            std::vector<std::string> {
                "pNext = pAllocator->pfnAllocation(pAllocator->pUserData, sizeof({structureType}), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);",
                "archive(*({structureType}*)pNext);",
            },
            "assert(false && \"Unrecognized VkStructureType\");"
        );
        file << "    }" << std::endl;
        file << "    return pNext;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
