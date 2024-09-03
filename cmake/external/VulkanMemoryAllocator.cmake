
include_guard(GLOBAL)
gvk_enable_target(VulkanMemoryAllocator)

set(VMA_STATIC_VULKAN_FUNCTIONS OFF CACHE BOOL "" FORCE)
set(VulkanMemoryAllocator_VERSION 009ecd192c1289c7529bff248a16cfe896254816) # 3.1.0
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git"
    GIT_TAG ${VulkanMemoryAllocator_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_Populate(VulkanMemoryAllocator)
FetchContent_GetProperties(VulkanMemoryAllocator SOURCE_DIR VulkanMemoryAllocator_SOURCE_DIR)
add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator INTERFACE "$<BUILD_INTERFACE:${VulkanMemoryAllocator_SOURCE_DIR}/include/>" $<INSTALL_INTERFACE:include>)

if(gvk-VulkanMemoryAllocator_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET VulkanMemoryAllocator VERSION ${VulkanMemoryAllocator_VERSION})
endif()
if(gvk-VulkanMemoryAllocator_INSTALL_HEADERS)
    gvk_install_headers(TARGET VulkanMemoryAllocator)
endif()
