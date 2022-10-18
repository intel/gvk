
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

#include "gvk/xml/manifest.hpp"
#include "cppgen-utilities.hpp"

namespace gvk {
namespace cppgen {

class DeserializeStructuresGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("deserialize-structures");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <iosfwd>" << std::endl;
        file << std::endl;
        gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
            if (structure.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
                file << string::replace("void deserialize(std::istream& istrm, const VkAllocationCallbacks* pAllocator, Auto<{structureType}>& obj);", "{structureType}", structure.name) << std::endl;
            }
        }
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/generated/decerealize-structures.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <istream>" << std::endl;
        file << std::endl;
        gvk::cppgen::NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "thread_local const VkAllocationCallbacks* detail::tlpDecerealizationAllocator;" << std::endl;
        for (const auto& structureItr : manifest.structures) {
            const auto& structure = structureItr.second;
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
