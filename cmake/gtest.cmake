
FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG b1f84bf1763b1010597bff13c79b5388eebdf205
    GIT_PROGRESS TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
set(folder "gvk/external/gtest/")
set_target_properties(gmock PROPERTIES FOLDER "${folder}")
set_target_properties(gmock_main PROPERTIES FOLDER "${folder}")
set_target_properties(gtest PROPERTIES FOLDER "${folder}")
set_target_properties(gtest_main PROPERTIES FOLDER "${folder}")
include(GoogleTest)
