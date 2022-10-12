
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG 45ce5ddd197d5c58f50fdd3296a5131c894e5527 # 3.3.7
    GIT_PROGRESS TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED
)
FetchContent_MakeAvailable(glfw)
set_target_properties(glfw PROPERTIES FOLDER "gvk/external/glfw/")
set_target_properties(update_mappings PROPERTIES FOLDER "gvk/external/glfw/")
