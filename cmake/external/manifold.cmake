
include_guard(GLOBAL)

set(MANIFOLD_CROSS_SECTION OFF CACHE BOOL "" FORCE)
set(MANIFOLD_PAR           ON  CACHE BOOL "" FORCE)
set(MANIFOLD_TEST          OFF CACHE BOOL "" FORCE)
set(manifold_VERSION 798d83c8d7fabcddd23c1617097b95ba40f2597c) # v3.3.2
FetchContent_Declare(
    manifold
    GIT_REPOSITORY "https://github.com/elalish/manifold.git"
    GIT_TAG ${manifold_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(manifold)
set_target_properties(manifold PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")
set_target_properties(tbb PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")
