
find_package(Vulkan REQUIRED)

################################################################################
# Set Vulkan_SDK_DIR and Vulkan_XML
if(MSVC)
    set(Vulkan_SDK_DIR "${Vulkan_INCLUDE_DIRS}/../")
else()
    set(Vulkan_SDK_DIR "$ENV{VULKAN_SDK}")
endif()
set(Vulkan_XML "${Vulkan_SDK_DIR}/share/vulkan/registry/vk.xml" CACHE STRING "" FORCE)

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
