
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

class EnumerateFormatsGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        File file("enumerate-formats.hpp");
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;

        // TODO : Documentation
        file << "enum class CompressionType" << std::endl;
        file << "{" << std::endl;
        file << "    CT_None," << std::endl;
        std::set<std::string> compressionTypes;
        for (const auto& formatItr : manifest.formats) {
            const auto& format = formatItr.second;
            if (!format.compression.empty() && compressionTypes.insert(format.compression).second) {
                file << "    CT_" << string::replace(format.compression, " ", "_") << "," << std::endl;
            }
        }
        file << "};" << std::endl;
        file << std::endl;

        // TODO : Documentation
        file << "enum class FormatClass" << std::endl;
        file << "{" << std::endl;
        std::set<std::string> formatClasses;
        for (const auto& formatItr : manifest.formats) {
            for (const auto& formatClass : formatItr.second.classes) {
                if (formatClasses.insert(formatClass).second) {
                    file << "    FC_" << string::replace(formatClass, "-", "_") << "," << std::endl;
                }
            }
        }
        file << "};" << std::endl;
        file << std::endl;

        // TODO : Documentation
        file << "enum class NumericFormat" << std::endl;
        file << "{" << std::endl;
        std::set<std::string> numericFormats;
        for (const auto& formatItr : manifest.formats) {
            for (const auto& component : formatItr.second.components) {
                if (numericFormats.insert(component.numericFormat).second) {
                    file << "    NF_" << component.numericFormat << "," << std::endl;
                }
            }
        }
        file << "};" << std::endl;
        file << std::endl;

        // TODO : Documentation
        file << "template <typename ProcessFormatFunctionType>" << std::endl;
        file << "inline void enumerate_formats(ProcessFormatFunctionType processFormat)" << std::endl;
        file << "{" << std::endl;
        auto itr = manifest.enumerations.find("VkFormat");
        assert(itr != manifest.enumerations.end());
        for (const auto& enumerator : itr->second.enumerators) {
            file << "    processFormat(" << enumerator.name << ");" << std::endl;
        }
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
