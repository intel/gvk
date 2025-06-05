
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

#include "gvk-string.hpp"
#include "gvk-xml.hpp"

#include <cassert>
#include <iostream>
#include <set>
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

template <typename CollectionType>
size_t search_api_element_collection(const char* pLabel, const CollectionType& collection, const std::vector<std::string>& finds)
{
    assert(pLabel);
    std::set<std::string> matches;
    for (const auto& itr : collection) {
        auto name = get_api_element_name(itr);
        for (const auto& find : finds) {
            if (gvk::string::contains(name, find)) {
                matches.insert(name);
            }
        }
    }
    if (!matches.empty()) {
        std::cout << pLabel << " : " << matches.size() << std::endl;
        for (const auto& match : matches) {
            std::cout << "    " << match << std::endl;
        }
    }
    return matches.size();
}

int main(int argc, const char* ppArgv[])
{
    if (2 < argc) {
        gvk::xml::Manifest gvkXmlManifest;
        if (create_xml_manifest(ppArgv[1], &gvkXmlManifest)) {
            std::cout << "Searching for" << std::endl;
            std::vector<std::string> finds;
            for (int i = 2; i < argc; ++i) {
                std::cout << "    " << ppArgv[i] << std::endl;
                finds.push_back(ppArgv[i]);
            }
            auto matchCount = search_api_element_collection("Constants", gvkXmlManifest.constants.enumerators, finds);
            matchCount += search_api_element_collection("Platforms", gvkXmlManifest.platforms, finds);
            matchCount += search_api_element_collection("Vendors", gvkXmlManifest.vendors, finds);
            matchCount += search_api_element_collection("Handles", gvkXmlManifest.handles, finds);
            matchCount += search_api_element_collection("Enumerations", gvkXmlManifest.enumerations, finds);
            matchCount += search_api_element_collection("Structures", gvkXmlManifest.structures, finds);
            matchCount += search_api_element_collection("Commands", gvkXmlManifest.commands, finds);
            matchCount += search_api_element_collection("Formats", gvkXmlManifest.formats, finds);
            matchCount += search_api_element_collection("Features", gvkXmlManifest.features, finds);
            matchCount += search_api_element_collection("Extensions", gvkXmlManifest.extensions, finds);
            std::cout << "Total : " << matchCount << std::endl;
        }
    } else {
        std::cout << "Provide the filepath to the vk.xml to search, and any number of tokens to search for" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "    ./gvk-xml-search <filepath/vk.xml> <token0> <token1> <...>" << std::endl;
    }
    return 0;
}
