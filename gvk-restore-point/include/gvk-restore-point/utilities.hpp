
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

#include "gvk-defines.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-restore-info.hpp"
#include "gvk-runtime.hpp"
#include "gvk-structures.hpp"
#include "VK_LAYER_INTEL_gvk_restore_point.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <set>

namespace gvk {
namespace restore_point {

using CapturedObject = GvkRestorePointObject;
using RestoredObject = GvkRestorePointObject;

class ObjectMap final
{
public:
    ObjectMap() = default;
    ObjectMap(const GvkRestorePointManifest& manifest);
    ObjectMap(ObjectMap&& other) = default;
    ObjectMap& operator=(ObjectMap&& other) = default;
    const GvkRestorePointManifest& get_manifest() const;
    void register_object_restoration(const CapturedObject& capturedObject, const RestoredObject& restoredObject);
    void register_object_destruction(const GvkRestorePointObject& object);

    void set_object_mapping(const CapturedObject& capturedObject, const RestoredObject& restoredObject);
    const std::map<CapturedObject, RestoredObject>& get_restored_objects() const;
    RestoredObject get_restored_object(const CapturedObject& capturedObject) const;
    CapturedObject get_captured_object(const RestoredObject& restoredObject) const;
private:
    Auto<GvkRestorePointManifest> mManifest;
    std::map<CapturedObject, RestoredObject> mRestoredObjects;
    std::map<RestoredObject, CapturedObject> mCapturedObjects;
    ObjectMap(const ObjectMap&) = delete;
    ObjectMap& operator=(const ObjectMap&) = delete;
};

class LayerInfo final
{
public:
    void register_object_creation(const GvkRestorePointObject& restorePointObject);
    void register_object_destruction(const GvkRestorePointObject& restorePointObject);
    ObjectMap objectMap;
    std::set<GvkRestorePointObject> createdObjects;
    std::set<GvkRestorePointObject> destroyedObjects;
};

class CreateInfo final
{
public:
    GvkRestorePointCreateFlags flags{ };
    VkInstance instance{ };
    std::filesystem::path path;
    uint32_t threadCount{ };
    PFN_gvkInitializeThreadCallback pfnInitializeThreadCallback{ };
    PFN_gvkProcessResourceDataCallback pfnProcessResourceDataCallback{ };
    VkBool32 repeating_HACK{ };
};

class ApplyInfo final
{
public:
    GvkRestorePointApplyFlags flags{ };
    bool restoreInstance_HACK{ };
    VkInstance instance{ };
    std::filesystem::path path;
    uint32_t threadCount{ };
    std::set<GvkStateTrackedObject> excludedObjects;
    PFN_gvkInitializeThreadCallback pfnInitializeThreadCallback{ };
    PFN_gvkProcessRestoredObjectCallback pfnProcessRestoredObjectCallback{ };
    PFN_gvkProcessResourceDataCallback pfnProcessResourceDataCallback{ };
#ifdef VK_USE_PLATFORM_WIN32_KHR
    PFN_gvkProcessWin32SurfaceCreateInfoCallback pfnProcessWin32SurfaceCreateInfoCallback{ };
#endif
    DispatchTable dispatchTable{ };
    VkBool32 repeating_HACK{ };
    LayerInfo* pLayerInfo{ };
};

class CmdEnumerationUserData final
{
public:
    const CreateInfo* pCreateInfo{ };
    VkCommandBuffer commandBuffer{ };
    std::ofstream jsonFile;
    std::ofstream cmdsFile;
    uint32_t cmdCount{ };
};

template <typename ObjectType>
inline GvkRestorePointObject get_restore_point_object_dependency(uint32_t dependencyCount, const GvkRestorePointObject* pDependencies)
{
    for (uint32_t i = 0; i < dependencyCount; ++i) {
        const auto& dependency = pDependencies[i];
        if (dependency.type == detail::get_object_type<ObjectType>()) {
            return dependency;
        }
    }
    return { };
}

template <typename ObjectType>
inline ObjectType get_dependency(uint32_t dependencyCount, const GvkRestorePointObject* pDependencies)
{
    return (ObjectType)get_restore_point_object_dependency<ObjectType>(dependencyCount, pDependencies).handle;
}

template <typename RestoreInfoType>
inline VkResult write_object_restore_info(const CreateInfo& restorePointCreateInfo, const std::string& type, const std::string& name, const RestoreInfoType& objectRestoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto path = restorePointCreateInfo.path / type;
        std::filesystem::create_directories(path);
        if (restorePointCreateInfo.flags & GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT) {
            auto infoPath = (path / name).replace_extension("info");
            std::ofstream infoFile(infoPath, std::ios::binary);
            gvk_result(infoFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            serialize(infoFile, objectRestoreInfo);
        }
        if (restorePointCreateInfo.flags & GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT) {
            auto jsonPath = (path / name).replace_extension("json");
            std::ofstream jsonFile(jsonPath);
            gvk_result(jsonFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
            jsonFile << to_string(objectRestoreInfo, printerFlags) << std::endl;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

template <typename RestoreInfoType>
inline VkResult read_object_restore_info(const std::filesystem::path& path, const std::string& type, const std::string& name, Auto<RestoreInfoType>& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto infoPath = (path / type / name).replace_extension("info");
        std::ifstream infoFile(infoPath, std::ios::binary);
        gvk_result(infoFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        deserialize(infoFile, nullptr, restoreInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

inline const void* remove_pnext_entries(VkBaseOutStructure* pNext, const std::set<VkStructureType>& structureType)
{
    // TODO : Make this function more generic...in its current state it's only safe
    //  if its safe to use detail::destroy_pnext_copy()
    while (pNext) {
        while (pNext->pNext && structureType.count(pNext->pNext->sType)) {
            auto pRemove = pNext->pNext;
            pNext->pNext = pRemove->pNext;
            pRemove->pNext = nullptr;
            detail::destroy_pnext_copy(pRemove, nullptr);
        }
        pNext = pNext->pNext;
    }
    return pNext;
}

} // namespace restore_point
} // namespace gvk
