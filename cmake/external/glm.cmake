

include_guard(GLOBAL)

set(glm_VERSION a532f5b1cf27d6a3c099437e6959cf7e398a0a67) # 1.0.2
FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG ${glm_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(glm)
set_target_properties(glm PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")

if(glm_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET glm-header-only VERSION ${glm_VERSION})
endif()
if(glm_INSTALL_HEADERS)
    install(DIRECTORY "${glm_SOURCE_DIR}/glm/" DESTINATION include/glm/)
endif()
