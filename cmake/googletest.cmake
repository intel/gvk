
FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850
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
