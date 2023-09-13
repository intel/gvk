
include_guard()

include(FetchContent)

set(glm_VERSION bf71a834948186f4097caa076cd2663c69a10e1e) # 0.9.9.8
FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG ${glm_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)
