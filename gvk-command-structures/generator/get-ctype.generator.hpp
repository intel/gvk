
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

class CommandStructureGetCTypeGenerator final
{
public:
    static void generate(const ApiElementCollectionInfo& apiElements)
    {
        FileGenerator file(apiElements.includePath + "/" + apiElements.name + "-structure-get-ctype.hpp");
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        for (const auto& include : apiElements.headerIncludes) {
            file << "#include \"" << include << "\"" << std::endl;
        }
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;

        file << "inline constexpr GvkCommandStructureCommandTypeFlags get_ctype(GvkCommandStructureType sType)" << std::endl;
        file << "{" << std::endl;
        file << "    switch (sType) {" << std::endl;
        for (const auto& structure : apiElements.structures) {
            if (!structure.vkStructureType.empty() && 1 < structure.members.size()) {
                CompileGuardGenerator compileGuards(file, structure.compileGuards);
                file << "    case " << structure.vkStructureType << ": {" << std::endl;
                file << "        return ";
                if (structure.members[1].type == "VkInstance") {
                    file << "GVK_COMMAND_STRUCTURE_COMMAND_TYPE_INSTANCE";
                } else if (structure.members[1].type == "VkPhysicalDevice") {
                    file << "GVK_COMMAND_STRUCTURE_COMMAND_TYPE_PHYSICAL_DEVICE";
                } else if (structure.members[1].type == "VkDevice") {
                    file << "GVK_COMMAND_STRUCTURE_COMMAND_TYPE_DEVICE";
                } else if (structure.members[1].type == "VkQueue") {
                    file << "GVK_COMMAND_STRUCTURE_COMMAND_TYPE_QUEUE";
                } else if (structure.members[1].type == "VkCommandBuffer") {
                    file << "GVK_COMMAND_STRUCTURE_COMMAND_TYPE_COMMAND_BUFFER";
                } else {
                    file << "GVK_COMMAND_STRUCTURE_COMMAND_TYPE_INSTANCE";
                }
                if (gvk::string::contains(structure.name, "Create") || gvk::string::contains(structure.name, "Allocate")) {
                    file << " | GVK_COMMAND_STRUCTURE_COMMAND_TYPE_CREATE";
                } else if (gvk::string::contains(structure.name, "Destroy") || gvk::string::contains(structure.name, "Free")) {
                    file << " | GVK_COMMAND_STRUCTURE_COMMAND_TYPE_DESTROY";
                }
                file << ";" << std::endl << "    } break;" << std::endl;
            }
        }
        file << "    default: {" << std::endl;
        file << "        return 0;" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
