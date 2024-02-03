
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
#include "gvk-command-structures/generated/execute-command-structure.hpp"

namespace gvk {
namespace restore_point {

VkResult Creator::process_VkDescriptorSet(GvkDescriptorSetRestoreInfo& restoreInfo)
{
    // TODO : Using Auto<VkWriteDescriptorSet> to ensure that VkWriteDescriptorSet
    //  pNext structures are deep copied since the source structures will go out of
    //  scope after the enumerate callback returns
    auto stateTrackedObject = get_default<GvkStateTrackedObject>();
    stateTrackedObject.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    stateTrackedObject.handle = (uint64_t)restoreInfo.handle;
    stateTrackedObject.dispatchableHandle = (uint64_t)get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
    auto enumerateDescriptorBindings = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
    {
        assert(pInfo);
        assert(pInfo->sType == get_stype<VkWriteDescriptorSet>());
        assert(pUserData);
        ((std::vector<Auto<VkWriteDescriptorSet>>*)pUserData)->push_back(*(VkWriteDescriptorSet*)pInfo);
    };
    std::vector<Auto<VkWriteDescriptorSet>> writeDescriptorSets;
    auto enumerateInfo = get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pfnCallback = enumerateDescriptorBindings;
    enumerateInfo.pUserData = &writeDescriptorSets;
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);
    restoreInfo.descriptorWriteCount = (uint32_t)writeDescriptorSets.size();
    std::vector<VkWriteDescriptorSet> writeDescriptorSetsEx;
    writeDescriptorSetsEx.reserve(writeDescriptorSets.size());
    for (const auto& writeDescriptorSet : writeDescriptorSets) {
        writeDescriptorSetsEx.push_back(writeDescriptorSet);
    }
    restoreInfo.pDescriptorWrites = !writeDescriptorSetsEx.empty() ? writeDescriptorSetsEx.data() : nullptr;
    return BasicCreator::process_VkDescriptorSet(restoreInfo);
}

VkResult Applier::restore_VkDescriptorSet(const GvkRestorePointObject& restorePointObject, const GvkDescriptorSetRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto descriptorSetLayout = get_dependency<VkDescriptorSetLayout>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        auto pDescriptorSetAllocateInfo = const_cast<VkDescriptorSetAllocateInfo*>(restoreInfo.pDescriptorSetAllocateInfo);
        auto descriptorSetCount = pDescriptorSetAllocateInfo->descriptorSetCount;
        auto pSetLayouts = pDescriptorSetAllocateInfo->pSetLayouts;
        pDescriptorSetAllocateInfo->descriptorSetCount = 1;
        pDescriptorSetAllocateInfo->pSetLayouts = &descriptorSetLayout;
        gvk_result(BasicApplier::restore_VkDescriptorSet(restorePointObject, restoreInfo));
        pDescriptorSetAllocateInfo->descriptorSetCount = descriptorSetCount;
        pDescriptorSetAllocateInfo->pSetLayouts = pSetLayouts;
    } gvk_result_scope_end;
    return gvkResult;
}

void Applier::destroy_VkDescriptorSet(const GvkRestorePointObject& restorePointObject)
{
    // TODO : Gather descriptor sets for batch destruction...
    auto commandStructure = get_default<GvkCommandStructureFreeDescriptorSets>();
    commandStructure.device = (VkDevice)restorePointObject.dispatchableHandle;
    commandStructure.descriptorSetCount = 1;
    commandStructure.pDescriptorSets = (const VkDescriptorSet*)&restorePointObject.handle;
    auto enumerateInfo = get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pUserData = &commandStructure.descriptorPool;
    enumerateInfo.pfnCallback = [](const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure*, void* pUserData)
    {
        assert(pStateTrackedObject);
        if (pStateTrackedObject->type == VK_OBJECT_TYPE_DESCRIPTOR_POOL) {
            *(VkDescriptorPool*)pUserData = (VkDescriptorPool)pStateTrackedObject->handle;
        }
    };
    gvkEnumerateStateTrackedObjectDependencies((GvkStateTrackedObject*)&restorePointObject, &enumerateInfo);
    detail::execute_command_structure(mApplyInfo.dispatchTable, commandStructure);
}

VkResult Applier::process_VkDescriptorSet(const GvkRestorePointObject& restorePointObject, const GvkDescriptorSetRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto descriptorSetLayout = get_dependency<VkDescriptorSetLayout>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        auto pDescriptorSetAllocateInfo = const_cast<VkDescriptorSetAllocateInfo*>(restoreInfo.pDescriptorSetAllocateInfo);
        auto descriptorSetCount = pDescriptorSetAllocateInfo->descriptorSetCount;
        auto pSetLayouts = pDescriptorSetAllocateInfo->pSetLayouts;
        pDescriptorSetAllocateInfo->descriptorSetCount = 1;
        pDescriptorSetAllocateInfo->pSetLayouts = &descriptorSetLayout;
        gvk_result(BasicApplier::process_VkDescriptorSet(restorePointObject, restoreInfo));
        pDescriptorSetAllocateInfo->descriptorSetCount = descriptorSetCount;
        pDescriptorSetAllocateInfo->pSetLayouts = pSetLayouts;
    } gvk_result_scope_end;
    return gvkResult;
}

#if 0
VkResult Applier::process_GvkCommandStructureAllocateDescriptorSets(const GvkRestorePointObject& restorePointObject, const GvkDescriptorSetRestoreInfo& restoreInfo, GvkCommandStructureAllocateDescriptorSets& commandStructure)
{
    (void)restorePointObject;
    (void)restoreInfo;
    assert(commandStructure.pAllocateInfo);
    const_cast<VkDescriptorSetAllocateInfo*>(commandStructure.pAllocateInfo)->descriptorSetCount = 1;
    return VK_SUCCESS;
}
#endif

VkResult Applier::process_VkDescriptorSet_bindings(const GvkRestorePointObject& capturedDescriptorSet)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        Auto<GvkDescriptorSetRestoreInfo> descriptorSetRestoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkDescriptorSet", to_hex_string(capturedDescriptorSet.handle), descriptorSetRestoreInfo));

        auto pNext = get_pnext<VkDescriptorSetVariableDescriptorCountAllocateInfo>(*descriptorSetRestoreInfo->pDescriptorSetAllocateInfo);
        if (pNext) {
            assert(false && "TODO : Documentation");
        }

        auto device = get_dependency<VkDevice>(descriptorSetRestoreInfo->dependencyCount, descriptorSetRestoreInfo->pDependencies);
        for (uint32_t i = 0; i < descriptorSetRestoreInfo->descriptorWriteCount; ++i) {
            auto descriptorWrite = descriptorSetRestoreInfo->pDescriptorWrites[i];
            bool activeDescriptor = false;
            detail::enumerate_structure_handles(
                descriptorWrite,
                [&](VkObjectType objectType, const uint64_t& handle)
                {
                    if (handle) {
                        GvkRestorePointObject handleRestorePointObject{ };
                        handleRestorePointObject.type = objectType;
                        handleRestorePointObject.handle = handle;
                        handleRestorePointObject.dispatchableHandle = (uint64_t)device;
                        auto itr = mRestorePointObjects.find(handleRestorePointObject);
                        if (itr != mRestorePointObjects.end()) {
                            activeDescriptor |= handle != (uint64_t)descriptorSetRestoreInfo->handle;
                            const_cast<uint64_t&>(handle) = itr->second.handle;
                        } else {
                            auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
                            mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                            mLog << "gvk::restore_point::Applier; ";
                            mLog << to_string(VK_OBJECT_TYPE_DESCRIPTOR_SET, printerFlags) << " " << descriptorSetRestoreInfo->handle << " ";
                            mLog << "skipping invalid " << to_string(objectType, printerFlags) << " " << to_hex_string(handle);
                            mLog << Log::Flush;
                        }
                    }
                }
            );
            if (activeDescriptor) {
                descriptorWrites.push_back(descriptorWrite);
            }
        }
        if (!descriptorWrites.empty()) {
            device = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)device, (uint64_t)device }).handle;
            mApplyInfo.dispatchTable.gvkUpdateDescriptorSets(device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk
