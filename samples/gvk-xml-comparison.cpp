
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

#include "gvk-xml.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

bool create_xml_manifest(const char* pFilePath, gvk::xml::Manifest* pGvkXmlManifest)
{
    assert(pFilePath);
    assert(pGvkXmlManifest);
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(pFilePath);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        *pGvkXmlManifest = gvk::xml::Manifest(xmlDocument);
        std::cout << "Successfully loaded " << pFilePath << std::endl;
    } else {
        std::cerr << "Failed to load " << pFilePath << " : " << tinyxml2::XMLDocument::ErrorIDToName(xmlResult) << std::endl;
    }
    return xmlResult == tinyxml2::XML_SUCCESS;
}

template <typename ItrType>
std::string get_api_element_name(const ItrType& itr)
{
    if constexpr (std::is_same_v<ItrType, std::string>) {
        return itr;
    } else if constexpr (std::is_same_v<ItrType, gvk::xml::Enumerator>) {
        return itr.name;
    } else {
        return itr.first;
    }
}

template <typename ItrType, typename CollectionType>
void process_api_element(const ItrType& itr, CollectionType& collection)
{
    if constexpr (std::is_same_v<ItrType, std::string> || std::is_same_v<ItrType, gvk::xml::Enumerator>) {
        collection.erase(itr);
    } else {
        collection.erase(itr.first);
    }
}

template <typename CollectionType>
void compare_api_element_collections(const char* pLabel, const CollectionType& lhs, const CollectionType& rhs)
{
    assert(pLabel);
    auto removed = lhs;
    for (const auto& itr : rhs) {
        process_api_element(itr, removed);
    }
    auto added = rhs;
    for (const auto& itr : lhs) {
        process_api_element(itr, added);
    }
    if (!removed.empty() || !added.empty()) {
        std::cout << "    " << pLabel << std::endl;
        for (const auto& itr : removed) {
            std::cout << "        - " << get_api_element_name(itr) << std::endl;
        }
        for (const auto& itr : added) {
            std::cout << "        + " << get_api_element_name(itr) << std::endl;
        }
    }
}

void compare_xml_manifests(const gvk::xml::Manifest& lhs, const gvk::xml::Manifest& rhs)
{
    compare_api_element_collections("Constants", lhs.constants.enumerators, rhs.constants.enumerators);
    compare_api_element_collections("Platforms", lhs.platforms, rhs.platforms);
    compare_api_element_collections("Vendors", lhs.vendors, rhs.vendors);
    compare_api_element_collections("Handles", lhs.handles, rhs.handles);
    compare_api_element_collections("Enumerations", lhs.enumerations, rhs.enumerations);
    compare_api_element_collections("Structures", lhs.structures, rhs.structures);
    compare_api_element_collections("Commands", lhs.commands, rhs.commands);
    compare_api_element_collections("Formats", lhs.formats, rhs.formats);
    compare_api_element_collections("Features", lhs.features, rhs.features);
    compare_api_element_collections("Extensions", lhs.extensions, rhs.extensions);
}

int main(int argc, const char* ppArgv[])
{
    if (2 < argc) {
        std::vector<std::pair<std::string, gvk::xml::Manifest>> gvkXmlManifests;
        gvkXmlManifests.reserve(argc - 1);
        for (int i = 1; i < argc; ++i) {
            gvk::xml::Manifest gvkXmlManifest;
            if (create_xml_manifest(ppArgv[i], &gvkXmlManifest)) {
                gvkXmlManifests.push_back({ ppArgv[i], std::move(gvkXmlManifest) });
                if (1 < i) {
                    std::cout << "--------------------------------------------------------------------------------" << std::endl;
                    std::cout << "Comparing" << std::endl;
                    const auto& lhs = gvkXmlManifests.rbegin()[1];
                    const auto& rhs = gvkXmlManifests.back();
                    std::cout << lhs.first << std::endl;
                    std::cout << rhs.first << std::endl;
                    compare_xml_manifests(lhs.second, rhs.second);
                    std::cout << "--------------------------------------------------------------------------------" << std::endl;
                }
            }
        }
    } else {
        std::cout << "Provide any number of filepaths to different versions of vk.xml to compare sequentially" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "    ./gvk-xml-parser <filepath0/vk.xml> <filepath1/vk.xml> <...>" << std::endl;
    }
    return 0;
}
