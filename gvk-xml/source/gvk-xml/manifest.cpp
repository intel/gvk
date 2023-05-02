
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

#include "gvk-xml/manifest.hpp"
#include "tinyxml2-utilities.hpp"

#include <cassert>
#include <map>

namespace gvk {
namespace xml {

template <typename ApiElementType>
static void create_api_element(const std::string& api, const tinyxml2::XMLElement& xmlElement, std::map<std::string, ApiElementType>& apiElements)
{
    ApiElementType apiElement(xmlElement);
    assert(!apiElement.apis.empty());
    assert(!apiElement.name.empty());
    if (apiElement.apis.count(api)) {
        apiElements[apiElement.name] = apiElement;
    }
}

template <typename FeatureType>
static void create_feature(const std::string& api, const tinyxml2::XMLElement& xmlElement, std::map<std::string, FeatureType>& features)
{
    FeatureType feature(api, xmlElement);
    assert(!feature.apis.empty());
    assert(!feature.name.empty());
    if (feature.apis.count(api)) {
        features[feature.name] = feature;
    }
}

static void add_requirements_to_manifest(const std::string& api, const Manifest& all, const Feature& feature, Manifest& manifest)
{
    if (api_enabled(api, feature.apis)) {
        // Copy required types from Manifest `all` to `manifest`
        for (const auto& type : feature.types) {
            const auto& handleItr = all.handles.find(type);
            if (handleItr != all.handles.end() && api_enabled(api, handleItr->second.apis)) {
                auto handle = handleItr->second;
                handle.compileGuards.insert(feature.compileGuards.begin(), feature.compileGuards.end());
                manifest.handles[handle.name] = handle;
            } else {
                const auto& structureItr = all.structures.find(type);
                if (structureItr != all.structures.end() && api_enabled(api, structureItr->second.apis)) {
                    auto structure = structureItr->second;
                    structure.compileGuards.insert(feature.compileGuards.begin(), feature.compileGuards.end());
                    manifest.structures[structure.name] = structure;
                } else {
                    const auto& allEnumerationItr = all.enumerations.find(type);
                    if (allEnumerationItr != all.enumerations.end() && api_enabled(api, allEnumerationItr->second.apis)) {
                        const auto& manifestEnumerationItr = manifest.enumerations.find(type);
                        auto enumeration = manifestEnumerationItr != manifest.enumerations.end() ? manifestEnumerationItr->second : allEnumerationItr->second;
                        const auto& enumerators = allEnumerationItr->second.enumerators;
                        enumeration.enumerators.insert(enumerators.begin(), enumerators.end());
                        enumeration.compileGuards.insert(feature.compileGuards.begin(), feature.compileGuards.end());
                        manifest.enumerations[enumeration.name] = enumeration;
                    }
                }
            }
        }
        // Copy required Enumerators from Manifest `all` to `manifest`
        for (const auto& featureEnumerationItr : feature.enumerations) {
            const auto& featureEnumeration = featureEnumerationItr.second;
            const auto& manifestEnumerationItr = manifest.enumerations.find(featureEnumeration.name);
            const auto& allEnumerationItr = all.enumerations.find(featureEnumeration.name);
            assert(allEnumerationItr != all.enumerations.end());
            auto enumeration = manifestEnumerationItr != manifest.enumerations.end() ? manifestEnumerationItr->second : allEnumerationItr->second;
            for (auto enumerator : featureEnumeration.enumerators) {
                enumerator.compileGuards.insert(feature.compileGuards.begin(), feature.compileGuards.end());
                enumeration.enumerators.insert(enumerator);
            }
            manifest.enumerations[enumeration.name] = enumeration;
        }
        // Copy required Commands from Manifest `all` to `manifest`
        for (const auto& commandName : feature.commands) {
            const auto& commandItr = all.commands.find(commandName);
            assert(commandItr != all.commands.end());
            auto command = commandItr->second;
            command.compileGuards.insert(feature.compileGuards.begin(), feature.compileGuards.end());
            manifest.commands[command.name] = command;
        }
    }
}

static void add_features_to_manifest(const std::string& api, const Manifest& all, Manifest& manifest)
{
    for (const auto& featureItr : all.features) {
        const auto& feature = featureItr.second;
        if (api_enabled(api, feature.apis)) {
            manifest.features[feature.name] = feature;
            add_requirements_to_manifest(api, all, feature, manifest);
        }
    }
}

static void add_extensions_to_manifest(const std::string& api, const Manifest& all, Manifest& manifest)
{
    for (const auto& extensionItr : all.extensions) {
        auto extension = extensionItr.second;
        if (api_enabled(api, extension.apis)) {
            auto platformItr = manifest.platforms.find(extension.platform);
            if (platformItr != manifest.platforms.end()) {
                const auto& platform = platformItr->second;
                extension.compileGuards.insert(platform.compileGuards.begin(), platform.compileGuards.end());
            }
            manifest.extensions[extension.name] = extension;
            add_requirements_to_manifest(api, all, extension, manifest);
        }
    }
}

template <typename ApiElementCollectionType>
static void post_process_vendors(const std::set<std::string>& vendors, ApiElementCollectionType& apiElements)
{
    // Populate ApiElement `vendor`
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

static void post_process_api_constants(const Manifest& all, Manifest& manifest)
{
    // Populate `constants`
    auto apiConstantsItr = all.enumerations.find("API Constants");
    assert(apiConstantsItr != all.enumerations.end());
    manifest.constants = apiConstantsItr->second;
}

static void post_process_enumerations(Manifest& manifest)
{
    // Populate aliased Enumerator values from the "real" Enumerator values
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

static void post_process_formats(Manifest& manifest)
{
    // There's no entry for VK_FORMAT_UNDEFINED in the <formats><formats/> xml
    //  element...Format `undefined` is added so that `formats` has an entry for
    //  every VkFormat Enumerator.
    Format undefined;
    undefined.name = "VK_FORMAT_UNDEFINED";
    manifest.formats[undefined.name] = undefined;
}

static void post_process_handles(Manifest& manifest)
{
    // Set create/destroy Commands
    for (const auto& commandItr : manifest.commands) {
        const auto& command = commandItr.second;
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
            default: {
            } break;
            }
        }
    }
    // Set parent/child Handles
    for (const auto& handleItr : manifest.handles) {
        for (const auto& parent : handleItr.second.parents) {
            auto parentHandleItr = manifest.handles.find(parent);
            assert(parentHandleItr != manifest.handles.end());
            parentHandleItr->second.children.insert(handleItr.second.name);
        }
    }
}

static void post_process_commands(Manifest& manifest)
{
    // Populate aliased Command info from the "real" Command
    for (auto& commandItr : manifest.commands) {
        auto& command = commandItr.second;
        if (!command.alias.empty()) {
            const auto& aliasItr = manifest.commands.find(command.alias);
            assert(aliasItr != manifest.commands.end());
            auto name = command.name;
            auto alias = command.alias;
            auto extension = command.extension;
            command = aliasItr->second;
            command.name = name;
            command.alias = alias;
            command.extension = extension;
        }
    }
}

static void post_process_object_types(Manifest& manifest)
{
    // Populate `vkObjectTypes`
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
    // Populate `vkStructureTypes`
    for (auto structureItr : manifest.structures) {
        const auto& structure = structureItr.second;
        if (!structure.vkStructureType.empty()) {
            manifest.vkStructureTypes.insert({ structureItr.first, structure.vkStructureType });
            manifest.vkStructureTypes.insert({ structure.vkStructureType, structureItr.first });
        }
    }
}

Manifest::Manifest(const tinyxml2::XMLDocument& xmlDocument, const std::string& api)
{
    auto pRegistryXml = xmlDocument.FirstChildElement("registry");
    if (pRegistryXml) {
        // Parse the xml and add everything to Manifest `all`.  Manifest 'all' contains
        //  all Features and Extensions regardless of their API or support status.  It
        //  will be used to lookup and copy ApiElements from enabled Features and
        //  Extensions.
        Manifest all;
        process_xml_elements(*pRegistryXml, "platforms", "platform", [&](const auto& xmlElement) { create_api_element(api, xmlElement, all.platforms); });
        process_xml_elements(*pRegistryXml, "tags", "tag", [&](const auto& xmlElement) { all.vendors.insert(get_xml_attribute(xmlElement, "name")); });
        process_xml_elements(*pRegistryXml, "enums", [&](const auto& xmlElement) { create_api_element(api, xmlElement, all.enumerations); });
        process_xml_elements(*pRegistryXml, "formats", "format", [&](const auto& xmlElement) { create_api_element(api, xmlElement, all.formats); });
        process_xml_elements(*pRegistryXml, "commands", "command", [&](const auto& xmlElement) { create_api_element(api, xmlElement, all.commands); });
        process_xml_elements(*pRegistryXml, "feature", [&](const auto& xmlElement) { create_feature(api, xmlElement, all.features); });
        process_xml_elements(*pRegistryXml, "extensions", "extension", [&](const auto& xmlElement) { create_feature(api, xmlElement, all.extensions); });
        process_xml_elements(*pRegistryXml, "types", "type",
            [&](const auto& typeXmlElement)
            {
                auto category = get_xml_attribute(typeXmlElement, "category");
                if (category == "handle") {
                    create_api_element(api, typeXmlElement, all.handles);
                } else if (category == "struct" || category == "union") {
                    create_api_element(api, typeXmlElement, all.structures);
                } else if (category == "enum") {
                    Enumeration enumeration(typeXmlElement);
                    assert(!enumeration.name.empty());
                    if (!enumeration.alias.empty()) {
                        all.enumerations.insert({ enumeration.name, enumeration });
                    }
                }
            }
        );

        // Copy `platforms`, `vendors`, amd `formats from Manifest `all` to `this`
        platforms = all.platforms;
        vendors = all.vendors;
        formats = all.formats;

        // Copy enabled Features and Extensions from Manifest `all` to `this`.  As
        //  Features and Extensions are added, all of the ApiElements required for
        //  each is added...this ensures that every ApiElement required to support the
        //  desired feature/extension set is present, and that ApiElements that aren't
        //  required aren't added.
        add_features_to_manifest(api, all, *this);
        add_extensions_to_manifest(api, all, *this);

        // After the Manifest is populated, post process ApiElements.  This includes
        //  setting create/destroy Commands for Handles, setting aliased ApiElements,
        //  etc.
        post_process_vendors(vendors, handles);
        post_process_vendors(vendors, enumerations);
        post_process_vendors(vendors, structures);
        post_process_vendors(vendors, commands);
        post_process_vendors(vendors, extensions);
        post_process_api_constants(all, *this);
        post_process_formats(*this);
        post_process_handles(*this);
        post_process_commands(*this);
        post_process_object_types(*this);
        post_process_structure_types(*this);

        // NOTE : post_process_enumerations() is called twice...this is because aliased
        //  entries can be processed before the "real" entry gets processed.  If this
        //  ever becomes a problem, a collection of all Enumerations/Enumerators should
        //  be created from this->enumerations, this->extensions[...]enumerations, and
        //  this->features[...]enumerations...process, then repopulate.
        post_process_enumerations(*this);
        post_process_enumerations(*this);
    }
}

} // namespace xml
} // namespace gvk
