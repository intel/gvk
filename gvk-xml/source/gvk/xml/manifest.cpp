
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
#include "tinyxml2-utilities.hpp"

#include <cassert>
#include <map>

namespace gvk {
namespace xml {

template <typename ApiElementType>
static void create_api_element(const tinyxml2::XMLElement& xmlElement, std::map<std::string, ApiElementType>& apiElements)
{
    ApiElementType apiElement(xmlElement);
    assert(!apiElement.name.empty());
    apiElements.insert({ apiElement.name, apiElement });
}

template <typename ApiElementCollectionType>
static void post_process_vendors(const std::set<std::string>& vendors, ApiElementCollectionType& apiElements)
{
    for (auto& apiElementItr : apiElements) {
        auto& apiElement = apiElementItr.second;
        if (apiElement.vendor.empty()) {
            auto tokens = string::split_camel_case(apiElement.name);
            if (!tokens.empty() && vendors.count(tokens.back())) {
                apiElement.vendor = tokens.back();
            }
        }
    }
}

static void post_process_api_constants(Manifest& manifest)
{
    auto apiConstantsItr = manifest.enumerations.find("API Constants");
    if (apiConstantsItr != manifest.enumerations.end()) {
        manifest.apiConstants = apiConstantsItr->second;
        manifest.enumerations.erase(apiConstantsItr);
    }
}

template <typename ApiElementCollectionType>
static void post_process_extension(const Extension& extension, const std::string& apiElementName, ApiElementCollectionType& apiElements)
{
    auto apiElementItr = apiElements.find(apiElementName);
    if (apiElementItr != apiElements.end()) {
        if (extension.supported == "disabled") {
            apiElements.erase(apiElementItr);
        } else {
            auto& apiElement = apiElementItr->second;
            apiElement.extension = extension.name;
            apiElement.compileGuards.insert(extension.compileGuards.begin(), extension.compileGuards.end());
        }
    }
}

static void post_process_extensions(Manifest& manifest)
{
    for (auto extensionItr : manifest.extensions) {
        auto& extension = extensionItr.second;
        auto platformItr = manifest.platforms.find(extension.platform);
        if (platformItr != manifest.platforms.end()) {
            const auto& platform = platformItr->second;
            extension.compileGuards.insert(platform.compileGuards.begin(), platform.compileGuards.end());
        }
        for (auto extensionEnumerationItr : extension.enumerations) {
            const auto& extensionEnumeration = extensionEnumerationItr.second;
            auto coreEnumerationItr = manifest.enumerations.find(extensionEnumerationItr.first);
            assert(coreEnumerationItr != manifest.enumerations.end());
            if (coreEnumerationItr != manifest.enumerations.end()) {
                auto& coreEnumeration = coreEnumerationItr->second;
                if (extension.supported != "disabled") {
                    for (auto extensionEnumerator : extensionEnumeration.enumerators) {
                        extensionEnumerator.compileGuards.insert(extension.compileGuards.begin(), extension.compileGuards.end());
                        coreEnumeration.enumerators.insert(extensionEnumerator);
                    }
                }
            }
        }
        for (const auto& type : extension.types) {
            post_process_extension(extension, type, manifest.handles);
            post_process_extension(extension, type, manifest.enumerations);
            post_process_extension(extension, type, manifest.structures);
        }
        for (const auto& command : extension.commands) {
            post_process_extension(extension, command, manifest.commands);
        }
    }
    std::erase_if(manifest.extensions, [](const auto& itr) { return itr.second.supported == "disabled"; });
}

static void post_process_handles(Manifest& manifest)
{
    for (auto commandItr : manifest.commands) {
        const auto& command = commandItr.second;
        if (1 < command.parameters.size()) {
            auto handleItr = manifest.handles.find(command.target);
            if (handleItr != manifest.handles.end()) {
                auto& handle = handleItr->second;
                switch (command.type) {
                case Command::Type::Create: {
                    handle.createCommands.insert(command.name);
                    for (const auto& parameter : command.parameters) {
                        if (string::contains(parameter.type, "Info")) {
                            handle.createInfos.insert(parameter.unqualifiedType);
                        }
                    }
                } break;
                case Command::Type::Destroy: {
                    handle.destroyCommands.insert(command.name);
                } break;
                default: break;
                }
            }
        }
    }
}

static void post_process_enumerations(Manifest& manifest)
{
    for (auto& enumerationItr : manifest.enumerations) {
        auto& enumeration = enumerationItr.second;
        std::map<std::string, Enumerator> enumerators;
        for (const auto& enumerator : enumeration.enumerators) {
            enumerators.insert({ enumerator.name, enumerator });
        }
        enumeration.enumerators.clear();
        for (auto enumeratorItr : enumerators) {
            auto enumerator = enumeratorItr.second;
            if (enumerator.value.empty() && !enumerator.alias.empty()) {
                auto aliasEnumeratorItr = enumerators.find(enumerator.alias);
                if (aliasEnumeratorItr != enumerators.end()) {
                    enumerator.value = aliasEnumeratorItr->second.value;
                }
            }
            enumeration.enumerators.insert(enumerator);
        }
    }
}

static Enumeration concatenate_enumerations(const Enumeration& lhs, const Enumeration& rhs)
{
    auto result = lhs;
    if (lhs.name == rhs.name) {
        for (const auto& enumerator : rhs.enumerators) {
            result.enumerators.insert(enumerator);
        }
    }
    return result;
}

static void post_process_features(Manifest& manifest)
{
    for (const auto& featureItr : manifest.features) {
        const auto& feature = featureItr.second;
        for (const auto& featureEnumerationItr : feature.enumerations) {
            const auto& featureEnumeration = featureEnumerationItr.second;
            auto enumerationItr = manifest.enumerations.find(featureEnumeration.name);
            assert(enumerationItr != manifest.enumerations.end());
            enumerationItr->second = concatenate_enumerations(enumerationItr->second, featureEnumeration);
        }
    }
}

static void post_process_object_types(Manifest& manifest)
{
    for (auto handleItr : manifest.handles) {
        const auto& handle = handleItr.second;
        if (!handle.vkObjectType.empty()) {
            manifest.vkObjectTypes.insert({ handleItr.first, handle.vkObjectType });
            manifest.vkObjectTypes.insert({ handle.vkObjectType, handleItr.first });
        }
    }
}

static void post_process_structure_types(Manifest& manifest)
{
    for (auto structureItr : manifest.structures) {
        const auto& structure = structureItr.second;
        if (!structure.vkStructureType.empty()) {
            manifest.vkStructureTypes.insert({ structureItr.first, structure.vkStructureType });
            manifest.vkStructureTypes.insert({ structure.vkStructureType, structureItr.first });
        }
    }
}

Manifest::Manifest(const tinyxml2::XMLDocument& xmlDocument)
{
    auto pRegistryXml = xmlDocument.FirstChildElement("registry");
    if (pRegistryXml) {
        process_xml_elements(*pRegistryXml, "platforms", "platform", [&](const auto& xmlElement) { create_api_element(xmlElement, platforms); });
        process_xml_elements(*pRegistryXml, "tags", "tag", [&](const auto& xmlElement) { vendors.insert(get_xml_attribute(xmlElement, "name")); });
        process_xml_elements(*pRegistryXml, "enums", [&](const auto& xmlElement) { create_api_element(xmlElement, enumerations); });
        process_xml_elements(*pRegistryXml, "commands", "command", [&](const auto& xmlElement) { create_api_element(xmlElement, commands); });
        process_xml_elements(*pRegistryXml, "extensions", "extension", [&](const auto& xmlElement) { create_api_element(xmlElement, extensions); });
        process_xml_elements(*pRegistryXml, "feature", [&](const auto& xmlElement) { create_api_element(xmlElement, features); });
        process_xml_elements(*pRegistryXml, "formats", "format", [&](const auto& xmlElement) { create_api_element(xmlElement, formats); });

        Format undefined;
        undefined.name = "VK_FORMAT_UNDEFINED";
        formats.insert({ undefined.name, undefined });

        process_xml_elements(*pRegistryXml, "types", "type",
            [&](const auto& typeXmlElement)
            {
                auto category = get_xml_attribute(typeXmlElement, "category");
                if (category == "handle") {
                    create_api_element(typeXmlElement, handles);
                } else if (category == "struct" || category == "union") {
                    create_api_element(typeXmlElement, structures);
                } else if (category == "enum") {
                    Enumeration enumeration(typeXmlElement);
                    assert(!enumeration.name.empty());
                    if (!enumeration.alias.empty()) {
                        enumerations.insert({ enumeration.name, enumeration });
                    }
                }
            }
        );

        post_process_vendors(vendors, handles);
        post_process_vendors(vendors, enumerations);
        post_process_vendors(vendors, structures);
        post_process_vendors(vendors, commands);
        post_process_vendors(vendors, extensions);
        post_process_extensions(*this);
        post_process_handles(*this);
        post_process_features(*this);
        post_process_object_types(*this);
        post_process_structure_types(*this);
        post_process_api_constants(*this);

        // NOTE : post_process_enumerations() is called twice...this is because aliased
        //  entries may be processed before the "real" entry gets processed.  If this
        //  ever becomes a problem, a collection of all enumerations/enumerators should
        //  be created from this->enumerations, extensions[...]enumerations, and
        //  features[...]enumerations, process those together, then repopulate each.
        post_process_enumerations(*this);
        post_process_enumerations(*this);
    }
}

} // namespace xml
} // namespace gvk
