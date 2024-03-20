
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
#include "gvk-format-info.hpp"

#include "stb/stb_image_write.h"

namespace gvk {
namespace restore_point {

VkResult Creator::process_VkImage(GvkImageRestoreInfo& restoreInfo)
{
    // TODO : Filter downloads based on flags

    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // Setup GvkStateTrackedObject
        auto device = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        auto stateTrackedObject = get_default<GvkStateTrackedObject>();
        stateTrackedObject.type = VK_OBJECT_TYPE_IMAGE;
        stateTrackedObject.handle = (uint64_t)restoreInfo.handle;
        stateTrackedObject.dispatchableHandle = (uint64_t)device;

        // Get VkDeviceMemory bindings
        auto enumerateDeviceMemoryBindings = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
        {
            assert(pInfo);
            assert(pInfo->sType == get_stype<VkBindImageMemoryInfo>());
            assert(pUserData);
            ((std::vector<VkBindImageMemoryInfo>*)pUserData)->push_back(*(VkBindImageMemoryInfo*)pInfo);
        };
        std::vector<VkBindImageMemoryInfo> bindImageMemoryInfos;
        auto enumerateInfo = get_default<GvkStateTrackedObjectEnumerateInfo>();
        enumerateInfo.pfnCallback = enumerateDeviceMemoryBindings;
        enumerateInfo.pUserData = &bindImageMemoryInfos;
        gvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);
        restoreInfo.memoryBindInfoCount = (uint32_t)bindImageMemoryInfos.size();
        restoreInfo.pMemoryBindInfos = !bindImageMemoryInfos.empty() ? bindImageMemoryInfos.data() : nullptr;

        // Get VkImageLayouts
        const auto& imageCreateInfo = restoreInfo.pImageCreateInfo ? *restoreInfo.pImageCreateInfo : VkImageCreateInfo { };
        auto imageSubresourceRange = get_default<VkImageSubresourceRange>();
        imageSubresourceRange.aspectMask = get_image_aspect_flags(imageCreateInfo.format);
        imageSubresourceRange.levelCount = imageCreateInfo.mipLevels;
        imageSubresourceRange.layerCount = imageCreateInfo.arrayLayers;
        auto imageSubresourceCount = imageSubresourceRange.levelCount * imageSubresourceRange.layerCount;
        std::vector<VkImageLayout> imageLayouts(imageSubresourceCount);
        gvkGetStateTrackedImageLayouts(&stateTrackedObject, &imageSubresourceRange, imageLayouts.data());
        restoreInfo.imageSubresourceCount = (uint32_t)imageLayouts.size();
        restoreInfo.pImageLayouts = !imageLayouts.empty() ? imageLayouts.data() : nullptr;

        // Get VkMemoryRequirements
        restoreInfo.memoryRequirements = get_default<VkMemoryRequirements2>();
        Device(device).get<DispatchTable>().gvkGetImageMemoryRequirements(device, restoreInfo.handle, &restoreInfo.memoryRequirements.memoryRequirements);

        // Submit for download
        if (mCreateInfo.flags & (GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT | GVK_RESTORE_POINT_CREATE_IMAGE_PNG_BIT)) {
            // TODO : Option for downloading swapchain images...
            if (!get_dependency<VkSwapchainKHR>(restoreInfo.dependencyCount, restoreInfo.pDependencies)) {
                auto downloadInfo = get_default<CopyEngine::DownloadImageInfo>();
                downloadInfo.device = device;
                downloadInfo.image = restoreInfo.handle;
                downloadInfo.imageCreateInfo = imageCreateInfo;
                downloadInfo.imageSubresourceRange = imageSubresourceRange;
                downloadInfo.pImageLayouts = !imageLayouts.empty() ? imageLayouts.data() : nullptr;
                downloadInfo.pUserData = this;
                downloadInfo.pfnCallback = process_downloaded_VkImage;
                mCopyEngines[device].download(downloadInfo);
            }
        }

        gvk_result(BasicCreator::process_VkImage(restoreInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

void Creator::process_downloaded_VkImage(const CopyEngine::DownloadImageInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData)
{
    assert(downloadInfo.pUserData);
    assert(pData);

    // TODO : Documentation
    // TODO : General cleanup
    const auto& creator = *(const Creator*)downloadInfo.pUserData;
    const auto& imageCreateInfo = downloadInfo.imageCreateInfo;
    auto path = creator.mCreateInfo.path / "VkImage";

    // TODO : Documentation
    // TODO : General cleanup
    if (creator.mCreateInfo.flags & (GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT | GVK_RESTORE_POINT_CREATE_IMAGE_PNG_BIT)) {
        std::filesystem::create_directories(path);
        path /= to_hex_string(downloadInfo.image);
    }

    // TODO : Documentation
    // TODO : General cleanup
    if (creator.mCreateInfo.flags & GVK_RESTORE_POINT_CREATE_IMAGE_PNG_BIT) {
        GvkFormatInfo formatInfo { };
        get_format_info(imageCreateInfo.format, &formatInfo);
        bool pngAble = imageCreateInfo.imageType == VK_IMAGE_TYPE_2D && formatInfo.compressionType == GVK_FORMAT_COMPRESSION_TYPE_NONE;
        for (uint32_t i = 0; i < formatInfo.componentCount && pngAble; ++i) {
            const auto& component = formatInfo.pComponents[i];
            if (component.name != GVK_FORMAT_COMPONENT_NAME_R &&
                component.name != GVK_FORMAT_COMPONENT_NAME_G &&
                component.name != GVK_FORMAT_COMPONENT_NAME_B &&
                component.name != GVK_FORMAT_COMPONENT_NAME_A) {
                pngAble = false;
            }
            if (component.numericFormat != GVK_NUMERIC_FORMAT_UNORM) {
                pngAble = false;
            }
            if (component.bits != 8) {
                pngAble = false;
            }
        }
        if (pngAble) {
            VkDeviceSize offset = 0;
            get_format_info(imageCreateInfo.format, &formatInfo);
            auto bytesPerTexel = get_bytes_per_texel(imageCreateInfo.format);
            const auto& imageSubresourceRange = downloadInfo.imageSubresourceRange;
            auto mipLevelStrMaxSize = std::to_string(imageSubresourceRange.baseMipLevel + imageSubresourceRange.levelCount).size();
            for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel < imageSubresourceRange.levelCount; ++mipLevel) {
                std::string mipLevelStr = std::to_string(mipLevel);
                mipLevelStr.insert(0, mipLevelStrMaxSize - mipLevelStr.size(), '0');
                auto mipLevelExtent = get_mip_level_extent(imageCreateInfo.extent, mipLevel);
                auto arrayLayerStrMaxSize = std::to_string(imageSubresourceRange.baseArrayLayer + imageSubresourceRange.layerCount).size();
                for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer < imageSubresourceRange.layerCount; ++arrayLayer) {
                    std::string arrayLayerStr = std::to_string(arrayLayer);
                    arrayLayerStr.insert(0, arrayLayerStrMaxSize - arrayLayerStr.size(), '0');
                    auto pngPath = path;
                    pngPath.replace_extension(mipLevelStr + '.' + arrayLayerStr + '.' + "png");
                    auto stride = mipLevelExtent.width * bytesPerTexel;
                    stbi_write_png(pngPath.string().c_str(), (int)mipLevelExtent.width, (int)mipLevelExtent.height, (int)formatInfo.componentCount, pData + offset, stride);
                    auto arrayElementSubresourceRange = imageSubresourceRange;
                    arrayElementSubresourceRange.baseMipLevel = mipLevel;
                    arrayElementSubresourceRange.levelCount = 1;
                    arrayElementSubresourceRange.baseArrayLayer = arrayLayer;
                    arrayElementSubresourceRange.layerCount = 1;
                    offset += get_image_data_size(imageCreateInfo, arrayElementSubresourceRange);
                }
            }
        }
    }

    if (creator.mCreateInfo.flags & GVK_RESTORE_POINT_CREATE_IMAGE_DATA_BIT) {
        auto imageDataSize = get_image_data_size(imageCreateInfo, downloadInfo.imageSubresourceRange);
        if (creator.mCreateInfo.pfnProcessResourceDataCallback) {
            GvkStateTrackedObject restorePointObject{ };
            restorePointObject.type = VK_OBJECT_TYPE_IMAGE;
            restorePointObject.handle = (uint64_t)downloadInfo.image;
            restorePointObject.dispatchableHandle = (uint64_t)downloadInfo.device;
            creator.mCreateInfo.pfnProcessResourceDataCallback(&restorePointObject, bindBufferMemoryInfo.memory, imageDataSize, pData);
        } else {
            std::ofstream dataFile(path.replace_extension("data"), std::ios::binary);
            dataFile.write((char*)pData, imageDataSize);
        }
    }
}

VkResult Applier::process_VkImage(const GvkRestorePointObject& restorePointObject, const GvkImageRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (!get_dependency<VkSwapchainKHR>(restoreInfo.dependencyCount, restoreInfo.pDependencies)) {
            const_cast<VkImageCreateInfo*>(restoreInfo.pImageCreateInfo)->usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            gvk_result(BasicApplier::process_VkImage(restorePointObject, restoreInfo));
            for (uint32_t i = 0; i < restoreInfo.memoryBindInfoCount; ++i) {
                auto deviceMemoryRestorePointObject = restorePointObject;
                deviceMemoryRestorePointObject.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
                deviceMemoryRestorePointObject.handle = (uint64_t)restoreInfo.pMemoryBindInfos[i].memory;
                gvk_result(process_object(deviceMemoryRestorePointObject));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkImage_layouts(const GvkRestorePointObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        Auto<GvkImageRestoreInfo> restoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkImage", to_hex_string(restorePointObject.handle), restoreInfo));
        auto vkDevice = get_dependency<VkDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
        vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)vkDevice, (uint64_t)vkDevice }).handle;
        auto vkImage = (VkImage)get_restored_object(restorePointObject).handle;

        const auto& imageCreateInfo = restoreInfo->pImageCreateInfo ? *restoreInfo->pImageCreateInfo : VkImageCreateInfo{ };
        auto imageSubresourceRange = get_default<VkImageSubresourceRange>();
        imageSubresourceRange.aspectMask = get_image_aspect_flags(imageCreateInfo.format);
        imageSubresourceRange.levelCount = imageCreateInfo.mipLevels;
        imageSubresourceRange.layerCount = imageCreateInfo.arrayLayers;
        auto imageSubresourceCount = imageSubresourceRange.levelCount * imageSubresourceRange.layerCount;
        std::vector<VkImageLayout> oldImageLayouts(imageSubresourceCount);
        GvkStateTrackedObject stateTrackedImage{ };
        stateTrackedImage.type = VK_OBJECT_TYPE_IMAGE;
        stateTrackedImage.handle = (uint64_t)vkImage;
        stateTrackedImage.dispatchableHandle = (uint64_t)vkDevice;
        gvkGetStateTrackedImageLayouts(&stateTrackedImage, &imageSubresourceRange, oldImageLayouts.data());

        CopyEngine::TransitionImageLayoutInfo transitionInfo{ };
        transitionInfo.device = vkDevice;
        transitionInfo.image = vkImage;
        transitionInfo.imageCreateInfo = *restoreInfo->pImageCreateInfo;
        transitionInfo.imageSubresourceRange.aspectMask = get_image_aspect_flags(restoreInfo->pImageCreateInfo->format);
        transitionInfo.imageSubresourceRange.levelCount = imageCreateInfo.mipLevels;
        transitionInfo.imageSubresourceRange.layerCount = imageCreateInfo.arrayLayers;
        std::vector<VkImageLayout> newImageLayouts(restoreInfo->pImageLayouts, restoreInfo->pImageLayouts + imageSubresourceCount);
        transitionInfo.pOldImageLayouts = oldImageLayouts.data();
        transitionInfo.pNewImageLayouts = newImageLayouts.data();
        mCopyEngines[vkDevice].transition_image_layouts(transitionInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkImage_data(const GvkRestorePointObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        Auto<GvkImageRestoreInfo> restoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkImage", to_hex_string(restorePointObject.handle), restoreInfo));
        if (!get_dependency<VkSwapchainKHR>(restoreInfo->dependencyCount, restoreInfo->pDependencies)) {
            auto device = get_dependency<VkDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
            device = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)device, (uint64_t)device }).handle;
            CopyEngine::UploadImageInfo uploadInfo{ };
            uploadInfo.path = (mApplyInfo.path / "VkImage" / to_hex_string(restorePointObject.handle)).replace_extension(".data");
            uploadInfo.device = device;
            uploadInfo.image = (VkImage)get_restored_object(restorePointObject).handle;
            uploadInfo.imageCreateInfo = *restoreInfo->pImageCreateInfo;
            uploadInfo.imageSubresourceRange.aspectMask = get_image_aspect_flags(restoreInfo->pImageCreateInfo->format);
            uploadInfo.imageSubresourceRange.levelCount = restoreInfo->pImageCreateInfo->mipLevels;
            uploadInfo.imageSubresourceRange.layerCount = restoreInfo->pImageCreateInfo->arrayLayers;
            auto imageSubresourceCount = uploadInfo.imageSubresourceRange.levelCount * uploadInfo.imageSubresourceRange.layerCount;
            std::vector<VkImageLayout> oldImageLayouts(imageSubresourceCount, VK_IMAGE_LAYOUT_UNDEFINED);
            std::vector<VkImageLayout> newImageLayouts(restoreInfo->pImageLayouts, restoreInfo->pImageLayouts + imageSubresourceCount);
            uploadInfo.pOldImageLayouts = oldImageLayouts.data();
            uploadInfo.pNewImageLayouts = newImageLayouts.data();
            uploadInfo.pUserData = this;
            uploadInfo.pfnCallback = process_VkImage_data_upload;
            mCopyEngines[device].upload(uploadInfo);
        } else {
            // TODO : Make the layout transition codepath modular...shouldn't need to check
            //  if we're working a swapchain image here...
            process_VkImage_layouts(restorePointObject);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void Applier::process_VkImage_data_upload(const CopyEngine::UploadImageInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (pData) {
            if (std::filesystem::exists(uploadInfo.path)) {
                std::ifstream dataFile(uploadInfo.path, std::ios::binary);
                gvk_result(dataFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                dataFile.read((char*)pData, get_image_data_size(uploadInfo.imageCreateInfo, uploadInfo.imageSubresourceRange));
            }
        } else {
            assert(uploadInfo.pUserData);
            const auto& applier = *(Applier*)uploadInfo.pUserData;
            if (applier.mApplyInfo.pfnProcessResourceDataCallback) {
                GvkStateTrackedObject restorePointObject{ };
                restorePointObject.type = VK_OBJECT_TYPE_IMAGE;
                restorePointObject.handle = (uint64_t)uploadInfo.image;
                restorePointObject.dispatchableHandle = (uint64_t)uploadInfo.device;
                applier.mApplyInfo.pfnProcessResourceDataCallback(&restorePointObject, bindBufferMemoryInfo.memory, get_image_data_size(uploadInfo.imageCreateInfo, uploadInfo.imageSubresourceRange), pData);
            }
        }
    } gvk_result_scope_end;
    assert(gvkResult == VK_SUCCESS);
}

VkResult Applier::process_VkImage_layouts(const GvkRestorePointObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        Auto<GvkImageRestoreInfo> restoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkImage", to_hex_string(restorePointObject.handle), restoreInfo));
        auto device = get_dependency<VkDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
        device = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)device, (uint64_t)device }).handle;
        CopyEngine::TransitionImageLayoutInfo transitionInfo{ };
        transitionInfo.device = device;
        transitionInfo.image = (VkImage)get_restored_object(restorePointObject).handle;
        transitionInfo.imageCreateInfo = *restoreInfo->pImageCreateInfo;
        transitionInfo.imageSubresourceRange.aspectMask = get_image_aspect_flags(restoreInfo->pImageCreateInfo->format);
        transitionInfo.imageSubresourceRange.levelCount = restoreInfo->pImageCreateInfo->mipLevels;
        transitionInfo.imageSubresourceRange.layerCount = restoreInfo->pImageCreateInfo->arrayLayers;
        auto imageSubresourceCount = transitionInfo.imageSubresourceRange.levelCount * transitionInfo.imageSubresourceRange.layerCount;
        std::vector<VkImageLayout> oldImageLayouts(imageSubresourceCount, VK_IMAGE_LAYOUT_UNDEFINED);
        std::vector<VkImageLayout> newImageLayouts(restoreInfo->pImageLayouts, restoreInfo->pImageLayouts + imageSubresourceCount);
        transitionInfo.pOldImageLayouts = oldImageLayouts.data();
        transitionInfo.pNewImageLayouts = newImageLayouts.data();
        mCopyEngines[device].transition_image_layouts(transitionInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk
