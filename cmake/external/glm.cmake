

include_guard(GLOBAL)
gvk_enable_target(glm)

set(glm_VERSION 0af55ccecd98d4e5a8d1fad7de25ba429d60e863) # 1.0.1
FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG ${glm_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(glm)
set_target_properties(glm PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")

if(gvk-glm_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET glm-header-only VERSION ${glm_VERSION})
endif()
if(gvk-glm_INSTALL_HEADERS)
    install(DIRECTORY "${glm_SOURCE_DIR}/glm/" DESTINATION include/glm/)
endif()
