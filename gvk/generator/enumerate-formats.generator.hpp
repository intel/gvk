
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
