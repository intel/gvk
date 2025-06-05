
include_guard(GLOBAL)

set(VMA_STATIC_VULKAN_FUNCTIONS OFF CACHE BOOL "" FORCE)
set(VulkanMemoryAllocator_VERSION 1d8f600fd424278486eade7ed3e877c99f0846b1) # 3.3.0
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

if(VulkanMemoryAllocator_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET VulkanMemoryAllocator VERSION ${VulkanMemoryAllocator_VERSION})
endif()
if(VulkanMemoryAllocator_INSTALL_HEADERS)
    gvk_install_headers(TARGET VulkanMemoryAllocator)
endif()
