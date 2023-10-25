
include_guard()

include(FetchContent)

set(JUST_INSTALL_CEREAL ON CACHE BOOL "" FORCE)
set(cereal_VERSION ebef1e929807629befafbb2918ea1a08c7194554) # 1.3.2
FetchContent_Declare(
    cereal
    GIT_REPOSITORY "https://github.com/USCiLab/cereal.git"
    GIT_TAG ${cereal_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(cereal)
gvk_install_artifacts(TARGET cereal VERSION ${cereal_VERSION})
