
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

#include "gvk/xml/manifest.hpp"
#include "gvk/printer.hpp"
#include "gvk/string.hpp"

#include <iostream>

inline void print_api_element_fields(gvk::Printer& printer, const gvk::xml::ApiElement& obj)
{
    printer.print_field("name", obj.name);
    if (!obj.alias.empty()) {
        printer.print_field("alias", obj.alias);
    }
    if (!obj.extension.empty()) {
        printer.print_field("extension", obj.extension);
    }
    if (!obj.compileGuards.empty()) {
        printer.print_collection("compileGuards", obj.compileGuards);
    }
}

template <>
void gvk::print<gvk::xml::Platform>(gvk::Printer& printer, const gvk::xml::Platform& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
        }
    );
}

template <>
void gvk::print<gvk::xml::Handle>(gvk::Printer& printer, const gvk::xml::Handle& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            if (obj.isDispatchable) {
                printer.print_field("isDispatchable", obj.isDispatchable);
            }
            if (!obj.vkObjectType.empty()) {
                printer.print_field("vkObjectType", obj.vkObjectType);
            }
            if (!obj.parents.empty()) {
                printer.print_collection("parents", obj.parents);
            }
            if (!obj.createInfos.empty()) {
                printer.print_collection("createInfos", obj.createInfos);
            }
            if (!obj.createCommands.empty()) {
                printer.print_collection("createCommands", obj.createCommands);
            }
            if (!obj.destroyCommands.empty()) {
                printer.print_collection("destroyCommands", obj.destroyCommands);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::Enumerator>(gvk::Printer& printer, const gvk::xml::Enumerator& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            if (!obj.value.empty()) {
                printer.print_field("value", obj.value);
            }
            if (!obj.bitPos.empty()) {
                printer.print_field("bitPos", obj.bitPos);
            }
            if (!obj.extensionNumber.empty()) {
                printer.print_field("extensionNumber", obj.extensionNumber);
            }
            if (!obj.offset.empty()) {
                printer.print_field("offset", obj.offset);
            }
            if (!obj.direction.empty()) {
                printer.print_field("direction", obj.direction);
            }
            if (!obj.extends.empty()) {
                printer.print_field("extends", obj.extends);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::Enumeration>(gvk::Printer& printer, const gvk::xml::Enumeration& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            if (obj.isBitmask) {
                printer.print_field("isBitmask", obj.isBitmask);
            }
            if (!obj.enumerators.empty()) {
                printer.print_collection("enumerators", obj.enumerators);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::FlagBits>(
    gvk::Printer& printer,
    std::underlying_type_t<gvk::xml::FlagBits> flags
)
{
    std::string flagsStr;
    if (printer.get_flags() & Printer::EnumIdentifier) {
        flagsStr = gvk::flags_to_string(flags,
            std::initializer_list<std::pair<gvk::xml::FlagBits, const char*>> {
                { gvk::xml::Optional, "gvk::xml::Optional" },
                { gvk::xml::Dynamic,  "gvk::xml::Dynamic"  },
                { gvk::xml::Static,   "gvk::xml::Static"   },
                { gvk::xml::Const,    "gvk::xml::Const"    },
                { gvk::xml::Pointer,  "gvk::xml::Pointer"  },
                { gvk::xml::Array,    "gvk::xml::Array"    },
                { gvk::xml::String,   "gvk::xml::String"   },
                { gvk::xml::Void,     "gvk::xml::Void"     },
                { gvk::xml::Function, "gvk::xml::Function" },
            }
        );
    }
    printer.print_enum(!flagsStr.empty() ? flagsStr.c_str() : nullptr, (gvk::xml::FlagBits)flags);
}

template <>
void gvk::print<gvk::xml::Parameter>(gvk::Printer& printer, const gvk::xml::Parameter& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            printer.print_field("type", obj.type);
            if (obj.unqualifiedType != obj.type) {
                printer.print_field("unqualifiedType", obj.unqualifiedType);
            }
            if (!obj.length.empty()) {
                printer.print_field("length", string::replace(obj.length, "\\", "\\\\"));
            }
            if (!obj.altLength.empty()) {
                printer.print_field("altLength", obj.altLength);
            }
            if (!obj.selector.empty()) {
                printer.print_field("selector", obj.selector);
            }
            if (!obj.limitType.empty()) {
                printer.print_field("limitType", obj.limitType);
            }
            if (!obj.values.empty()) {
                printer.print_collection("values", obj.values);
            }
            if (1 < obj.dimensionCount) {
                printer.print_field("dimensionCount", obj.dimensionCount);
            }
            if (obj.bitField) {
                printer.print_field("bitField", obj.bitField);
            }
            if (obj.flags) {
                printer.print_flags<gvk::xml::FlagBits>("flags", obj.flags);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::Structure>(gvk::Printer& printer, const gvk::xml::Structure& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            if (obj.isUnion) {
                printer.print_field("isUnion", obj.isUnion);
            }
            if (!obj.vkStructureType.empty()) {
                printer.print_field("vkStructureType", obj.vkStructureType);
            }
            if (!obj.members.empty()) {
                printer.print_collection("members", obj.members);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::Command::Type>(gvk::Printer& printer, const gvk::xml::Command::Type& value)
{
    switch (value) {
    case gvk::xml::Command::Type::Common: printer.print_enum("gvk::xml::Command::Type::Common", value); break;
    case gvk::xml::Command::Type::Cmd: printer.print_enum("gvk::xml::Command::Type::Cmd", value); break;
    case gvk::xml::Command::Type::Create: printer.print_enum("gvk::xml::Command::Type::Create", value); break;
    case gvk::xml::Command::Type::Destroy: printer.print_enum("gvk::xml::Command::Type::Destroy", value); break;
    default: assert(false);
    }
}

template <>
void gvk::print<gvk::xml::Command>(gvk::Printer& printer, const gvk::xml::Command& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            printer.print_field("type", obj.type);
            if (!obj.target.empty()) {
                printer.print_field("target", obj.target);
            }
            printer.print_field("returnType", obj.returnType);
            if (!obj.successCodes.empty()) {
                printer.print_collection("successCodes", obj.successCodes);
            }
            if (!obj.errorCodes.empty()) {
                printer.print_collection("errorCodes", obj.errorCodes);
            }
            if (!obj.parameters.empty()) {
                printer.print_collection("parameters", obj.parameters);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::Extension::Type>(gvk::Printer& printer, const gvk::xml::Extension::Type& value)
{
    switch (value) {
    case gvk::xml::Extension::Type::Instance: printer.print_enum("gvk::xml::Extension::Type::Instance", value); break;
    case gvk::xml::Extension::Type::Device: printer.print_enum("gvk::xml::Extension::Type::Device", value); break;
    default: assert(false);
    }
}

template <>
void gvk::print<gvk::xml::Extension>(gvk::Printer& printer, const gvk::xml::Extension& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            printer.print_field("number", obj.number);
            printer.print_field("type", obj.type);
            if (!obj.platform.empty()) {
                printer.print_field("platform", obj.platform);
            }
            if (!obj.supported.empty()) {
                printer.print_field("supported", obj.supported);
            }
            if (!obj.deprecatedBy.empty()) {
                printer.print_field("deprecatedBy", obj.deprecatedBy);
            }
            if (!obj.obsoletedBy.empty()) {
                printer.print_field("obsoletedBy", obj.obsoletedBy);
            }
            if (!obj.promotedTo.empty()) {
                printer.print_field("promotedTo", obj.promotedTo);
            }
            if (!obj.requirements.empty()) {
                printer.print_collection("requirements", obj.requirements);
            }
            if (!obj.types.empty()) {
                printer.print_collection("types", obj.types);
            }
            if (!obj.enumerations.empty()) {
                printer.print_collection("enumerations", obj.enumerations, [](auto itr) { return itr.second; });
            }
            if (!obj.commands.empty()) {
                printer.print_collection("commands", obj.commands);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::Feature>(gvk::Printer& printer, const gvk::xml::Feature& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            printer.print_field("api", obj.api);
            printer.print_field("number", obj.number);
            if (!obj.requirements.empty()) {
                printer.print_collection("requirements", obj.requirements);
            }
            if (!obj.types.empty()) {
                printer.print_collection("types", obj.types);
            }
            if (!obj.enumerations.empty()) {
                printer.print_collection("enumerations", obj.enumerations, [](auto itr) { return itr.second; });
            }
            if (!obj.commands.empty()) {
                printer.print_collection("commands", obj.commands);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::Plane>(gvk::Printer& printer, const gvk::xml::Plane& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("index", obj.index);
            printer.print_field("widthDivisor", obj.widthDivisor);
            printer.print_field("heightDivisor", obj.heightDivisor);
            printer.print_field("compatible", obj.compatible);
        }
    );
}

template <>
void gvk::print<gvk::xml::Component>(gvk::Printer& printer, const gvk::xml::Component& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("name", obj.name);
            printer.print_field("bits", obj.bits);
            printer.print_field("numericFormat", obj.numericFormat);
            printer.print_field("planeIndex", obj.planeIndex);
        }
    );
}

template <>
void gvk::print<gvk::xml::Format>(gvk::Printer& printer, const gvk::xml::Format& obj)
{
    printer.print_object(
        [&]()
        {
            print_api_element_fields(printer, obj);
            if (!obj.classes.empty()) {
                printer.print_collection("classes", obj.classes);
            }
            printer.print_field("blockSize", obj.blockSize);
            printer.print_field("texelsPerBlock", obj.texelsPerBlock);
            if (obj.chroma) {
                printer.print_field("chroma", obj.chroma);
            }
            if (obj.packed) {
                printer.print_field("packed", obj.packed);
            }
            if (obj.blockExtent[0] || obj.blockExtent[1] || obj.blockExtent[2]) {
                printer.print_collection("blockExtent", obj.blockExtent);
            }
            if (!obj.compressionType.empty()) {
                printer.print_field("compressionType", obj.compressionType);
            }
            if (!obj.spirvImageFormat.empty()) {
                printer.print_field("spirvImageFormat", obj.spirvImageFormat);
            }
            if (!obj.components.empty()) {
                printer.print_collection("components", obj.components);
            }
            if (!obj.planes.empty()) {
                printer.print_collection("planes", obj.planes);
            }
        }
    );
}

template <>
void gvk::print<gvk::xml::Manifest>(gvk::Printer& printer, const gvk::xml::Manifest& obj)
{
    printer.print_object(
        [&]()
        {
            auto processItr = [](auto itr) { return itr.second; };
            printer.print_collection("platforms", obj.platforms, processItr);
            printer.print_collection("vendors", obj.vendors);
            printer.print_field("apiConstants", obj.apiConstants);
            printer.print_collection("handles", obj.handles, processItr);
            printer.print_collection("enumerations", obj.enumerations, processItr);
            printer.print_collection("structures", obj.structures, processItr);
            printer.print_collection("commands", obj.commands, processItr);
            printer.print_collection("extensions", obj.extensions, processItr);
            printer.print_collection("features", obj.features, processItr);
            printer.print_collection("formats", obj.formats, processItr);
        }
    );
}

int main(int argc, const char* pArgv[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile((argc > 1 ? pArgv[1] : GVK_XML_FILE_PATH));
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        std::cout << gvk::to_string(manifest) << std::endl;
    }
    return 0;
}
