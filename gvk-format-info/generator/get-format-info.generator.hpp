
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

class GetFormatInfoGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_FORMAT_INFO_GENERATED_SOURCE_PATH "/get-format-info.cpp");
        file << std::endl;
        file << "#include \"gvk-format-info.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <array>" << std::endl;
        file << "#include <cassert>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk");
        file << std::endl;
        file << "void get_format_info(VkFormat format, GvkFormatInfo* pFormatInfo)" << std::endl;
        file << "{" << std::endl;
        file << "    assert(pFormatInfo);" << std::endl;
        file << "    switch (format) {" << std::endl;
        for (const auto& formatItr : manifest.formats) {
            const auto& format = formatItr.second;
            file << "    case " << format.name << ": {" << std::endl;

            file << "        pFormatInfo->blockSize = " << format.blockSize << ";" << std::endl;
            file << "        pFormatInfo->texelsPerBlock = " << format.texelsPerBlock << ";" << std::endl;
            file << "        pFormatInfo->chroma = " << format.chroma << ";" << std::endl;
            file << "        pFormatInfo->packed = " << format.packed << ";" << std::endl;
            file << "        pFormatInfo->blockExtent[0] = " << format.blockExtent[0] << ";" << std::endl;
            file << "        pFormatInfo->blockExtent[1] = " << format.blockExtent[1] << ";" << std::endl;
            file << "        pFormatInfo->blockExtent[2] = " << format.blockExtent[2] << ";" << std::endl;

            if (!format.compressionType.empty()) {
                file << "        pFormatInfo->compressionType = GVK_FORMAT_COMPRESSION_TYPE_" << string::replace(format.compressionType, " ", "_") << ";" << std::endl;
            } else {
                file << "        pFormatInfo->compressionType = GVK_FORMAT_COMPRESSION_TYPE_NONE;" << std::endl;
            }

            std::string numericFormat = "UNDEFINED";
            if (!format.components.empty()) {
                numericFormat = format.components.front().numericFormat;
                for (const auto& component : format.components) {
                    if (component.numericFormat != numericFormat) {
                        numericFormat = "UNDEFINED";
                        break;
                    }
                }
            }
            file << "        pFormatInfo->numericFormat = GVK_NUMERIC_FORMAT_" << numericFormat << ";" << std::endl;

            auto spirvImageFormat = !format.spirvImageFormat.empty() ? "\"" + format.spirvImageFormat + "\"" : "nullptr";
            file << "        pFormatInfo->pSpirvImageFormat = " << spirvImageFormat << ";" << std::endl;

            if (!format.classes.empty()) {
                file << "        static const std::array<GvkFormatClass, " << format.classes.size() << "> scClasses {" << std::endl;
                for (const auto& formatClass : format.classes) {
                    file << "            GVK_FORMAT_CLASS_" << string::to_upper(string::replace(formatClass, "-", "_")) << "," << std::endl;
                }
                file << "        };" << std::endl;
                file << "        pFormatInfo->classCount = (uint32_t)scClasses.size();" << std::endl;
                file << "        pFormatInfo->pClasses = scClasses.data();" << std::endl;
            } else {
                file << "        pFormatInfo->classCount = 0;" << std::endl;
                file << "        pFormatInfo->pClasses = nullptr;" << std::endl;
            }

            if (!format.planes.empty()) {
                file << "        static const std::array<GvkFormatPlaneInfo, " << format.planes.size() << "> scPlanes {" << std::endl;
                for (const auto& plane : format.planes) {
                    file << "            GvkFormatPlaneInfo {" << std::endl;
                    file << "                /* index         */ " << plane.index << "," << std::endl;
                    file << "                /* widthDivisor  */ " << plane.widthDivisor << "," << std::endl;
                    file << "                /* heightDivisor */ " << plane.heightDivisor << "," << std::endl;
                    file << "                /* compatible    */ " << plane.compatible << "," << std::endl;
                    file << "            }," << std::endl;
                }
                file << "        };" << std::endl;
                file << "        pFormatInfo->planeCount = (uint32_t)scPlanes.size();" << std::endl;
                file << "        pFormatInfo->pPlanes = scPlanes.data();" << std::endl;
            } else {
                file << "        pFormatInfo->planeCount = 0;" << std::endl;
                file << "        pFormatInfo->pPlanes = nullptr;" << std::endl;
            }

            if (!format.components.empty()) {
                file << "        static const std::array<GvkFormatComponentInfo, " << format.components.size() << "> scComponents {" << std::endl;
                for (const auto& component : format.components) {
                    file << "            GvkFormatComponentInfo {" << std::endl;
                    file << "                /* name          */ GVK_FORMAT_COMPONENT_NAME_" << component.name << "," << std::endl;
                    file << "                /* bits          */ " << component.bits << "," << std::endl;
                    file << "                /* numericFormat */ GVK_NUMERIC_FORMAT_" << component.numericFormat << "," << std::endl;
                    file << "                /* planeIndex    */ " << component.planeIndex << "," << std::endl;
                    file << "            }," << std::endl;
                }
                file << "        };" << std::endl;
                file << "        pFormatInfo->componentCount = (uint32_t)scComponents.size();" << std::endl;
                file << "        pFormatInfo->pComponents = scComponents.data();" << std::endl;
            } else {
                file << "        pFormatInfo->componentCount = 0;" << std::endl;
                file << "        pFormatInfo->pComponents = nullptr;" << std::endl;
            }

            file << "    } break;" << std::endl;
        }
        file << "    default: {" << std::endl;
        file << "        *pFormatInfo = { };" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
