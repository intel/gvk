
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

class BasicCreatorGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PATH,
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX,
            GVK_RESTORE_POINT_GENERATED_SOURCE_PATH,
            "basic-creator"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static std::string get_restore_info_type_name(const std::string& handleName)
    {
        auto extensionVendor = gvk::cppgen::get_extension_vendor(handleName);
        return "Gvk" + gvk::string::remove(gvk::string::strip_vk(handleName), extensionVendor) + "RestoreInfo" + extensionVendor;
    }

    static std::string get_create_info_variable_name(const std::string& createInfoTypeName)
    {
        auto createInfoVariableName = string::strip_vk(createInfoTypeName);
        createInfoVariableName[0] = string::to_lower(createInfoVariableName[0]);
        return createInfoVariableName;
    }

    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << "#include \"gvk-dispatch-table.hpp\"" << std::endl;
        file << "#include \"gvk-reference/handle-id.hpp\"" << std::endl;
        file << "#include \"gvk-restore-info.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/utilities.hpp\"" << std::endl;
        file << "#include \"VK_LAYER_INTEL_gvk_state_tracker.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <unordered_set>" << std::endl;
        file << "#include <vector>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "class BasicCreator" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        file << "    BasicCreator() = default;" << std::endl;
        file << "    virtual ~BasicCreator() = 0;" << std::endl;
        file << "    virtual VkResult create_restore_point(const CreateInfo& createInfo) = 0;" << std::endl;
        file << "protected:" << std::endl;
        file << "    static void process_object(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData);" << std::endl;
        file << "    static void process_dependency(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData);" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "    virtual VkResult process_" << handle.name << "(" << get_restore_info_type_name(handle.name) << "& restoreInfo); " << std::endl;
            }
        }
        file << "    std::unordered_set<HandleId<uint64_t, uint64_t>> mProcessedHandles;" << std::endl;
        file << "    std::vector<GvkRestorePointObject> mRestorePointObjects;" << std::endl;
        file << "    CreateInfo mCreateInfo{ };" << std::endl;
        file << "    VkResult mResult { VK_ERROR_INITIALIZATION_FAILED };" << std::endl;
        file << "private:" << std::endl;
        file << "    class DependencyEnumerationInfo final" << std::endl;
        file << "    {" << std::endl;
        file << "    public:" << std::endl;
        file << "        BasicCreator* pCreator { nullptr };" << std::endl;
        file << "        std::vector<GvkRestorePointObject> dependencies;" << std::endl;
        file << "    };" << std::endl;
        file << "    BasicCreator(const BasicCreator&) = delete;" << std::endl;
        file << "    BasicCreator& operator=(const BasicCreator&) = delete;" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        (void)manifest;
        file << "#include \"gvk-structures/defaults.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/layer.hpp\"" << std::endl;
        file << "#include \"VK_LAYER_INTEL_gvk_state_tracker.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "BasicCreator::~BasicCreator()" << std::endl;
        file << "{" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "void BasicCreator::process_object(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData)" << std::endl;
        file << "{" << std::endl;
        file << "    (void)pInfo;" << std::endl;
        file << "    assert(pStateTrackedObject);" << std::endl;
        file << "    assert(pUserData);" << std::endl;
        file << "    auto pCreator = (BasicCreator*)pUserData;" << std::endl;
        file << "    gvk_result_scope_begin(VK_SUCCESS) {" << std::endl;
        file << "        if (pCreator->mProcessedHandles.insert(HandleId<uint64_t, uint64_t>(pStateTrackedObject->dispatchableHandle, pStateTrackedObject->handle)).second) {" << std::endl;
        file << "            pCreator->mRestorePointObjects.push_back(*(GvkRestorePointObject*)pStateTrackedObject);" << std::endl;
        file << "            GvkStateTrackedObjectInfo stateTrackedObjectInfo { };" << std::endl;
        file << "            gvkGetStateTrackedObjectInfo(pStateTrackedObject, &stateTrackedObjectInfo);" << std::endl;
        file << "            DependencyEnumerationInfo dependencyEnumerationInfo { };" << std::endl;
        file << "            dependencyEnumerationInfo.pCreator = pCreator;" << std::endl;
        file << "            GvkStateTrackedObjectEnumerateInfo enumerateInfo { };" << std::endl;
        file << "            enumerateInfo.pfnCallback = process_dependency;" << std::endl;
        file << "            enumerateInfo.pUserData = &dependencyEnumerationInfo;" << std::endl;
        file << "            gvkEnumerateStateTrackedObjectDependencies(pStateTrackedObject, &enumerateInfo);" << std::endl;
        file << "            switch (pStateTrackedObject->type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "            case " << handle.vkObjectType << ": {" << std::endl;
                file << "                auto restoreInfo = get_default<" << get_restore_info_type_name(handle.name) << ">();" << std::endl;
                file << "                restoreInfo.flags = stateTrackedObjectInfo.flags;" << std::endl;
                file << "                restoreInfo.handle = (" << handle.name << ")pStateTrackedObject->handle;" << std::endl;
                file << "                restoreInfo.pName = stateTrackedObjectInfo.pName;" << std::endl;
                file << "                restoreInfo.dependencyCount = (uint32_t)dependencyEnumerationInfo.dependencies.size();" << std::endl;
                file << "                restoreInfo.pDependencies = !dependencyEnumerationInfo.dependencies.empty() ? dependencyEnumerationInfo.dependencies.data() : nullptr;" << std::endl;
                if (!handle.createInfos.empty()) {
                    file << "                VkStructureType createInfoType { };" << std::endl;
                    file << "                gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoType, nullptr);" << std::endl;
                    for (const auto& createInfoType : handle.createInfos) {
                        const auto& createInfoItr = manifest.structures.find(createInfoType);
                        assert(createInfoItr != manifest.structures.end());
                        const auto& createInfo = createInfoItr->second;
                        CompileGuardGenerator createInfoCompileGuard(file, get_inner_scope_compile_guards(handle.compileGuards, createInfo.compileGuards));
                        file << "                " << createInfo.name << " " << get_create_info_variable_name(createInfo.name) << " { };" << std::endl;
                    }
                    file << "                switch (createInfoType) {" << std::endl;
                    for (const auto& createInfoType : handle.createInfos) {
                        const auto& createInfoItr = manifest.structures.find(createInfoType);
                        assert(createInfoItr != manifest.structures.end());
                        const auto& createInfo = createInfoItr->second;
                        CompileGuardGenerator createInfoCompileGuard(file, get_inner_scope_compile_guards(handle.compileGuards, createInfo.compileGuards));
                        file << "                case " << createInfo.vkStructureType << ": {" << std::endl;
                        file << "                    gvkGetStateTrackedObjectCreateInfo(pStateTrackedObject, &createInfoType, (VkBaseOutStructure*)&" << get_create_info_variable_name(createInfo.name) << ");" << std::endl;
                        file << "                    restoreInfo.p" << string::strip_vk(createInfo.name) << " = &" << get_create_info_variable_name(createInfo.name) << ";" << std::endl;
                        file << "                } break;" << std::endl;
                    }
                    file << "                default: {" << std::endl;
                    file << "                    gvk_result(VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
                    file << "                } break;" << std::endl;
                    file << "                }" << std::endl;
                }
                file << "                gvk_result(pCreator->process_" << handle.name << "(restoreInfo));" << std::endl;
                file << "            } break;" << std::endl;
            }
        }
        file << "            default: {" << std::endl;
        file << "                gvk_result(VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "            } break;" << std::endl;
        file << "            }" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    pCreator->mResult = gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "void BasicCreator::process_dependency(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData)" << std::endl;
        file << "{" << std::endl;
        file << "    (void)pInfo;" << std::endl;
        file << "    assert(pStateTrackedObject);" << std::endl;
        file << "    assert(pUserData);" << std::endl;
        file << "    auto pDependencyEnumerationInfo = (DependencyEnumerationInfo*)pUserData;" << std::endl;
        file << "    assert(pDependencyEnumerationInfo->pCreator);" << std::endl;
        file << "    process_object(pStateTrackedObject, nullptr, pDependencyEnumerationInfo->pCreator);" << std::endl;
        file << "    GvkRestorePointObject dependency { };" << std::endl;
        file << "    dependency.type = pStateTrackedObject->type;" << std::endl;
        file << "    dependency.handle = pStateTrackedObject->handle;" << std::endl;
        file << "    dependency.dispatchableHandle = pStateTrackedObject->dispatchableHandle;" << std::endl;
        file << "    pDependencyEnumerationInfo->dependencies.push_back(dependency);" << std::endl;
        file << "}" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                file << std::endl;
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "VkResult BasicCreator::process_" << handle.name << "(" << get_restore_info_type_name(handle.name) << "& restoreInfo)" << std::endl;
                file << "{" << std::endl;
                file << "    return write_object_restore_info(mCreateInfo, \"" << handle.name << "\", to_hex_string(restoreInfo.handle), restoreInfo);" << std::endl;
                file << "}" << std::endl;
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
