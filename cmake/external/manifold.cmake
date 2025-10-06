
include_guard(GLOBAL)

set(MANIFOLD_CROSS_SECTION OFF CACHE BOOL "" FORCE)
set(MANIFOLD_PAR           ON  CACHE BOOL "" FORCE)
set(MANIFOLD_TEST          OFF CACHE BOOL "" FORCE)
set(manifold_VERSION 3a29e9566f60d1021271a1a30ef654697df9eade) # v3.2.1
FetchContent_Declare(
    manifold
    GIT_REPOSITORY "https://github.com/elalish/manifold.git"
    GIT_TAG ${manifold_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(manifold)
set_target_properties(manifold PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")
set_target_properties(tbb PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/")
