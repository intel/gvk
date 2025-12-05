
include_guard(GLOBAL)

include(FindPackageHandleStandardArgs)

################################################################################
# Vulkan SDK
set(gvk-vulkan-sdk "${CMAKE_BINARY_DIR}/cmake/gvk-vulkan-sdk.cmake")
configure_file("${CMAKE_CURRENT_LIST_DIR}/../gvk-vulkan-sdk.cmake.in" "${gvk-vulkan-sdk}" @ONLY)
configure_file("${gvk-vulkan-sdk}" "${CMAKE_BINARY_DIR}/gvk-test-package/gvk-vulkan-sdk.cmake" COPYONLY)
install(FILES "${gvk-vulkan-sdk}" DESTINATION cmake/)
include("${gvk-vulkan-sdk}")

################################################################################
# Vulkan-Headers
if(NOT TARGET Vulkan::Headers)
    FetchContent_Declare(
        Vulkan-Headers
        GIT_REPOSITORY "https://github.com/KhronosGroup/Vulkan-Headers.git"
        GIT_TAG ${Vulkan-Headers_VERSION}
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(Vulkan-Headers)
    FetchContent_GetProperties(Vulkan-Headers SOURCE_DIR Vulkan-Headers_SOURCE_DIR)
endif()

################################################################################
# If configured to omit the Vulkan SDK, setup target to build against
if(NOT TARGET Vulkan::Vulkan)
    add_library(Vulkan::Vulkan INTERFACE IMPORTED)
    set_target_properties(Vulkan::Vulkan PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Vulkan-Headers_SOURCE_DIR}/include/")
    find_library(VULKAN_LIB Vulkan)
    set(Vulkan_XML "${Vulkan-Headers_SOURCE_DIR}/registry/vk.xml" CACHE STRING "" FORCE)
endif()
