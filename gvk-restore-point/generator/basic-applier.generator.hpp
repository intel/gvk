
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

class BasicApplierGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PATH,
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX,
            GVK_RESTORE_POINT_GENERATED_SOURCE_PATH,
            "basic-applier"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static bool pure_virtual(const std::string& handleName)
    {
        static const std::set<std::string> scPureVirtual{
            // "VkInstance",
            // "VkDevice",
            // "VkPipeline",
            // "VkCommandBuffer",
            // "VkDescriptorSet",
            // "VkDisplayModeKHR",
            // "VkShaderEXT",
            // "VkSurfaceKHR",
            "VkSwapchainKHR",
        };
        return scPureVirtual.count(handleName);
    }

    static std::string strip_vk_extension_vendor(const std::string& handleName)
    {
        return gvk::string::remove(gvk::string::strip_vk(handleName), gvk::cppgen::get_extension_vendor(handleName));
    }

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
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << "#include \"gvk-dispatch-table.hpp\"" << std::endl;
        file << "#include \"gvk-reference/handle-id.hpp\"" << std::endl;
        file << "#include \"gvk-restore-info.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/utilities.hpp\"" << std::endl;
        file << "#include \"VK_LAYER_INTEL_gvk_state_tracker.hpp\"" << std::endl;
        file << std::endl;
        file << "#include <map>" << std::endl;
        file << "#include <set>" << std::endl;
        file << "#include <vector>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "class BasicApplier" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        file << "    BasicApplier() = default;" << std::endl;
        file << "    virtual ~BasicApplier() = 0;" << std::endl;
        file << "    virtual VkResult apply_restore_point(const ApplyInfo& applyInfo) = 0;" << std::endl;
        file << "    VkResult register_restored_object(const GvkRestorePointObject& capturedObject, const GvkRestorePointObject& restoredObject);" << std::endl;
        file << "    virtual VkResult register_restored_object_ex(const GvkRestorePointObject& capturedObject, const GvkRestorePointObject& restoredObject) = 0;" << std::endl;
        file << "    GvkRestorePointObject get_restored_object(const GvkRestorePointObject& restorePointObject);" << std::endl;
        file << "protected:" << std::endl;
        file << "    virtual VkResult restore_object(const GvkRestorePointObject& restorePointObject);" << std::endl;
        file << "    virtual VkResult restore_object_state(const GvkRestorePointObject& restorePointObject);" << std::endl;
        file << "    virtual VkResult restore_object_name(const GvkRestorePointObject& restorePointObject);" << std::endl;
        file << "    virtual VkResult restore_object_name(const GvkRestorePointObject& restorePointObject, uint32_t dependencyCount, const GvkRestorePointObject* pDependencies, const char* pName) = 0;" << std::endl;
        file << "    VkResult process_object(const GvkRestorePointObject& restorePointObject);" << std::endl;
        file << "    VkResult process_dependencies(uint32_t dependencyCount, const GvkRestorePointObject* pDependencies);" << std::endl;
        file << "    VkResult restore_dependencies(uint32_t dependencyCount, const GvkRestorePointObject* pDependencies);" << std::endl;
        file << "    VkResult restore_dependencies_state(uint32_t dependencyCount, const GvkRestorePointObject* pDependencies);" << std::endl;
        file << "    void destroy_object(const GvkRestorePointObject& restorePointObject);" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "    virtual VkResult restore_" << handle.name << "(const GvkRestorePointObject& restorePointObject, const " << get_restore_info_type_name(handle.name) << "& restoreInfo)" << (pure_virtual(handle.name) ? " = 0;" : ";") << std::endl;
                file << "    virtual VkResult restore_" << handle.name << "_state(const GvkRestorePointObject&, const " << get_restore_info_type_name(handle.name) << "&)" << (pure_virtual(handle.name) ? " = 0;" : " { return VK_SUCCESS; }") << std::endl;
                file << "    virtual VkResult process_" << handle.name << "(const GvkRestorePointObject& restorePointObject, const " << get_restore_info_type_name(handle.name) << "& restoreInfo)" << (pure_virtual(handle.name) ? " = 0;" : ";") << std::endl;
                for (const auto& createCommandName : handle.createCommands) {
                    const auto& createCommandItr = manifest.commands.find(createCommandName);
                    assert(createCommandItr != manifest.commands.end());
                    const auto& createCommand = createCommandItr->second;
                    auto commandStructureName = "GvkCommandStructure" + string::strip_vk(createCommand.name);
                    CompileGuardGenerator createCommandCompileGuard(file, get_inner_scope_compile_guards(handle.compileGuards, createCommand.compileGuards));
                    file << "    virtual VkResult process_" << commandStructureName << "(const GvkRestorePointObject&, const " << get_restore_info_type_name(handle.name) << "&, " << commandStructureName << "&) { return VK_SUCCESS; }" << std::endl;
                }
                file << "    virtual void destroy_" << handle.name << "(const GvkRestorePointObject& restorePointObject)" << (pure_virtual(handle.name) ? " = 0;" : ";") << std::endl;
            }
        }
        file << "    std::set<GvkRestorePointObject> mRestoredObjects;" << std::endl;
        file << "    std::set<GvkRestorePointObject> mRestoredObjectStates;" << std::endl;
        file << "    std::set<GvkRestorePointObject> mProcessedObjects;" << std::endl;
        file << "    std::map<GvkRestorePointObject, GvkRestorePointObject> mRestorePointObjects;" << std::endl;
        file << "    // std::map<GvkRestorePointObject, GvkRestorePointObject> mRestorePointObjectsEx;" << std::endl;
        file << "    ApplyInfo mApplyInfo{ };" << std::endl;
        file << "    VkResult mResult{ VK_ERROR_INITIALIZATION_FAILED };" << std::endl;
        file << "private:" << std::endl;
        file << "    BasicApplier(const BasicApplier&) = delete;" << std::endl;
        file << "    BasicApplier& operator=(const BasicApplier&) = delete;" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-command-structures.hpp\"" << std::endl;
        file << "#include \"gvk-command-structures/generated/execute-command-structure.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/generated/update-structure-handles.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/layer.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << "#include \"VK_LAYER_INTEL_gvk_restore_point.hpp\"" << std::endl;
        file << "#include \"VK_LAYER_INTEL_gvk_state_tracker.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::restore_point");
        file << std::endl;
        file << "BasicApplier::~BasicApplier()" << std::endl;
        file << "{" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult BasicApplier::register_restored_object(const GvkRestorePointObject& capturedObject, const GvkRestorePointObject& restoredObject)" << std::endl;
        file << "{" << std::endl;
        file << "    auto inserted = mRestorePointObjects.insert({ capturedObject, restoredObject }).second;" << std::endl;
        file << "    if (inserted && mApplyInfo.pfnProcessRestoredObjectCallback) {" << std::endl;
        file << "        mApplyInfo.pfnProcessRestoredObjectCallback((const GvkStateTrackedObject*)&capturedObject, (const GvkStateTrackedObject*)&restoredObject);" << std::endl;
        file << "    }" << std::endl;
        file << "    return inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "GvkRestorePointObject BasicApplier::get_restored_object(const GvkRestorePointObject& restorePointObject)" << std::endl;
        file << "{" << std::endl;
        file << "    auto itr = mRestorePointObjects.find(restorePointObject);" << std::endl;
        file << "    return itr != mRestorePointObjects.end() ? itr->second : GvkRestorePointObject{ };" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult BasicApplier::restore_object(const GvkRestorePointObject& restorePointObject)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_SUCCESS) {" << std::endl;
        file << "        switch (restorePointObject.type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "        case " << handle.vkObjectType << ": {" << std::endl;
                file << "            Auto<" << get_restore_info_type_name(handle.name) << "> restoreInfo;" << std::endl;
                file << "            gvk_result(read_object_restore_info(mApplyInfo.path, \"" << handle.name << "\", to_hex_string(restorePointObject.handle), restoreInfo));" << std::endl;
                file << "            gvk_result(restore_dependencies(restoreInfo->dependencyCount, restoreInfo->pDependencies));" << std::endl;
                file << "            gvk_result(restore_" << handle.name << "(restorePointObject, *restoreInfo));" << std::endl;
                file << "        } break;" << std::endl;
            }
        }
        file << "        default: {" << std::endl;
        file << "            gvk_result(VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "        } break;" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult BasicApplier::restore_object_state(const GvkRestorePointObject& restorePointObject)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_SUCCESS) {" << std::endl;
        file << "        switch (restorePointObject.type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "        case " << handle.vkObjectType << ": {" << std::endl;
                file << "            Auto<" << get_restore_info_type_name(handle.name) << "> restoreInfo;" << std::endl;
                file << "            gvk_result(read_object_restore_info(mApplyInfo.path, \"" << handle.name << "\", to_hex_string(restorePointObject.handle), restoreInfo));" << std::endl;
                file << "            gvk_result(restore_dependencies_state(restoreInfo->dependencyCount, restoreInfo->pDependencies));" << std::endl;
                file << "            gvk_result(restore_" << handle.name << "_state(restorePointObject, *restoreInfo));" << std::endl;
                file << "        } break;" << std::endl;
            }
        }
        file << "        default: {" << std::endl;
        file << "            gvk_result(VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "        } break;" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult BasicApplier::restore_object_name(const GvkRestorePointObject& restorePointObject)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_SUCCESS) {" << std::endl;
        file << "        switch (restorePointObject.type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "        case " << handle.vkObjectType << ": {" << std::endl;
                file << "            Auto<" << get_restore_info_type_name(handle.name) << "> restoreInfo;" << std::endl;
                file << "            gvk_result(read_object_restore_info(mApplyInfo.path, \"" << handle.name << "\", to_hex_string(restorePointObject.handle), restoreInfo));" << std::endl;
                file << "            gvk_result(restore_object_name(restorePointObject, restoreInfo->dependencyCount, restoreInfo->pDependencies, restoreInfo->pName));" << std::endl;
                file << "        } break;" << std::endl;
            }
        }
        file << "        default: {" << std::endl;
        file << "            gvk_result(VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "        } break;" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult BasicApplier::process_object(const GvkRestorePointObject& restorePointObject)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_SUCCESS) {" << std::endl;
        file << "        if (mProcessedObjects.insert(restorePointObject).second) {" << std::endl;
        file << "            switch (restorePointObject.type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "            case " << handle.vkObjectType << ": {" << std::endl;
                file << "                Auto<" << get_restore_info_type_name(handle.name) << "> restoreInfo;" << std::endl;
                file << "                gvk_result(read_object_restore_info(mApplyInfo.path, \"" << handle.name << "\", to_hex_string(restorePointObject.handle), restoreInfo));" << std::endl;
                file << "                gvk_result(process_dependencies(restoreInfo->dependencyCount, restoreInfo->pDependencies));" << std::endl;
                file << "                gvk_result(process_" << handle.name << "(restorePointObject, *restoreInfo));" << std::endl;
                file << "            } break;" << std::endl;
            }
        }
        file << "            default: {" << std::endl;
        file << "                gvk_result(VK_ERROR_INITIALIZATION_FAILED);" << std::endl;
        file << "            } break;" << std::endl;
        file << "            }" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult BasicApplier::process_dependencies(uint32_t dependencyCount, const GvkRestorePointObject* pDependencies)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_SUCCESS) {" << std::endl;
        file << "        for (uint32_t i = 0; i < dependencyCount; ++i) {" << std::endl;
        file << "            gvk_result(process_object(pDependencies[i]));" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult BasicApplier::restore_dependencies(uint32_t dependencyCount, const GvkRestorePointObject* pDependencies)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_SUCCESS) {" << std::endl;
        file << "        for (uint32_t i = 0; i < dependencyCount; ++i) {" << std::endl;
        file << "            gvk_result(restore_object(pDependencies[i]));" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "VkResult BasicApplier::restore_dependencies_state(uint32_t dependencyCount, const GvkRestorePointObject* pDependencies)" << std::endl;
        file << "{" << std::endl;
        file << "    gvk_result_scope_begin(VK_SUCCESS) {" << std::endl;
        file << "        for (uint32_t i = 0; i < dependencyCount; ++i) {" << std::endl;
        file << "            gvk_result(restore_object_state(pDependencies[i]));" << std::endl;
        file << "        }" << std::endl;
        file << "    } gvk_result_scope_end;" << std::endl;
        file << "    return gvkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "void BasicApplier::destroy_object(const GvkRestorePointObject& restorePointObject)" << std::endl;
        file << "{" << std::endl;
        file << "    switch (restorePointObject.type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty() && !handle.destroyCommands.empty()) {
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "    case " << handle.vkObjectType << ": {" << std::endl;
                file << "        destroy_" << handle.name << "(restorePointObject);" << std::endl;
                file << "    } break;" << std::endl;
            }
        }
        file << "    default: {" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                file << std::endl;
                auto handleCompileGuards = handle.compileGuards;
                if (pure_virtual(handle.name)) {
                    handleCompileGuards.insert("process_" + handle.name + "_PURE_VIRTUAL");
                }
                CompileGuardGenerator compileGuardGenerator(file, handleCompileGuards);
                file << "VkResult BasicApplier::restore_" << handle.name << "(const GvkRestorePointObject& restorePointObject, const " << get_restore_info_type_name(handle.name) << "& restoreInfo)" << std::endl;
                file << "{" << std::endl;
                if (!handle.createInfos.empty()) {
                    file << "    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {" << std::endl;
                    for (const auto& createInfoType : handle.createInfos) {
                        const auto& createInfoItr = manifest.structures.find(createInfoType);
                        assert(createInfoItr != manifest.structures.end());
                        const auto& createInfo = createInfoItr->second;
                        CompileGuardGenerator createInfoCompileGuard(file, get_inner_scope_compile_guards(handleCompileGuards, createInfo.compileGuards));
                        file << "        if (restoreInfo.p" << string::strip_vk(createInfo.name) << ") {" << std::endl;
                        for (const auto& createCommandName : handle.createCommands) {
                            const auto& createCommandItr = manifest.commands.find(createCommandName);
                            assert(createCommandItr != manifest.commands.end());
                            const auto& createCommand = createCommandItr->second;
                            if (createCommand.get_create_info_parameter().unqualifiedType == createInfoType) {
                                file << "            auto commandStructure = get_default<GvkCommandStructure" << string::strip_vk(createCommand.name) << ">();" << std::endl;
                                std::string outHandleParameterName;
                                for (const auto& parameter : createCommand.parameters) {
                                    if (manifest.handles.count(parameter.type)) {
                                        file << "            commandStructure." << parameter.name << " = get_dependency<" << parameter.type << ">(restoreInfo.dependencyCount, restoreInfo.pDependencies);" << std::endl;
                                    } else if (string::contains(parameter.unqualifiedType, "CreateInfo") || string::contains(parameter.unqualifiedType, "AllocateInfo")) {
                                        file << "            commandStructure." << parameter.name << " = restoreInfo.p" << string::strip_vk(parameter.unqualifiedType) << ";" << std::endl;
                                    } else if (manifest.handles.count(parameter.unqualifiedType)) {
                                        outHandleParameterName = parameter.name;
                                    }
                                }
                                file << "            gvk_result(process_GvkCommandStructure" << string::strip_vk(createCommand.name) << "(restorePointObject, restoreInfo, commandStructure));" << std::endl;
                                file << "            update_command_structure_handles(mRestorePointObjects, commandStructure);" << std::endl;
                                file << "            " << handle.name << " handle = restoreInfo.handle;" << std::endl;
                                file << "            commandStructure." << outHandleParameterName << " = &handle;" << std::endl;
                                file << "            gvk_result(detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure));" << std::endl;
                                file << "            auto restoredObject = restorePointObject;" << std::endl;
                                file << "            restoredObject.handle = (uint64_t)handle;" << std::endl;
                                if (handle.isDispatchable) {
                                    file << "            restoredObject.dispatchableHandle = (uint64_t)handle;" << std::endl;
                                } else {
                                    file << "            restoredObject.dispatchableHandle = (uint64_t)commandStructure." << createCommand.parameters[0].name << ";" << std::endl;
                                }
                                file << "            gvk_result(register_restored_object_ex(restorePointObject, restoredObject));" << std::endl;
                            }
                        }
                        file << "        }" << std::endl;
                    }
                    file << "    } gvk_result_scope_end;" << std::endl;
                    file << "    return gvkResult;" << std::endl;
                } else {
                    file << "    (void)restorePointObject;" << std::endl;
                    file << "    (void)restoreInfo;" << std::endl;
                    file << "    return VK_SUCCESS;" << std::endl;
                }
                file << "}" << std::endl;
                file << std::endl;
                file << "VkResult BasicApplier::process_" << handle.name << "(const GvkRestorePointObject& restorePointObject, const " << get_restore_info_type_name(handle.name) << "& restoreInfo)" << std::endl;
                file << "{" << std::endl;
                if (!handle.createInfos.empty()) {
                    file << "    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {" << std::endl;
                    for (const auto& createInfoType : handle.createInfos) {
                        const auto& createInfoItr = manifest.structures.find(createInfoType);
                        assert(createInfoItr != manifest.structures.end());
                        const auto& createInfo = createInfoItr->second;
                        CompileGuardGenerator createInfoCompileGuard(file, get_inner_scope_compile_guards(handleCompileGuards, createInfo.compileGuards));
                        file << "        if (restoreInfo.p" << string::strip_vk(createInfo.name) << ") {" << std::endl;
                        for (const auto& createCommandName : handle.createCommands) {
                            const auto& createCommandItr = manifest.commands.find(createCommandName);
                            assert(createCommandItr != manifest.commands.end());
                            const auto& createCommand = createCommandItr->second;
                            if (createCommand.get_create_info_parameter().unqualifiedType == createInfoType) {
                                file << "            auto commandStructure = get_default<GvkCommandStructure" << string::strip_vk(createCommand.name) << ">();" << std::endl;
                                std::string outHandleParameterName;
                                for (const auto& parameter : createCommand.parameters) {
                                    if (manifest.handles.count(parameter.type)) {
                                        file << "            commandStructure." << parameter.name << " = get_dependency<" << parameter.type << ">(restoreInfo.dependencyCount, restoreInfo.pDependencies);" << std::endl;
                                    } else if (string::contains(parameter.unqualifiedType, "CreateInfo") || string::contains(parameter.unqualifiedType, "AllocateInfo")) {
                                        file << "            commandStructure." << parameter.name << " = restoreInfo.p" << string::strip_vk(parameter.unqualifiedType) << ";" << std::endl;
                                    } else if (manifest.handles.count(parameter.unqualifiedType)) {
                                        outHandleParameterName = parameter.name;
                                    }
                                }
                                file << "            gvk_result(process_GvkCommandStructure" << string::strip_vk(createCommand.name) << "(restorePointObject, restoreInfo, commandStructure));" << std::endl;
                                file << "            update_command_structure_handles(mRestorePointObjects, commandStructure);" << std::endl;
                                file << "            " << handle.name << " handle = restoreInfo.handle;" << std::endl;
                                file << "            commandStructure." << outHandleParameterName << " = &handle;" << std::endl;
                                file << "            gvk_result(detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure));" << std::endl;
                                file << "            auto restoredObject = restorePointObject;" << std::endl;
                                file << "            restoredObject.handle = (uint64_t)handle;" << std::endl;
                                if (handle.isDispatchable) {
                                    file << "            restoredObject.dispatchableHandle = (uint64_t)handle;" << std::endl;
                                } else {
                                    file << "            restoredObject.dispatchableHandle = (uint64_t)commandStructure." << createCommand.parameters[0].name << ";" << std::endl;
                                }
                                file << "            gvk_result(register_restored_object(restorePointObject, restoredObject));" << std::endl;
                            }
                        }
                        file << "        }" << std::endl;
                    }
                    file << "    } gvk_result_scope_end;" << std::endl;
                    file << "    return gvkResult;" << std::endl;
                } else {
                    file << "    (void)restorePointObject;" << std::endl;
                    file << "    (void)restoreInfo;" << std::endl;
                    file << "    return VK_SUCCESS;" << std::endl;
                }
                file << "}" << std::endl;
                file << std::endl;
                file << "void BasicApplier::destroy_" << handle.name << "(const GvkRestorePointObject& restorePointObject)" << std::endl;
                file << "{" << std::endl;
                if (!handle.destroyCommands.empty()) {
                    assert(handle.destroyCommands.size() == 1);
                    const auto& destroyCommandItr = manifest.commands.find(*handle.destroyCommands.begin());
                    assert(destroyCommandItr != manifest.commands.end());
                    const auto& destroyCommand = destroyCommandItr->second;
                    file << "    auto commandStructure = get_default<GvkCommandStructure" << string::strip_vk(destroyCommand.name) << ">();" << std::endl;
                    for (const auto& parameter : destroyCommand.parameters) {
                        if (parameter.type == handle.name) {
                            file << "    commandStructure." << parameter.name << " = (" << parameter.type << ")restorePointObject.handle;" << std::endl;
                        } else if (parameter.unqualifiedType == handle.name) {
                            file << "    commandStructure." << parameter.length << " = 1;" << std::endl;
                            file << "    commandStructure." << parameter.name << " = (" << parameter.type << ")&restorePointObject.handle;" << std::endl;
                        } else if (parameter.type == handle.get_dispatchable_handle(manifest)) {
                            file << "    commandStructure." << parameter.name << " = (" << parameter.type << ")restorePointObject.dispatchableHandle;" << std::endl;
                        }
                    }
                file << "    detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure);" << std::endl;
                } else {
                    file << "    (void)restorePointObject;" << std::endl;
                }
                file << "}" << std::endl;
            }
        }
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
