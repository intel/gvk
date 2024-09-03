
include_guard(GLOBAL)
gvk_enable_target(cereal)

set(JUST_INSTALL_CEREAL ON CACHE BOOL "" FORCE)
set(cereal_VERSION ebef1e929807629befafbb2918ea1a08c7194554) # 1.3.2
FetchContent_Declare(
    cereal
    GIT_REPOSITORY "https://github.com/USCiLab/cereal.git"
    GIT_TAG ${cereal_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(cereal)
FetchContent_GetProperties(cereal SOURCE_DIR cereal_SOURCE_DIR)

if(gvk-cereal_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET cereal VERSION ${cereal_VERSION})
endif()
if(gvk-cereal_INSTALL_HEADERS)
    install(DIRECTORY "${cereal_SOURCE_DIR}/include/cereal/" DESTINATION include/cereal/)
endif()
