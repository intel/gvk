
include_guard(GLOBAL)

################################################################################
# Vulkan SDK version, URLs, and hashes
set(Vulkan-SDK_VERSION 1.4.304.0)
set(Vulkan-SDK_LINUX_URL "https://sdk.lunarg.com/sdk/download/${Vulkan-SDK_VERSION}/linux/vulkansdk-linux-x86_64-${Vulkan-SDK_VERSION}.tar.xz")
set(Vulkan-SDK_LINUX_SHA256 d9f9246353a38e432548d866758d11381ebfc208a3e4d80921988b1533a344f7)
set(Vulkan-SDK_WINDOWS_URL "https://sdk.lunarg.com/sdk/download/${Vulkan-SDK_VERSION}/windows/VulkanSDK-${Vulkan-SDK_VERSION}-Installer.exe")
set(Vulkan-SDK_WINDOWS_SHA256 6a25ee4f2fa880eee3e1b3ac47ac93d10ac1ba459cca3bbbdad049f42d5469f5)

################################################################################
# Check for installed Vulkan SDK
if(DEFINED ENV{VULKAN_SDK})
    # NOTE : It's kinda cumbersome to be using REGEX to find the Vulkan SDK version
    #   in random files in the SDK that aren't defined by any schema/specification.
    #   On Linux the README is used and on Windows the Qt installer components.xml
    #   is used.  Unfortunately there doesn't really seem to be a reliable way to
    #   ascertain what version of the Vulkan SDK is in use.  It is possible to get
    #   the value of VK_HEADER_VERSION_COMPLETE from vulkan_core.h, but this isn't
    #   the full SDK version, for example...
    #       For Vulkan SDK 1.4.304.0, VK_HEADER_VERSION_COMPLETE yields 0.1.4.304
    #       (note that a non-zero leading digit indicates a variant, eg. Vulkan SC)
    if(LINUX)
        set(Vulkan-SDK_README "$ENV{VULKAN_SDK}/README.txt")
        if(EXISTS "${Vulkan-SDK_README}")
            file(STRINGS "${Vulkan-SDK_README}" Vulkan-SDK_FOUND_VERSION REGEX "release_notes.html")
            string(REPLACE "Release Notes: https://vulkan.lunarg.com/doc/sdk/" "" Vulkan-SDK_FOUND_VERSION ${Vulkan-SDK_FOUND_VERSION})
            string(REPLACE "/linux/release_notes.html" "" Vulkan-SDK_FOUND_VERSION ${Vulkan-SDK_FOUND_VERSION})
            string(STRIP ${Vulkan-SDK_FOUND_VERSION} Vulkan-SDK_FOUND_VERSION)
        endif()
    elseif(WIN32)
        set(Vulkan-SDK_COMPONENTS_XML "$ENV{VULKAN_SDK}/components.xml")
        if(EXISTS "${Vulkan-SDK_COMPONENTS_XML}")
            file(STRINGS "${Vulkan-SDK_COMPONENTS_XML}" Vulkan-SDK_FOUND_VERSION REGEX "<ApplicationName>")
            string(REPLACE "<ApplicationName>" "" Vulkan-SDK_FOUND_VERSION ${Vulkan-SDK_FOUND_VERSION})
            string(REPLACE "Vulkan SDK" "" Vulkan-SDK_FOUND_VERSION ${Vulkan-SDK_FOUND_VERSION})
            string(REPLACE "</ApplicationName>" "" Vulkan-SDK_FOUND_VERSION ${Vulkan-SDK_FOUND_VERSION})
            string(STRIP ${Vulkan-SDK_FOUND_VERSION} Vulkan-SDK_FOUND_VERSION)
        endif()
    endif()
endif()

################################################################################
# Download Vulkan SDK if the required version isn't installed
if(NOT Vulkan-SDK_VERSION STREQUAL Vulkan-SDK_FOUND_VERSION)
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
    set(VULKAN_SDK "$ENV{VULKAN_SDK}")
    string(REPLACE "\\" "/" VULKAN_SDK "${VULKAN_SDK}")
    string(REPLACE "//" "/" VULKAN_SDK "${VULKAN_SDK}")
    set(ENV{VULKAN_SDK} "${VULKAN_SDK}")
endif()

################################################################################
# Find package and set Vulkan_XML
find_package(Vulkan ${Vulkan-SDK_VERSION} EXACT)
set(Vulkan_XML "$ENV{VULKAN_SDK}/share/vulkan/registry/vk.xml" CACHE STRING "" FORCE)
string(REPLACE "//" "/" Vulkan_XML "${Vulkan_XML}")
