
include_guard(GLOBAL)

set(fetchContentQuiet ${FETCHCONTENT_QUIET})
set(FETCHCONTENT_QUIET FALSE)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# NOTE : GVK uses boost.asio, but that module is being pulled standalone.  There
#   is a potential bug in boost.asio, so GVK is locked onto an older version of
#   boost.asio until resolved.
set(CMAKE_INSTALL_LIBDIR         lib/$<CONFIG>/                 CACHE STRING "" FORCE)
set(BOOST_INCLUDE_LIBRARIES      geometry multiprecision python CACHE STRING "" FORCE)
set(BOOST_SKIP_INSTALL_RULES     OFF                            CACHE BOOL   "" FORCE)
set(BUILD_SHARED_LIBS            OFF                            CACHE BOOL   "" FORCE)
set(BOOST_ENABLE_CMAKE           ON                             CACHE BOOL   "" FORCE)
set(BOOST_ENABLE_PYTHON          ON                             CACHE BOOL   "" FORCE)
set(BOOST_EXPORT_DEPENDENCIES    ON                             CACHE BOOL   "" FORCE)
set(BOOST_INSTALL_INCLUDE_SUBDIR ""                             CACHE STRING "" FORCE)
set(BOOST_INSTALL_CMAKEDIR       cmake                          CACHE STRING "" FORCE)
set(boost_VERSION boost-1.88.0)
FetchContent_Declare(
    boost
    GIT_REPOSITORY "https://github.com/boostorg/boost.git"
    GIT_TAG ${boost_VERSION}
    GIT_PROGRESS TRUE
)

set(FETCHCONTENT_QUIET ${fetchContentQuiet})
FetchContent_MakeAvailable(boost)

gvk_get_directory_targets(${boost_SOURCE_DIR} boostTargets)
foreach(boostTarget ${boostTargets})
    string(REPLACE "::" ";" boostTarget ${boostTarget})
    list(LENGTH boostTarget length)
    if(length EQUAL 2)
        list(GET boostTarget 0 boostDirectory)
        list(GET boostTarget 1 boostTarget)
        string(REPLACE "${boost_SOURCE_DIR}" "" boostDirectory "${boostDirectory}")
        set_target_properties(${boostTarget} PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/boost/")
    else()
        message(FATAL_ERROR "${boostTarget} has nonstandard directory structure; standalone configuration required")
    endif()
endforeach()
