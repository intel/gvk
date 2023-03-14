
include_guard()

include(FetchContent)

set(VMA_STATIC_VULKAN_FUNCTIONS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git"
    GIT_TAG a6bfc237255a6bac1513f7c1ebde6d8aed6b5191 # 3.0.1
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)
FetchContent_GetProperties(VulkanMemoryAllocator SOURCE_DIR VulkanMemoryAllocatorSourceDirectory)
set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")
