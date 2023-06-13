
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

inline std::string get_restored_handle_arguments(const xml::Manifest& manifest, const xml::Command& command)
{
    std::stringstream strStrm;
    for (const auto& parameter : command.parameters) {
        const auto& handleItr = manifest.handles.find(parameter.type);
        if (handleItr != manifest.handles.end()) {
            const auto& handle = handleItr->second;
            strStrm << "        auto " << parameter.name << " = (" << handle.name << ")get_restored_handle(get_object(" << handle.vkObjectType << ", restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);" << std::endl;
        }
    }
    return strStrm.str();
}

inline std::string get_create_command_arguments(const xml::Manifest& manifest, const xml::Command& command)
{
    std::stringstream strStrm;
    for (const auto& parameter : command.parameters) {
        if (manifest.handles.count(parameter.type)) {
            strStrm << parameter.name;
        } else if (manifest.structures.count(parameter.unqualifiedType)) {
            if (parameter.unqualifiedType == "VkAllocationCallbacks") {
                strStrm << "nullptr";
            } else {
                strStrm << "restoreInfo.p" << string::strip_vk(parameter.unqualifiedType);
            }
        } else if (parameter.unqualifiedType == command.target) {
            strStrm << "&handle";
        }
        strStrm << ", ";
    }
    auto str = strStrm.str();
    str.pop_back();
    str.pop_back();
    return str;
}

inline std::string get_create_command_call(const xml::Manifest& manifest, const xml::Handle& handle, const std::string& createInfo)
{
    std::stringstream strStrm;
    for (const auto& createCommand : handle.createCommands) {
        const auto& createCommandItr = manifest.commands.find(createCommand);
        const auto& command = createCommandItr->second;
        for (const auto& parameter : createCommandItr->second.parameters) {
            if (parameter.unqualifiedType == createInfo) {
                strStrm << get_restored_handle_arguments(manifest, command);
                strStrm << "        vkResult = mDispatchTable.g" << command.name << "(" << get_create_command_arguments(manifest, command) << ");" << std::endl;
                break;
            }
        }
    }
    return strStrm.str();
}

class BasicRestorePointApplierGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        ModuleGenerator module(
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PATH,
            GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX,
            GVK_RESTORE_POINT_GENERATED_SOURCE_PATH,
            "basic-restore-point-applier"
        );
        generate_header(module.header, manifest);
        generate_source(module.source, manifest);
    }

private:
    static const std::set<std::string>& pure_virtual()
    {
        static const std::set<std::string> scPureVirtual {
            "VkInstance",
            "VkDevice",
            "VkPipeline",
            "VkCommandBuffer",
            "VkDescriptorSet",
            "VkDisplayModeKHR",
            "VkShaderEXT",
            "VkSurfaceKHR",
            "VkSwapchainKHR",
        };
        return scPureVirtual;
    }

    static void generate_header(FileGenerator& file, const xml::Manifest& manifest)
    {
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << "#include \"gvk-dispatch-table.hpp\"" << std::endl;
        file << "#include \"gvk-environment.hpp\"" << std::endl;
        file << "#include \"gvk-runtime.hpp\"" << std::endl;
        file << "#include \"gvk-structures.hpp\"" << std::endl;
        file << "#include \"gvk-reference/handle-id.hpp\"" << std::endl;
        file << "#include \"gvk-restore-point/detail/restore-point-applier-base.hpp\"" << std::endl;
        file << "#include \"VK_LAYER_INTEL_gvk_state_tracker.h\"" << std::endl;
        file << std::endl;
        file << "#include <fstream>" << std::endl;
        file << "#include <unordered_map>" << std::endl;
        file << "#include <unordered_set>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;
        file << "class BasicRestorePointApplier" << std::endl;
        file << "    : public RestorePointApplierBase" << std::endl;
        file << "{" << std::endl;
        file << "public:" << std::endl;
        file << "    virtual ~BasicRestorePointApplier() = 0;" << std::endl;
        file << "    virtual VkResult apply_restore_point(const restore_point::ApplyInfo& restorePointInfo, const DispatchTable& dispatchTable, const DispatchTable& dynamicDispatchTable) = 0;" << std::endl;
        file << std::endl;
        file << "protected:" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                auto extensionVendor = gvk::cppgen::get_extension_vendor(handle.name);
                auto restoreInfoTypeName = "Gvk" + gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor) + "RestoreInfo" + extensionVendor;
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "    virtual VkResult process_" << handle.name << "(const GvkStateTrackedObject& stateTrackedObject, const " << restoreInfoTypeName << "& restoreInfo)";
                if (pure_virtual().count(handle.name)) {
                    file << " = 0";
                }
                file << ";" << std::endl;
            }
        }
        file << "    virtual VkResult restore_command_buffer_cmds(const GvkStateTrackedObject& stateTrackedObject);" << std::endl;
        file << "    virtual VkResult process_object(const GvkStateTrackedObject& stateTrackedObject);" << std::endl;
        file << "};" << std::endl;
        file << std::endl;
    }

    static void generate_source(FileGenerator& file, const xml::Manifest& manifest)
    {
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
        file << "#include <fstream>" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::detail");
        file << std::endl;
        file << "BasicRestorePointApplier::~BasicRestorePointApplier()" << std::endl;
        file << "{" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                file << std::endl;
                auto extensionVendor = gvk::cppgen::get_extension_vendor(handle.name);
                auto restoreInfoTypeName = "Gvk" + gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor) + "RestoreInfo" + extensionVendor;
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                if (pure_virtual().count(handle.name)) {
                    file << "#if 0 // NOTE : Pure virtual" << std::endl;
                }
                file << "VkResult BasicRestorePointApplier::process_" << handle.name << "(const GvkStateTrackedObject& stateTrackedObject, const " << restoreInfoTypeName << "& restoreInfo)" << std::endl;
                file << "{" << std::endl;
                file << "    (void)restoreInfo;" << std::endl;
                file << "    auto vkResult = VK_SUCCESS;" << std::endl;
                file << "    assert(mCurrentObjects.empty());" << std::endl;
                file << "    mCurrentObjects.push_back(stateTrackedObject);" << std::endl;
                for (const auto& createInfoName : handle.createInfos) {
                    const auto& createInfoItr = manifest.structures.find(createInfoName);
                    assert(createInfoItr != manifest.structures.end());
                    const auto& createInfo = createInfoItr->second;
                    CompileGuardGenerator createInfoCompileGuard(file, get_inner_scope_compile_guards(handle.compileGuards, createInfo.compileGuards));
                    file << "    if (restoreInfo.p" << string::strip_vk(createInfo.name) << ") {" << std::endl;
                    file << "        update_handles(restoreInfo.p" << string::strip_vk(createInfo.name) << ");" << std::endl;
                    file << "        " << handle.name << " handle = mApplyInfo.recording ? (" << handle.name << ")stateTrackedObject.handle : VK_NULL_HANDLE;" << std::endl;
                    file << get_create_command_call(manifest, handle, createInfo.name);
                    file << "        if (vkResult == VK_SUCCESS) {" << std::endl;
                    file << "            register_restored_handle(" << handle.vkObjectType << ", stateTrackedObject.handle, (uint64_t)handle);" << std::endl;
                    file << "        }" << std::endl;
                    file << "    }" << std::endl;
                }
                file << "    mCurrentObjects.clear();" << std::endl;
                file << "    return vkResult;" << std::endl;
                file << "}" << std::endl;
                if (pure_virtual().count(handle.name)) {
                    file << "#endif // NOTE : Pure virtual" << std::endl;
                }
            }
        }
        file << std::endl;
        file << "VkResult BasicRestorePointApplier::process_object(const GvkStateTrackedObject& stateTrackedObject)" << std::endl;
        file << "{" << std::endl;
        file << "    auto vkResult = VK_SUCCESS;" << std::endl;
        file << "    if (mProcessedHandles.insert(HandleId<uint64_t, uint64_t>(stateTrackedObject.dispatchableHandle, stateTrackedObject.handle)).second) {" << std::endl;
        file << "        mProcessedIds.insert(stateTrackedObject.handle);" << std::endl;
        file << "        mProcessedIds.insert(stateTrackedObject.dispatchableHandle);" << std::endl;
        file << "        switch (stateTrackedObject.type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty() && !handle.vkObjectType.empty()) {
                auto extensionVendor = gvk::cppgen::get_extension_vendor(handle.name);
                auto restoreInfoTypeName = "Gvk" + gvk::string::remove(gvk::string::strip_vk(handle.name), extensionVendor) + "RestoreInfo" + extensionVendor;
                CompileGuardGenerator compileGuardGenerator(file, handle.compileGuards);
                file << "        case " << handle.vkObjectType << ": {" << std::endl;
                file << "            Auto<" << restoreInfoTypeName << "> restoreInfo;" << std::endl;
                file << "            auto path = (mApplyInfo.path / \"" << handle.name << "\" / to_hex_string(stateTrackedObject.handle)).replace_extension(\".info\");" << std::endl;
                file << "            std::ifstream file(path, std::ios::binary);" << std::endl;
                file << "            assert(file.is_open());" << std::endl;
                file << "            deserialize(file, nullptr, restoreInfo);" << std::endl;
                file << "            assert((restoreInfo->dependencyCount == 0) == (restoreInfo->pDependencies == nullptr));" << std::endl;
                file << "            for (uint32_t i = 0; i < restoreInfo->dependencyCount && vkResult == VK_SUCCESS; ++i) {" << std::endl;
                file << "                vkResult = process_object(*(const GvkStateTrackedObject*)&restoreInfo->pDependencies[i]);" << std::endl;
                file << "            }" << std::endl;
                file << "            if (vkResult == VK_SUCCESS) {" << std::endl;
                file << "                vkResult = process_" << handle.name << "(stateTrackedObject, restoreInfo);" << std::endl;
                file << "            }" << std::endl;
                file << "        } break;" << std::endl;
            }
        }
        file << "        default: {" << std::endl;
        file << "            assert(false && \"TODO : Documentation\");" << std::endl;
        file << "        } break;" << std::endl;;
        file << "        }" << std::endl;
        file << "    }" << std::endl;
        file << "    return vkResult;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
