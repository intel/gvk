
include_guard(GLOBAL)
gvk_enable_target(glfw)

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(glfw_VERSION 7b6aead9fb88b3623e3b3725ebb42670cbe4c579) # 3.4
FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG ${glfw_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(glfw)
FetchContent_GetProperties(glfw SOURCE_DIR glfw_SOURCE_DIR)
set(folder "${GVK_IDE_FOLDER}/external/glfw/")
set_target_properties(glfw PROPERTIES FOLDER "${folder}")
set_target_properties(update_mappings PROPERTIES FOLDER "${folder}")

if(gvk-glfw_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET glfw VERSION ${glfw_VERSION})
endif()
if(gvk-glfw_INSTALL_HEADERS)
    install(DIRECTORY "${glfw_SOURCE_DIR}/include/GLFW/" DESTINATION include/GLFW/)
endif()
