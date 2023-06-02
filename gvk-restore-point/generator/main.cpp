
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

#include "gvk-cppgen.hpp"
#include "gvk-string.hpp"
#include "gvk-xml.hpp"
#include "apply-command-buffer-restore-point.generator.hpp"
#include "basic-restore-point-applier.generator.hpp"
#include "basic-restore-point-creator.generator.hpp"
#include "create-command-buffer-restore-point.generator.hpp"

std::vector<gvk::xml::Structure> get_restore_info_structures(const gvk::xml::Manifest& manifest)
{
    std::vector<gvk::xml::Structure> structures;

    gvk::xml::Structure gvkRestorePointObject;
    gvkRestorePointObject.name = "GvkRestorePointObject";
    gvkRestorePointObject.members.push_back(gvk::cppgen::create_parameter("VkObjectType", "objectType"));
    gvkRestorePointObject.members.push_back(gvk::cppgen::create_parameter("uint64_t", "handle"));
    gvkRestorePointObject.members.push_back(gvk::cppgen::create_parameter("uint64_t", "dispatchableHandle"));
    structures.push_back(gvkRestorePointObject);

    gvk::xml::Structure gvkRestorePointManifest;
    gvkRestorePointManifest.name = "GvkRestorePointManifest";
    gvk::cppgen::add_array_members_to_structure("GvkRestorePointObject", "objectCount", "pObjects", gvkRestorePointManifest);
    structures.push_back(gvkRestorePointManifest);

    gvk::xml::Structure gvkRestoreInfoBaseStructure;
    gvkRestoreInfoBaseStructure.name = "GvkRestoreInfoBaseStructure";
    gvkRestoreInfoBaseStructure.members.push_back(gvk::cppgen::create_parameter("GvkRestoreInfoStructureType", "sType"));
    structures.push_back(gvkRestoreInfoBaseStructure);

    for (const auto& handleItr : manifest.handles) {
        const auto& handle = handleItr.second;
        if (handle.alias.empty()) {
            gvk::xml::Structure structure;
            auto extensionVendor = gvk::cppgen::get_extension_vendor(handle.name);
            structure.name = "Gvk" + gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor) + "RestoreInfo" + extensionVendor;
            structure.vendor = handle.vendor;
            structure.extension = handle.extension;
            structure.compileGuards = handle.compileGuards;
            structure.vkStructureType = "GVK_STRUCTURE_TYPE";
            for (const auto& token : gvk::string::split_camel_case(gvk::string::strip_vk(handle.name))) {
                structure.vkStructureType += "_" + gvk::string::to_upper(token);
            }
            structure.vkStructureType += "_RESTORE_INFO";
            structure.members.push_back(gvk::cppgen::create_parameter("GvkRestoreInfoStructureType", "sType"));
            structure.members.push_back(gvk::cppgen::create_parameter("GvkRestorePointObjectStatusFlags", "statusFlags"));
            auto handleName = gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor);
            handleName[0] = gvk::string::to_lower(handleName[0]);
            structure.members.push_back(gvk::cppgen::create_parameter(handle.name, handleName));

            gvk::xml::Parameter nameParameter;
            nameParameter.type = "const char*";
            nameParameter.unqualifiedType = "char";
            nameParameter.name = "pName";
            nameParameter.flags = gvk::xml::Const | gvk::xml::Pointer | gvk::xml::Dynamic | gvk::xml::Array | gvk::xml::String;
            structure.members.push_back(nameParameter);

            for (const auto& createInfoName : handle.createInfos) {
                gvk::xml::Parameter createInfoMember;
                createInfoMember.type = "const " + createInfoName + "*";
                createInfoMember.unqualifiedType = createInfoName;
                createInfoMember.name = "p" + gvk::string::strip_vk(createInfoName);
                auto createInfoItr = manifest.structures.find(createInfoName);
                assert(createInfoItr != manifest.structures.end());
                createInfoMember.compileGuards = createInfoItr->second.compileGuards;
                createInfoMember.flags = gvk::xml::Const | gvk::xml::Pointer;
                structure.members.push_back(createInfoMember);
            }

            if (handle.name == "VkInstance") {
                gvk::cppgen::add_array_members_to_structure("VkPhysicalDevice", "physicalDeviceCount", "pPhysicalDevices", structure);
            }
            if (handle.name == "VkPhysicalDevice") {
                structure.members.push_back(gvk::cppgen::create_parameter("VkPhysicalDeviceProperties", "physicalDeviceProperties"));
                structure.members.push_back(gvk::cppgen::create_parameter("VkPhysicalDeviceFeatures", "physicalDeviceFeatures"));
                structure.members.push_back(gvk::cppgen::create_parameter("VkPhysicalDeviceMemoryProperties", "physicalDeviceMemoryProperties"));
                gvk::cppgen::add_array_members_to_structure("VkQueueFamilyProperties", "queueFamilyPropertyCount", "pQueueFamilyProperties", structure);
            }
            if (handle.name == "VkDevice") {
                gvk::cppgen::add_array_members_to_structure("VkQueue", "queueCount", "pQueues", structure);
            }
            if (handle.name == "VkQueue") {
                structure.members.push_back(gvk::cppgen::create_parameter("VkDeviceQueueCreateInfo", "deviceQueueCreateInfo"));
            }
            if (handle.name == "VkDeviceMemory") {
                gvk::xml::Structure mappedMemoryInfo;
                mappedMemoryInfo.name = "GvkMappedMemoryInfo";
                mappedMemoryInfo.members.push_back(gvk::cppgen::create_parameter("VkDeviceSize", "offset"));
                mappedMemoryInfo.members.push_back(gvk::cppgen::create_parameter("VkDeviceSize", "size"));
                mappedMemoryInfo.members.push_back(gvk::cppgen::create_parameter("VkMemoryMapFlags", "flags"));
                mappedMemoryInfo.members.push_back(gvk::cppgen::create_parameter("uint64_t", "dataHandle"));
                structures.push_back(mappedMemoryInfo);
                structure.members.push_back(gvk::cppgen::create_parameter("GvkMappedMemoryInfo", "mappedMemoryInfo"));
                gvk::cppgen::add_array_members_to_structure("VkBindBufferMemoryInfo", "bufferBindInfoCount", "pBufferBindInfos", structure);
                gvk::cppgen::add_array_members_to_structure("VkBindImageMemoryInfo", "imageBindInfoCount", "pImageBindInfos", structure);
            }
            if (handle.name == "VkEvent") {
                structure.members.push_back(gvk::cppgen::create_parameter("VkResult", "status"));
            }
            if (handle.name == "VkFence") {
                structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "signaled"));
            }
            if (handle.name == "VkSemaphore") {
                structure.members.push_back(gvk::cppgen::create_parameter("VkBool32", "signaled"));
            }
            if (handle.name == "VkBuffer") {
                gvk::cppgen::add_array_members_to_structure("VkBindBufferMemoryInfo", "memoryBindInfoCount", "pMemoryBindInfos", structure);
            }
            if (handle.name == "VkImage") {
                gvk::cppgen::add_array_members_to_structure("VkBindImageMemoryInfo", "memoryBindInfoCount", "pMemoryBindInfos", structure);
                gvk::cppgen::add_array_members_to_structure("VkImageLayout", "imageSubresourceCount", "pImageLayouts", structure);
            }
            if (handle.name == "VkDescriptorSet") {
                gvk::cppgen::add_array_members_to_structure("VkWriteDescriptorSet", "descriptorWriteCount", "pDescriptorWrites", structure);
            }
            if (handle.name == "VkSurfaceKHR") {
                structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "width"));
                structure.members.push_back(gvk::cppgen::create_parameter("uint32_t", "height"));
            }
            if (handle.name == "VkSwapchainKHR") {
                gvk::xml::Structure swapchainImageRestoreInfo;
                swapchainImageRestoreInfo.name = "GvkSwapchainImageRestoreInfo";
                gvk::xml::Parameter imageParameter;
                swapchainImageRestoreInfo.members.push_back(gvk::cppgen::create_parameter("VkImage", "image"));
                swapchainImageRestoreInfo.members.push_back(gvk::cppgen::create_parameter("VkBool32", "acquired"));
                swapchainImageRestoreInfo.members.push_back(gvk::cppgen::create_parameter("VkFence", "fence"));
                swapchainImageRestoreInfo.members.push_back(gvk::cppgen::create_parameter("VkSemaphore", "semaphore"));
                structures.push_back(swapchainImageRestoreInfo);
                gvk::cppgen::add_array_members_to_structure("GvkSwapchainImageRestoreInfo", "imageCount", "pImages", structure);
            }
            gvk::cppgen::add_array_members_to_structure("GvkRestorePointObject", "dependencyCount", "pDependencies", structure);
            structures.push_back(structure);
        }
    }
    return structures;
}

gvk::xml::Enumeration get_restore_info_structure_type_enumeration(const gvk::xml::Manifest& manifest)
{
    std::set<std::string> sTypeValues;
    gvk::xml::Enumeration enumeration;
    enumeration.name = "GvkRestoreInfoStructureType";

    gvk::xml::Enumerator enumerator;
    enumerator.name = "GVK_STRUCTURE_TYPE_UNDEFINED";
    enumerator.value = "0";
    sTypeValues.insert(enumerator.value);
    enumeration.enumerators.insert(enumerator);

    for (const auto& commandStructure : get_restore_info_structures(manifest)) {
        if (!commandStructure.vkStructureType.empty()) {
            enumerator.name = commandStructure.vkStructureType;
            enumerator.value = gvk::to_hex_string(gvk::string::hash(enumerator.name));
            enumerator.compileGuards = commandStructure.compileGuards;
            if (!sTypeValues.insert(enumerator.value).second) {
                enumerator.value += " GVK_RESTORE_INFO_STRUCTURE_TYPE collision!";
            }
            enumeration.enumerators.insert(enumerator);
        }
    }
    return enumeration;
}

gvk::xml::Enumeration get_restore_point_object_status_bits_enumeration()
{
    gvk::xml::Enumeration enumeration;
    enumeration.name = "GvkRestorePointObjectStatusFlagBits";
    enumeration.isBitmask = true;

    gvk::xml::Enumerator enumerator;
    enumerator.name = "GVK_RESTORE_POINT_OBJECT_STATUS_ACTIVE_BIT";
    enumerator.value = "0x00000001";
    enumeration.enumerators.insert(enumerator);

    enumerator.name = "GVK_RESTORE_POINT_OBJECT_STATUS_DESTROYED_BIT";
    enumerator.value = "0x00000002";
    enumeration.enumerators.insert(enumerator);

    return enumeration;
}

int main(int, const char* [])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        gvk::cppgen::ApiElementCollectionInfo apiElements { };
        apiElements.name = "restore-info";
        apiElements.headerGuard = "gvk_restore_info_h";
        apiElements.includePath = GVK_RESTORE_POINT_GENERATED_INCLUDE_PATH;
        apiElements.includePrefix = GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX;
        apiElements.sourcePath = GVK_RESTORE_POINT_GENERATED_SOURCE_PATH;
        apiElements.enumerations.push_back(get_restore_info_structure_type_enumeration(manifest));
        apiElements.enumerations.push_back(get_restore_point_object_status_bits_enumeration());
        apiElements.structures = get_restore_info_structures(manifest);
        apiElements.headerIncludes = {
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX "restore-info.h",
        };
        apiElements.sourceIncludes = {
            "gvk-structures.hpp",
        };
        gvk::cppgen::ApiElementCollectionDeclarationGenerator::generate(apiElements);
        gvk::cppgen::EnumerationToStringGenerator::generate(apiElements);
        gvk::cppgen::StructureCerealizationGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureComparisonOperatorsGenerator::generate(apiElements);
        gvk::cppgen::StructureCreateCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDecerealizationGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureDeserializationGenerator::generate(apiElements);
        gvk::cppgen::StructureDestroyCopyGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureGetSTypeGenerator::generate(apiElements);
        gvk::cppgen::StructureMakeTupleGenerator::generate(manifest, apiElements);
        gvk::cppgen::StructureSerializationGenerator::generate(apiElements);
        apiElements.manuallyImplemented.insert("GvkRestorePointObject");
        gvk::cppgen::StructureToStringGenerator::generate(manifest, apiElements);

        gvk::cppgen::ApplyCommandBufferRestorePointGenerator::generate(manifest);
        gvk::cppgen::BasicRestorePointApplierGenerator::generate(manifest);
        gvk::cppgen::BasicRestorePointCreatorGenerator::generate(manifest);
        gvk::cppgen::CreateCommandBufferRestorePointGenerator::generate(manifest);
    }
    return 0;
}
