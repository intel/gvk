
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

#include "gvk-restore-point/creator.hpp"
#include "gvk-environment.hpp"
#include "gvk-format-info.hpp"
#include "gvk-layer.hpp"
#include "gvk-structures.hpp"

#include <set>
#include <utility>
#include <vector>

namespace gvk {
namespace restore_point {

template <typename T>
static void write_command(GvkRestorePointCreateFlags flags, std::ofstream& infoFile, std::ofstream& jsonFile, const GvkCommandBaseStructure* pCommand)
{
    assert(pCommand->sType == get_stype<T>());
    if (flags & GVK_RESTORE_POINT_CREATE_OBJECT_INFO_BIT) {
        serialize(infoFile, *(const T*)pCommand);
    }
    if (flags & GVK_RESTORE_POINT_CREATE_OBJECT_JSON_BIT) {
        jsonFile << to_string(*(const T*)pCommand, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
    }
}

VkResult Creator::create_restore_point(const CreateInfo& createInfo)
{
    mLog.set_instance(createInfo.instance);
    mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    mLog << "Entered gvk::restore_point::Creator::create_restore_point()" << layer::Log::Flush;

    mCreateInfo = createInfo;
    std::filesystem::create_directories(createInfo.path);

    // Process the VkInstance
    GvkStateTrackedObject stateTrackedInstance{ };
    stateTrackedInstance.type = VK_OBJECT_TYPE_INSTANCE;
    stateTrackedInstance.handle = (uint64_t)createInfo.instance;
    stateTrackedInstance.dispatchableHandle = (uint64_t)createInfo.instance;
    process_object(&stateTrackedInstance, nullptr, this);

    // Enumerate all VkInstance objects with process_object()
    GvkStateTrackedObjectEnumerateInfo enumerateInfo{ };
    enumerateInfo.pfnCallback = process_object;
    enumerateInfo.pUserData = this;
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);

    // Prepare array of all objects besides debug objects
    std::vector<GvkStateTrackedObject> objects;
    objects.reserve(mCreateInfo.gvkRestorePoint->objectMap.size());
    for (const auto& itr : mCreateInfo.gvkRestorePoint->objectMap.get_restored_objects()) {
        const auto& object = itr.first;
        if (object.type != VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT &&
            object.type != VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT) {
            objects.push_back(object);
        }
    }

    gvk_result_scope_begin(mResult) {
        // Write the GvkRestorePointManifest
        auto restorePointManifest = get_default<GvkRestorePointManifest>();
        restorePointManifest.objectCount = (uint32_t)objects.size();
        restorePointManifest.pObjects = objects.data();
        mCreateInfo.gvkRestorePoint->manifest = restorePointManifest;
        gvk_result(write_object_restore_info(mCreateInfo, { }, "GvkRestorePointManifest", restorePointManifest));

        // Process acceleration structure data after everything else
        if (mCreateInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT) {
            gvk_result(process_VkAccelerationStructureKHR_downloads());
        }

        // Wait for transfers to complete before moving on
        for (const auto& gvkDevice : mDevices) {
            gvk_result(gvkDevice.get<DispatchTable>().gvkDeviceWaitIdle(gvkDevice));
        }
    } gvk_result_scope_end;
    mResult = gvkResult;

    mInstance.reset();
    mDevices.clear();
    mDeviceQueueCreateInfos.clear();
    mCopyEngines.clear();

    mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    mLog << "Leaving gvk::restore_point::Creator::create_restore_point() " << gvk::to_string(mResult, Printer::Default & ~Printer::EnumValue) << layer::Log::Flush;
    return mResult;
}

VkResult Creator::process_VkDebugReportCallbackEXT(GvkDebugReportCallbackRestoreInfoEXT& restoreInfo)
{
    (void)restoreInfo;
    // NOOP : VkDebugReportCallbackEXT excluded from restore point
    return VK_SUCCESS;
}

VkResult Creator::process_VkDebugUtilsMessengerEXT(GvkDebugUtilsMessengerRestoreInfoEXT& restoreInfo)
{
    (void)restoreInfo;
    // NOOP : VkDebugUtilsMessengerEXT excluded from restore point
    return VK_SUCCESS;
}

} // namespace restore_point
} // namespace gvk
