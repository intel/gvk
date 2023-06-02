
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

class EnumerateFormatsGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_FORMAT_INFO_GENERATED_INCLUDE_PATH "/enumerate-formats.hpp");
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "template <typename ProcessFormatFunctionType>" << std::endl;
        file << "inline void enumerate_formats(ProcessFormatFunctionType processFormat)" << std::endl;
        file << "{" << std::endl;
        auto itr = manifest.enumerations.find("VkFormat");
        assert(itr != manifest.enumerations.end());
        for (const auto& enumerator : itr->second.enumerators) {
            // NOTE : I'd prefer that enumerate_formats() enumerates all VkFormats, but the
            //  validation layer has started complaining...
            //      vkGetPhysicalDeviceFormatProperties2: value of format (...) does not
            //      fall within the begin..end range of the core VkFormat enumeration
            //      tokens and is not an extension added token
            //  ...the check here blocks non-core VkFormats from being enumerated.
            // TODO : Find a way for enumerate_formats() to know what VkFormats are enabled
            //  so that all available VkFormats can be enumerated.
            if (string::to_number<int64_t>(enumerator.value) < xml::Enumerator::ExtensionBaseValue) {
                file << "    if (!processFormat(" << enumerator.name << ")) {" << std::endl;
                file << "        return;" << std::endl;
                file << "    }" << std::endl;
            }
        }
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
