
include_guard(GLOBAL)
include(CMakePackageConfigHelpers)
set(gvk-vulkan-sdk "${CMAKE_BINARY_DIR}/cmake/gvk-vulkan-sdk.cmake")
configure_file("${CMAKE_CURRENT_LIST_DIR}/../gvk-vulkan-sdk.cmake.in" "${gvk-vulkan-sdk}" @ONLY)
configure_file("${gvk-vulkan-sdk}" "${CMAKE_BINARY_DIR}/gvk-test-package/gvk-vulkan-sdk.cmake" COPYONLY)
install(FILES "${gvk-vulkan-sdk}" DESTINATION cmake/)
include("${gvk-vulkan-sdk}")
