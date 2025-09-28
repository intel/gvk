
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

class CommandCollectionCerealizationGenerator final
{
public:
    static void generate(const ApiElementCollectionInfo& apiElements)
    {
        FileGenerator file(GVK_COMMAND_STRUCTURES_GENERATED_INCLUDE_PATH "/command-collection-cerealization.hpp");
        file << "#include \"gvk-command-structures/generated/command-structure-cerealization.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures/generated/command-structure-decerealization.hpp\"" << std::endl;
        file << "#include \"gvk-structures/detail/cerealization-utilities.hpp\"" << std::endl;
        file << std::endl;
        file << "namespace cereal {" << std::endl;
        file << std::endl;
        file << "template <typename ArchiveType>" << std::endl;
        file << "inline void save(ArchiveType& archive, const GvkCommandCollection& obj)" << std::endl;
        file << "{" << std::endl;
        file << "    archive(obj.commandCount);" << std::endl;
        file << "    for (uint32_t i = 0; i < obj.commandCount; ++i) {" << std::endl;
        file << "        assert(obj.ppCommands[i]);" << std::endl;
        file << "        archive(obj.ppCommands[i]->sType);" << std::endl;
        generate_type_erased_structure_switch(
            file,
            apiElements,
            "        ",
            "obj.ppCommands[i]->sType",
            "archive(*(const {structureType}*)obj.ppCommands[i]);",
            "assert(false && \"Unrecognized structure type\");"
        );
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "template <typename ArchiveType>" << std::endl;
        file << "inline void load(ArchiveType& archive, GvkCommandCollection& obj)" << std::endl;
        file << "{" << std::endl;
        file << "    archive(obj.commandCount);" << std::endl;
        file << "    if (obj.commandCount) {" << std::endl;
        file << "        assert(gvk::detail::tlpDecerealizationAllocator);" << std::endl;
        file << "        auto pAllocator = gvk::detail::tlpDecerealizationAllocator;" << std::endl;
        file << "        auto ppCommands = (GvkCommandBaseStructure**)pAllocator->pfnAllocation(pAllocator->pUserData, obj.commandCount * sizeof(GvkCommandBaseStructure*), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);" << std::endl;
        file << "        for (uint32_t i = 0; i < obj.commandCount; ++i) {" << std::endl;
        file << "            GvkCommandStructureType sType = GVK_COMMAND_STRUCTURE_TYPE_UNDEFINED;" << std::endl;
        file << "            archive(sType);" << std::endl;
        generate_type_erased_structure_switch(
            file,
            apiElements,
            "            ",
            "sType",
            std::vector<std::string> {
                "ppCommands[i] = (GvkCommandBaseStructure*)pAllocator->pfnAllocation(pAllocator->pUserData, sizeof({structureType}), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);",
                "archive(*({structureType}*)ppCommands[i]);"
            },
            "assert(false && \"Unrecognized structure type\");"
        );
        file << "        }" << std::endl;
        file << "        obj.ppCommands = ppCommands;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "} // namespace cereal" << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
