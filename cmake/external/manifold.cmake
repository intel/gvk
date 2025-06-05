
include_guard(GLOBAL)

set(MANIFOLD_CROSS_SECTION OFF CACHE BOOL "" FORCE)
set(MANIFOLD_PAR           ON  CACHE BOOL "" FORCE)
set(MANIFOLD_TEST          OFF CACHE BOOL "" FORCE)
set(manifold_VERSION 1ec20369ca7655a2b1f7a37d0ad2e33bec38bd5e) # master 1 May 2025
FetchContent_Declare(
    manifold
    GIT_REPOSITORY "https://github.com/elalish/manifold.git"
    GIT_TAG ${manifold_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(manifold)
set_target_properties(manifold PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")
set_target_properties(tbb PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")
