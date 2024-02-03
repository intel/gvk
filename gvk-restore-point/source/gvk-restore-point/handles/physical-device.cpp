
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

#include "gvk-restore-point/applier.hpp"
#include "gvk-restore-point/creator.hpp"

namespace gvk {
namespace restore_point {

VkResult Creator::process_VkPhysicalDevice(GvkPhysicalDeviceRestoreInfo& restoreInfo)
{
    // NOTE : Documentation
    DispatchTable dispatchTable { };
    dispatchTable.gvkGetInstanceProcAddr = load_vkGetInstanceProcAddr();
    DispatchTable::load_global_entry_points(&dispatchTable);
    DispatchTable::load_instance_entry_points(get_dependency<VkInstance>(restoreInfo.dependencyCount, restoreInfo.pDependencies), &dispatchTable);

    dispatchTable.gvkGetPhysicalDeviceFeatures(restoreInfo.handle, &restoreInfo.physicalDeviceFeatures);
    dispatchTable.gvkGetPhysicalDeviceProperties(restoreInfo.handle, &restoreInfo.physicalDeviceProperties);
    dispatchTable.gvkGetPhysicalDeviceMemoryProperties(restoreInfo.handle, &restoreInfo.physicalDeviceMemoryProperties);
    uint32_t queueFamilyPropertyCount = 0;
    dispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(restoreInfo.handle, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    dispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(restoreInfo.handle, &queueFamilyPropertyCount, queueFamilyProperties.data());
    restoreInfo.queueFamilyPropertyCount = (uint32_t)queueFamilyProperties.size();
    restoreInfo.pQueueFamilyProperties = !queueFamilyProperties.empty() ? queueFamilyProperties.data() : nullptr;
    return BasicCreator::process_VkPhysicalDevice(restoreInfo);
}

VkResult Applier::process_VkPhysicalDevice(const GvkRestorePointObject& restorePointObject, const GvkPhysicalDeviceRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        auto& unrestoredPhysicalDevices = mUnrestoredPhysicalDevices[restoreInfo.physicalDeviceProperties];
        if (!unrestoredPhysicalDevices.empty()) {
            auto restoredObject = restorePointObject;
            restoredObject.handle = (uint64_t)unrestoredPhysicalDevices.front();
            restoredObject.dispatchableHandle = (uint64_t)unrestoredPhysicalDevices.front();
            gvk_result(register_restored_object(restorePointObject, restoredObject));
            unrestoredPhysicalDevices.erase(unrestoredPhysicalDevices.begin());
        }
        // TODO : Warn when a capture time physical device is unavailable at playback
        //  time.  It's not necessarily an error, as long as the physical device isn't
        //  used...ie. the capture system had integrated and Arc A770 graphics, but the
        //  playback system only has Arc A770, as long as only the Arc A770 graphics
        //  are used there's no problem.
        // NOTE : When it comes time to setup support for cross GPU playback this will
        //  need to be addressed in more detail.
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk
