
include_guard()

include(FetchContent)

set(JUST_INSTALL_CEREAL ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    cereal
    GIT_REPOSITORY "https://github.com/USCiLab/cereal.git"
    GIT_TAG ebef1e929807629befafbb2918ea1a08c7194554 # 1.3.2
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(cereal)
FetchContent_GetProperties(cereal SOURCE_DIR cerealSourceDirectory)
