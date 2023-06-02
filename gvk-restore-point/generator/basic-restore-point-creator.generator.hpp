
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
#include "gvk-xml.hpp"

namespace gvk {
namespace cppgen {

inline std::string get_create_info_variable_name(const std::string& createInfoName)
{
    auto createInfoVariableName = string::strip_vk(createInfoName);
    createInfoVariableName[0] = string::to_lower(createInfoVariableName[0]);
    return createInfoVariableName;
}

class BasicRestorePointCreatorGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PATH,
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX,
            GVK_RESTORE_POINT_GENERATED_SOURCE_PATH,
            "basic-restore-point-creator"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << "#include \"gvk-reference/handle-id.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/restore-point-info.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info.h\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-enumerations-to-string.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-comparison-operators.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-create-copy.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-deserialization.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-destroy-copy.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-get-stype.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-serialization.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-to-string.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <unordered_set>" << std::endl;
        file << "#include <vector>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "class BasicRestorePointCreator" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        file << "    virtual VkResult create_restore_point(VkInstance instance, const CreateInfo& restorePointInfo);" << std::endl;
        file << std::endl;
        file << "protected:" << std::endl;
        file << "    class DependencyEnumerationInfo final" << std::endl;
        file << "    {" << std::endl;
        file << "    public:" << std::endl;
        file << "        BasicRestorePointCreator* pRestorePointCreator { nullptr };" << std::endl;
        file << "        std::vector<GvkRestorePointObject> dependencies;" << std::endl;
        file << "    };" << std::endl;
        file << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                auto extensionVendor = gvk::cppgen::get_extension_vendor(handle.name);
                auto restoreInfoTypeName = "Gvk" + gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor) + "RestoreInfo" + extensionVendor;
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "    virtual VkResult process_" << handle.name << "(const GvkStateTrackedObject& stateTrackedObject, " << restoreInfoTypeName << "& restoreInfo);" << std::endl;
            }
        }
        file << "    static void process_object(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData);" << std::endl;
        file << "    static void process_dependency(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData);" << std::endl;
        file << "    std::unordered_set<HandleId<uint64_t, uint64_t>> mProcessedHandles;" << std::endl;
        file << "    std::vector<GvkRestorePointObject> mRestorePointObjects;" << std::endl;
        file << "    CreateInfo mRestorePointInfo;" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-restore-point/generated/restore-info.h\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-get-stype.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-serialization.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/restore-info-structure-to-string.hpp\"" << std::endl;
        file << "#include \"gvk-structures/defaults.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <cassert>" << std::endl;
        file << "#include <fstream>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "VkResult BasicRestorePointCreator::create_restore_point(VkInstance instance, const CreateInfo& restorePointInfo)" << std::endl;
        file << "{" << std::endl;
        file << "    (void)instance;" << std::endl;
        file << "    mRestorePointInfo = restorePointInfo;" << std::endl;
        file << "    return VK_SUCCESS;" << std::endl;
        file << "}" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                file << std::endl;
                auto extensionVendor = gvk::cppgen::get_extension_vendor(handle.name);
                auto restoreInfoTypeName = "Gvk" + gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor) + "RestoreInfo" + extensionVendor;
                auto handleMemberName = gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor);
                handleMemberName[0] = gvk::string::to_lower(handleMemberName[0]);
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "VkResult BasicRestorePointCreator::process_" << handle.name << "(const GvkStateTrackedObject& stateTrackedObject, " << restoreInfoTypeName << "& restoreInfo)" << std::endl;
                file << "{" << std::endl;
                file << "    restoreInfo." << handleMemberName << " = (" << handle.name << ")stateTrackedObject.handle;" << std::endl;
                file << "    GvkStateTrackedObjectInfo stateTrackedObjectInfo { };" << std::endl;
                file << "    pfnGvkGetStateTrackedObjectInfo(&stateTrackedObject, &stateTrackedObjectInfo);" << std::endl;
                file << "    restoreInfo.statusFlags = stateTrackedObjectInfo.flags;" << std::endl;
                if (!handle.createInfos.empty()) {
                    file << "    VkStructureType createInfoType { };" << std::endl;
                    file << "    pfnGvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, nullptr);" << std::endl;
                    for (const auto& createInfoName : handle.createInfos) {
                        const auto& createInfoItr = manifest.structures.find(createInfoName);
                        assert(createInfoItr != manifest.structures.end());
                        const auto& createInfo = createInfoItr->second;
                        CompileGuardGenerator createInfoCompileGuard(file, get_inner_scope_compile_guards(handle.compileGuards, createInfo.compileGuards));
                        file << "    " << createInfo.name << " " << get_create_info_variable_name(createInfo.name) << " { };" << std::endl;
                    }
                    file << "    switch (createInfoType) {" << std::endl;
                    for (const auto& createInfoName : handle.createInfos) {
                        const auto& createInfoItr = manifest.structures.find(createInfoName);
                        assert(createInfoItr != manifest.structures.end());
                        const auto& createInfo = createInfoItr->second;
                        CompileGuardGenerator createInfoCompileGuard(file, get_inner_scope_compile_guards(handle.compileGuards, createInfo.compileGuards));
                        file << "    case " << createInfo.vkStructureType << ": {" << std::endl;
                        file << "        pfnGvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&" << get_create_info_variable_name(createInfo.name) << ");" << std::endl;
                        file << "        restoreInfo.p" << string::strip_vk(createInfo.name) << " = &" << get_create_info_variable_name(createInfo.name) << ";" << std::endl;
                        file << "    } break;" << std::endl;
                    }
                    file << "    default: {" << std::endl;
                    file << "        assert(false && \"TODO : Documentation\");" << std::endl;
                    file << "    } break;" << std::endl;
                    file << "    }" << std::endl;
                }
                file << "    GvkStateTrackedObjectEnumerateInfo enumerateInfo { };" << std::endl;
                file << "    enumerateInfo.pfnCallback = process_dependency;" << std::endl;
                file << "    DependencyEnumerationInfo dependencyEnumerationInfo { };" << std::endl;
                file << "    dependencyEnumerationInfo.pRestorePointCreator = this;" << std::endl;
                file << "    enumerateInfo.pUserData = &dependencyEnumerationInfo;" << std::endl;
                file << "    pfnGvkEnumerateStateTrackedObjectDependencies(&stateTrackedObject, &enumerateInfo);" << std::endl;
                file << "    restoreInfo.dependencyCount = (uint32_t)dependencyEnumerationInfo.dependencies.size();" << std::endl;
                file << "    restoreInfo.pDependencies = !dependencyEnumerationInfo.dependencies.empty() ? dependencyEnumerationInfo.dependencies.data() : nullptr;" << std::endl;
                file << "    auto path = mRestorePointInfo.path / \"" << handle.name << "\";" << std::endl;
                file << "    std::filesystem::create_directories(path);" << std::endl;
                file << "    {" << std::endl;
                file << "        auto jsonPath = (path / to_hex_string(stateTrackedObject.handle)).replace_extension(\"json\");" << std::endl;
                file << "        std::ofstream jsonFile(jsonPath);" << std::endl;
                file << "        assert(jsonFile.is_open());" << std::endl;
                file << "        jsonFile << to_string(restoreInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;" << std::endl;
                file << "    }" << std::endl;
                file << "    {" << std::endl;
                file << "        auto infoPath = (path / to_hex_string(stateTrackedObject.handle)).replace_extension(\"info\");" << std::endl;
                file << "        std::ofstream infoFile(infoPath, std::ios::binary);" << std::endl;
                file << "        assert(infoFile.is_open());" << std::endl;
                file << "        serialize(infoFile, restoreInfo);" << std::endl;
                file << "    }" << std::endl;
                file << "    return VK_SUCCESS;" << std::endl;
                file << "}" << std::endl;
            }
        }
        file << std::endl;
        file << "void BasicRestorePointCreator::process_object(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData)" << std::endl;
        file << "{" << std::endl;
        file << "    (void)pInfo;" << std::endl;
        file << "    assert(pStateTrackedObject);" << std::endl;
        file << "    assert(pUserData);" << std::endl;
        file << "    auto pRestorePointCreator = (BasicRestorePointCreator*)pUserData;" << std::endl;
        file << "    if (pRestorePointCreator->mProcessedHandles.insert(HandleId<uint64_t, uint64_t>(pStateTrackedObject->dispatchableHandle, pStateTrackedObject->handle)).second) {" << std::endl;
        file << "        pRestorePointCreator->mRestorePointObjects.push_back(*(GvkRestorePointObject*)pStateTrackedObject);" << std::endl;
        file << "        switch (pStateTrackedObject->type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty() && !handle.vkObjectType.empty()) {
                auto extensionVendor = gvk::cppgen::get_extension_vendor(handle.name);
                auto restoreInfoTypeName = "Gvk" + gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor) + "RestoreInfo" + extensionVendor;
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "        case " << handle.vkObjectType << ": {" << std::endl;
                file << "            auto restoreInfo = get_default<" << restoreInfoTypeName << ">();" << std::endl;
                file << "            pRestorePointCreator->process_" << handle.name << "(*pStateTrackedObject, restoreInfo);" << std::endl;
                file << "        } break;" << std::endl;
            }
        }
        file << "        default: {" << std::endl;
        file << "            assert(false && \"TODO : Documentation\");" << std::endl;
        file << "        } break;" << std::endl;;
        file << "        }" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "void BasicRestorePointCreator::process_dependency(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData)" << std::endl;
        file << "{" << std::endl;
        file << "    (void)pInfo;" << std::endl;
        file << "    assert(pStateTrackedObject);" << std::endl;
        file << "    assert(pUserData);" << std::endl;
        file << "    auto pDependencyEnumerationInfo = (DependencyEnumerationInfo*)pUserData;" << std::endl;
        file << "    assert(pDependencyEnumerationInfo->pRestorePointCreator);" << std::endl;
        file << "    auto pRestorePointCreator = pDependencyEnumerationInfo->pRestorePointCreator;" << std::endl;
        file << "    process_object(pStateTrackedObject, nullptr, pRestorePointCreator);" << std::endl;
        file << "    GvkRestorePointObject dependency { };" << std::endl;
        file << "    dependency.objectType = pStateTrackedObject->type;" << std::endl;
        file << "    dependency.handle = pStateTrackedObject->handle;" << std::endl;
        file << "    dependency.dispatchableHandle = pStateTrackedObject->dispatchableHandle;" << std::endl;
        file << "    pDependencyEnumerationInfo->dependencies.push_back(dependency);" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
