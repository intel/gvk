
include_guard()

include(FetchContent)

set(glm_VERSION 0af55ccecd98d4e5a8d1fad7de25ba429d60e863) # 1.0.1
FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG ${glm_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)
set_target_properties(glm PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")
