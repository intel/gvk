
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

#include "gvk-cppgen/include.hpp"

namespace gvk {
namespace cppgen {

class DeserializeStructureGenerator final
{
public:
    static void generate(
        const std::string& structureCollectionName,
        const std::string& structureCollectionInclude,
        const std::vector<xml::Structure>& structures
    )
    {
        ModuleGenerator module(
            GVK_STRUCTURES_GENERATED_INCLUDE_PATH,
            GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX,
            GVK_STRUCTURES_GENERATED_SOURCE_PATH,
            structureCollectionName + "-deserialization"
        );
        module.header << "#include \"gvk-structures/defines.hpp\"" << std::endl;
        if (!structureCollectionInclude.empty()) {
            module.header << "#include \"" << structureCollectionInclude << "\"" << std::endl;
        }
        generate_header(module.header, structures);
        module.source << "#include \"" << GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX << structureCollectionName << "-create-copy.hpp\"" << std::endl;
        module.source << "#include \"" << GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX << structureCollectionName << "-destroy-copy.hpp\"" << std::endl;
        module.source << "#include \"" << GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX << structureCollectionName << "-decerealization.hpp\"" << std::endl;
        if (structureCollectionName != "core-structures") {
            module.source << "#include \"gvk-structures/generated/core-structures-decerealization.hpp\"" << std::endl;
        }
        generate_source(module.source, structures);
    }

private:
    static void generate_header(FileGenerator& file, const std::vector<xml::Structure>& structures)
    {
        file << "#include \"gvk-structures/auto.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <iosfwd>" << std::endl;
        file << std::endl;
        gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        for (const auto& structure : structures) {
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << string::replace("void deserialize(std::istream& istrm, const VkAllocationCallbacks* pAllocator, Auto<{structureType}>& obj);", "{structureType}", structure.name) << std::endl;
        }
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const std::vector<xml::Structure>& structures)
    {
        file << std::endl;
        file << "#include <istream>" << std::endl;
        file << std::endl;
        gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
        for (const auto& structure : structures) {
            if (structure.alias.empty()) {
                file << std::endl;
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace(
R"(void deserialize(std::istream& istrm, const VkAllocationCallbacks* pAllocator, Auto<{structureType}>& obj)
{
    assert(!pAllocator && "TODO : VkAllocationCallbacks need to be hooked up in gvk::Auto<>");
    obj.reset();
    cereal::BinaryInputArchive archive(istrm);
    detail::tlpDecerealizationAllocator = detail::validate_allocation_callbacks(pAllocator);
    archive(const_cast<{structureType}&>(*obj));
}
)", "{structureType}", structure.name);
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
