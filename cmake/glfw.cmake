
include_guard()

include(FetchContent)

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(glfw_VERSION e2c92645460f680fd272fd2eed591efb2be7dc31) # 3.3.9
FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG ${glfw_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glfw)
set(folder "${GVK_IDE_FOLDER}/external/glfw/")
set_target_properties(glfw PROPERTIES FOLDER "${folder}")
set_target_properties(update_mappings PROPERTIES FOLDER "${folder}")
