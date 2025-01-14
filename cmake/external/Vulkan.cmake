
include_guard(GLOBAL)

################################################################################
# Vulkan SDK version, URLs, and hashes
set(Vulkan-SDK_VERSION 1.3.296.0)
set(Vulkan-SDK_LINUX_URL "https://sdk.lunarg.com/sdk/download/${Vulkan-SDK_VERSION}/linux/vulkansdk-linux-x86_64-${Vulkan-SDK_VERSION}.tar.xz")
set(Vulkan-SDK_LINUX_SHA256 79b0a1593dadc46180526250836f3e53688a9a5fb42a0e5859eb72316dc4d53e)
set(Vulkan-SDK_WINDOWS_URL "https://sdk.lunarg.com/sdk/download/${Vulkan-SDK_VERSION}/windows/VulkanSDK-${Vulkan-SDK_VERSION}-Installer.exe")
set(Vulkan-SDK_WINDOWS_SHA256 acb4ae0786fd3e558f8b3c36cc3eba91638984217ba8a6795ec64d2f9ffd8c4b)

################################################################################
# Check for installed Vulkan SDK, if not found download
find_package(Vulkan ${Vulkan-SDK_VERSION} EXACT)
if(NOT Vulkan_FOUND OR NOT DEFINED ENV{VULKAN_SDK})
    unset(Vulkan_FOUND CACHE)
    if(LINUX)
        FetchContent_Declare(Vulkan-SDK URL ${Vulkan-SDK_LINUX_URL} URL_HASH SHA256=${Vulkan-SDK_LINUX_SHA256})
        FetchContent_MakeAvailable(Vulkan-SDK)
        FetchContent_GetProperties(Vulkan-SDK SOURCE_DIR Vulkan-SDK_SOURCE_DIR)
        set(ENV{VULKAN_SDK} "${Vulkan-SDK_SOURCE_DIR}/x86_64")
    elseif(WIN32)
        FetchContent_Declare(Vulkan-SDK URL ${Vulkan-SDK_WINDOWS_URL} URL_HASH SHA256=${Vulkan-SDK_WINDOWS_SHA256} DOWNLOAD_NO_EXTRACT ON)
        FetchContent_MakeAvailable(Vulkan-SDK)
        FetchContent_GetProperties(Vulkan-SDK SOURCE_DIR Vulkan-SDK_SOURCE_DIR)
        if(NOT EXISTS "${Vulkan-SDK_SOURCE_DIR}/${Vulkan-SDK_VERSION}/")
            set(cmd "${Vulkan-SDK_SOURCE_DIR}/VulkanSDK-${Vulkan-SDK_VERSION}-Installer.exe" --root "${Vulkan-SDK_SOURCE_DIR}/${Vulkan-SDK_VERSION}/" --accept-licenses --default-answer --confirm-command install copy_only=1)
            execute_process(COMMAND ${cmd} WORKING_DIRECTORY "${Vulkan-SDK_SOURCE_DIR}")
        endif()
        set(ENV{VULKAN_SDK} "${Vulkan-SDK_SOURCE_DIR}/${Vulkan-SDK_VERSION}")
    endif()
    find_package(Vulkan ${Vulkan-SDK_VERSION} EXACT REQUIRED)
endif()

################################################################################
# Normalize path ENV{VULKAN_SDK}
set(VULKAN_SDK $ENV{VULKAN_SDK})
string(REPLACE "\\" "/" VULKAN_SDK ${VULKAN_SDK})
set(ENV{VULKAN_SDK} ${VULKAN_SDK})

################################################################################
# Set Vulkan_XML
set(Vulkan_XML "$ENV{VULKAN_SDK}/share/vulkan/registry/vk.xml" CACHE STRING "" FORCE)
string(REPLACE "//" "/" Vulkan_XML ${Vulkan_XML})

################################################################################
# Set Vulkan_VERSION
set(vulkan-api-version_SOURCE_DIR "${PROJECT_BINARY_DIR}/vulkan-api-version/")
set(vulkan-api-version_BINARY_DIR "${vulkan-api-version_SOURCE_DIR}/bin/")
file(WRITE "${vulkan-api-version_SOURCE_DIR}/vulkan-api-version.cpp"
"
#include \"vulkan/vulkan.h\"
#include <cstdio>
int main(int argc, char* argv[])
{
    printf(
        \"%d.%d.%d\",
        VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
        VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
        VK_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE)
    );
    return 0;
}
"
)
set(ENV{VK_LOADER_DEBUG} "")
set(ENV{VK_INSTANCE_LAYERS} "")
try_run(
    runResult
    compileResult
    "${vulkan-api-version_BINARY_DIR}"
    "${vulkan-api-version_SOURCE_DIR}/vulkan-api-version.cpp"
    LINK_LIBRARIES Vulkan::Vulkan
    COMPILE_OUTPUT_VARIABLE compileOutput
    RUN_OUTPUT_VARIABLE Vulkan_VERSION
)
if(NOT compileResult)
    message(FATAL_ERROR "Failed to compile vulkan-api-version\n${compileOutput}")
endif()
if(runResult)
    message(FATAL_ERROR "Failed to execute vulkan-api-version\n${runResult} : ${Vulkan_VERSION}")
endif()
string(REPLACE "." ";" vukanVersionValues "${Vulkan_VERSION}")
list(GET vukanVersionValues 0 Vulkan_VERSION_MAJOR)
list(GET vukanVersionValues 1 Vulkan_VERSION_MINOR)
list(GET vukanVersionValues 2 Vulkan_VERSION_PATCH)
