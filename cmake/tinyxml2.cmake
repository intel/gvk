
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    tinyxml2 GIT_REPOSITORY
    "https://github.com/leethomason/tinyxml2.git"
    GIT_TAG 1dee28e51f9175a31955b9791c74c430fe13dc82 # 9.0.0
    GIT_PROGRESS TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED
)
FetchContent_MakeAvailable(tinyxml2)
set(folder "gvk/external/tinyxml2/")
set_target_properties(tinyxml2 PROPERTIES FOLDER "${folder}")
set_target_properties(xmltest PROPERTIES FOLDER "${folder}")
