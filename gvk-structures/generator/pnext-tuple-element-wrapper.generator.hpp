
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

class PNextTupleElementWrapperGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_STRUCTURES_GENERATED_SOURCE_PATH "/pnext-tuple-element-wrapper.cpp");
        file << std::endl;
        file << "#include \"gvk-structures/generated/core-structure-comparison-operators.hpp\"" << std::endl;
        file << "#include \"gvk-structures/detail/make-tuple-utilities.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;
        file << "bool operator==(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)" << std::endl;
        file << "{" << std::endl;
        file << "    if (lhs.pNext && rhs.pNext) {" << std::endl;
        file << "        auto lhsSType = ((const VkBaseInStructure*)lhs.pNext)->sType;" << std::endl;
        file << "        auto rhsSType = ((const VkBaseInStructure*)rhs.pNext)->sType;" << std::endl;
        file << "        if (lhsSType != rhsSType) {" << std::endl;
        file << "            return false;" << std::endl;
        file << "        }" << std::endl;
        generate_pnext_switch(
            file,
            manifest,
            "        ",
            "lhsSType",
            "return *(const {structureType}*)lhs.pNext == *(const {structureType}*)rhs.pNext;",
            "assert(false && \"Unsupported VkStructureType\");"
        );
        file << "    }" << std::endl;
        file << "    return !lhs.pNext == !rhs.pNext;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "bool operator<(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs)" << std::endl;
        file << "{" << std::endl;
        file << "    if (lhs.pNext && rhs.pNext) {" << std::endl;
        file << "        auto lhsSType = ((const VkBaseInStructure*)lhs.pNext)->sType;" << std::endl;
        file << "        auto rhsSType = ((const VkBaseInStructure*)rhs.pNext)->sType;" << std::endl;
        file << "        if (lhsSType != rhsSType) {" << std::endl;
        file << "            return lhsSType < rhsSType;" << std::endl;
        file << "        }" << std::endl;
        generate_pnext_switch(
            file,
            manifest,
            "        ",
            "lhsSType",
            "return *(const {structureType}*)lhs.pNext < *(const {structureType}*)rhs.pNext;",
            "assert(false && \"Unsupported VkStructureType\");"
        );
        file << "    }" << std::endl;
        file << "    return !lhs.pNext == !!rhs.pNext;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
