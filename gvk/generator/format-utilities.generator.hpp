
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

class FormatUtilitiesGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        Module module("format-utilities");
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(File& file, const xml::Manifest& manifest)
    {
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;

        // Generate ComponentName enum
        file << "enum class ComponentName" << std::endl;
        file << "{" << std::endl;
        file << "    CN_None," << std::endl;
        std::set<std::string> formatNames;
        for (const auto& formatItr : manifest.formats) {
            for (const auto& component : formatItr.second.components) {
                if (formatNames.insert(component.name).second) {
                    file << "    CN_" << component.name << "," << std::endl;
                }
            }
        }
        file << "};" << std::endl;
        file << std::endl;

        // Generate CompressionType enum
        file << "enum class CompressionType" << std::endl;
        file << "{" << std::endl;
        file << "    CT_None," << std::endl;
        std::set<std::string> compressionTypes;
        for (const auto& formatItr : manifest.formats) {
            const auto& format = formatItr.second;
            if (!format.compressionType.empty() && compressionTypes.insert(format.compressionType).second) {
                file << "    CT_" << string::replace(format.compressionType, " ", "_") << "," << std::endl;
            }
        }
        file << "};" << std::endl;
        file << std::endl;

        // Generate FormatClass enum
        file << "enum class FormatClass" << std::endl;
        file << "{" << std::endl;
        file << "    FC_None," << std::endl;
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

        // Generate NumericFormat enum
        file << "enum class NumericFormat" << std::endl;
        file << "{" << std::endl;
        file << "    NF_None," << std::endl;
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

        // Generate enumerate_formats()
        file << "template <typename ProcessFormatFunctionType>" << std::endl;
        file << "inline void enumerate_formats(ProcessFormatFunctionType processFormat)" << std::endl;
        file << "{" << std::endl;
        auto itr = manifest.enumerations.find("VkFormat");
        assert(itr != manifest.enumerations.end());
        for (const auto& enumerator : itr->second.enumerators) {
            file << "    if (!processFormat(" << enumerator.name << ")) {" << std::endl;
            file << "        return;" << std::endl;
            file << "    }" << std::endl;
        }
        file << "}" << std::endl;
        file << std::endl;
    }

    static void generate_source(File& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk/format.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;

        // Generate get_format_info()
        file << "const FormatInfo& get_format_info(VkFormat format)" << std::endl;
        file << "{" << std::endl;
        file << "    switch (format) {" << std::endl;
        auto tab = "            ";
        for (const auto& formatItr : manifest.formats) {
            const auto& format = formatItr.second;
            file << "    case " << format.name << ": {" << std::endl;
            file << "        static const FormatInfo sFormatInfo {" << std::endl;

            // Generate FormatInfo classes array
            file << tab << "/* .classes          = */ std::set<FormatClass> {" << std::endl;
            for (const auto& formatClass : format.classes) {
                file << tab << "    FormatClass::FC_" << string::replace(formatClass, "-", "_") << "," << std::endl;
            }
            file << tab << "}," << std::endl;

            // Generate FormatInfo fields
            file << tab << "/* .blockSize        = */ " << format.blockSize << "," << std::endl;
            file << tab << "/* .texelsPerBlock   = */ " << format.texelsPerBlock << "," << std::endl;
            file << tab << "/* .chroma           = */ " << format.chroma << "," << std::endl;
            file << tab << "/* .packed           = */ " << format.packed << "," << std::endl;

            // Generate FormatInfo blockExtent field
            file << tab << "/* .blockExtent      = */ std::array<uint32_t, 3> {" << std::endl;
            if (format.blockExtent[0] || format.blockExtent[1] || format.blockExtent[2]) {
                for (const auto& dimension : format.blockExtent) {
                    file << tab << "    " << dimension << "," << std::endl;
                }
            }
            file << tab <<"}," << std::endl;

            // Generate FormatInfo compressionType field
            std::string compressionType = "CompressionType::CT_None";
            if (!format.compressionType.empty()) {
                compressionType = "CompressionType::CT_" + string::replace(format.compressionType, " ", "_");
            }
            file << tab << "/* .compressionType  = */ " << compressionType << "," << std::endl;

            // Generate FormatInfo numericFormat
            std::string numericFormat = "None";
            if (!format.components.empty()) {
                numericFormat = format.components.front().numericFormat;
                for (const auto& component : format.components) {
                    if (component.numericFormat != numericFormat) {
                        numericFormat = "None";
                        break;
                    }
                }
            }
            file << tab << "/* .numericFormat    = */ NumericFormat::NF_" << numericFormat << "," << std::endl;

            // Generate FormatInfo spirvImageFormat field
            auto spirvImageFormat = !format.spirvImageFormat.empty() ? format.spirvImageFormat : "{ }";
            file << tab << "/* .spirvImageFormat = */ " << spirvImageFormat << "," << std::endl;

            // Generate FormatInfo planes array
            file << tab << "/* .planes           = */ std::vector<FormatInfo::Plane> {" << std::endl;
            for (const auto& plane : format.planes) {
                file << tab << "    {" << std::endl;
                file << tab << "        /* .index         = */ " << plane.index << "," << std::endl;
                file << tab << "        /* .widthDivisor  = */ " << plane.widthDivisor << "," << std::endl;
                file << tab << "        /* .heightDivisor = */ " << plane.heightDivisor << "," << std::endl;
                file << tab << "        /* .compatible    = */ " << plane.compatible << "," << std::endl;
                file << tab << "    }," << std::endl;
            }
            file << tab << "}," << std::endl;

            // Generate FormatInfo components array
            file << tab << "/* .components       = */ std::vector<FormatInfo::Component> {" << std::endl;
            for (const auto& component : format.components) {
                file << tab << "    {" << std::endl;
                file << tab << "        /* .name          = */ ComponentName::CN_" << component.name << "," << std::endl;
                file << tab << "        /* .bits          = */ " << component.bits << "," << std::endl;
                file << tab << "        /* .numericFormat = */ NumericFormat::NF_" << component.numericFormat << "," << std::endl;
                file << tab << "        /* .planeIndex    = */ " << component.planeIndex << "," << std::endl;
                file << tab << "    }," << std::endl;
            }
            file << tab << "}," << std::endl;
            file << "        };" << std::endl;
            file << "        return sFormatInfo;" << std::endl;
            file << "    } break;" << std::endl;
        }
        file << "    default:{" << std::endl;
        file << "        static const FormatInfo sFormatInfo { };" << std::endl;
        file << "        return sFormatInfo;" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
