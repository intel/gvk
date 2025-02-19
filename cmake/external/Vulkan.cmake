
include_guard(GLOBAL)

################################################################################
# Set Vulkan SDK version, URLs, and hashes
set(Vulkan-SDK_VERSION 1.4.304.0)
set(Vulkan-SDK_LINUX_URL "https://sdk.lunarg.com/sdk/download/${Vulkan-SDK_VERSION}/linux/vulkansdk-linux-x86_64-${Vulkan-SDK_VERSION}.tar.xz")
set(Vulkan-SDK_LINUX_SHA256 d9f9246353a38e432548d866758d11381ebfc208a3e4d80921988b1533a344f7)
set(Vulkan-SDK_WINDOWS_URL "https://sdk.lunarg.com/sdk/download/${Vulkan-SDK_VERSION}/windows/VulkanSDK-${Vulkan-SDK_VERSION}-Installer.exe")
set(Vulkan-SDK_WINDOWS_SHA256 6a25ee4f2fa880eee3e1b3ac47ac93d10ac1ba459cca3bbbdad049f42d5469f5)

################################################################################
# Check installed Vulkan SDK version
if(DEFINED ENV{VULKAN_SDK})
    # FROM : https://github.com/Kitware/CMake/blob/master/Modules/FindVulkan.cmake
    set(VULKAN_CORE_H "$ENV{VULKAN_SDK}/include/vulkan/vulkan_core.h")
    if(EXISTS ${VULKAN_CORE_H})
        file(STRINGS  ${VULKAN_CORE_H} VulkanHeaderVersionLine REGEX "^#define VK_HEADER_VERSION ")
        string(REGEX MATCHALL "[0-9]+" VulkanHeaderVersion "${VulkanHeaderVersionLine}")
        file(STRINGS  ${VULKAN_CORE_H} VulkanHeaderVersionLine2 REGEX "^#define VK_HEADER_VERSION_COMPLETE ")
        string(REGEX MATCHALL "[0-9]+" VulkanHeaderVersion2 "${VulkanHeaderVersionLine2}")
        list(LENGTH VulkanHeaderVersion2 _len)
        #  versions >= 1.2.175 have an additional numbers in front of e.g. '0, 1, 2' instead of '1, 2'
        if(_len EQUAL 3)
            list(REMOVE_AT VulkanHeaderVersion2 0)
        endif()
        list(APPEND VulkanHeaderVersion2 ${VulkanHeaderVersion})
        list(JOIN VulkanHeaderVersion2 "." Vulkan-SDK_FOUND_VERSION)
    endif()
    unset(VULKAN_CORE_H)
endif()

################################################################################
# Download Vulkan SDK if the required version isn't installed
if(NOT Vulkan-SDK_VERSION VERSION_EQUAL Vulkan-SDK_FOUND_VERSION)
    if(LINUX)
        FetchContent_Declare(Vulkan-SDK URL ${Vulkan-SDK_LINUX_URL} URL_HASH SHA256=${Vulkan-SDK_LINUX_SHA256})
        FetchContent_MakeAvailable(Vulkan-SDK)
        FetchContent_GetProperties(Vulkan-SDK SOURCE_DIR Vulkan-SDK_SOURCE_DIR)
        set(ENV{VULKAN_SDK} "${Vulkan-SDK_SOURCE_DIR}/x86_64/")
    elseif(WIN32)
        FetchContent_Declare(Vulkan-SDK URL ${Vulkan-SDK_WINDOWS_URL} URL_HASH SHA256=${Vulkan-SDK_WINDOWS_SHA256} DOWNLOAD_NO_EXTRACT ON)
        FetchContent_MakeAvailable(Vulkan-SDK)
        FetchContent_GetProperties(Vulkan-SDK SOURCE_DIR Vulkan-SDK_SOURCE_DIR)
        if(NOT EXISTS "${Vulkan-SDK_SOURCE_DIR}/${Vulkan-SDK_VERSION}/")
            set(cmd "${Vulkan-SDK_SOURCE_DIR}/VulkanSDK-${Vulkan-SDK_VERSION}-Installer.exe" --root "${Vulkan-SDK_SOURCE_DIR}/${Vulkan-SDK_VERSION}/" --accept-licenses --default-answer --confirm-command install copy_only=1)
            execute_process(COMMAND ${cmd} WORKING_DIRECTORY "${Vulkan-SDK_SOURCE_DIR}")
        endif()
        set(ENV{VULKAN_SDK} "${Vulkan-SDK_SOURCE_DIR}/${Vulkan-SDK_VERSION}/")
    endif()
endif()

################################################################################
# Normalize SDK path
set(VULKAN_SDK "$ENV{VULKAN_SDK}")
string(REPLACE "\\" "/" VULKAN_SDK "${VULKAN_SDK}")
string(REPLACE "//" "/" VULKAN_SDK "${VULKAN_SDK}")
set(ENV{VULKAN_SDK} "${VULKAN_SDK}")

################################################################################
# Find package and set Vulkan_XML
find_package(Vulkan ${Vulkan-SDK_VERSION} EXACT)
set(Vulkan_XML "$ENV{VULKAN_SDK}/share/vulkan/registry/vk.xml" CACHE STRING "" FORCE)
