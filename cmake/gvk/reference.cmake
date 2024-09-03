
include_guard(GLOBAL)
gvk_enable_target(gvk-reference)

if(gvk-build-tests)
    include(external/asio)
endif()

add_subdirectory(gvk-reference)
