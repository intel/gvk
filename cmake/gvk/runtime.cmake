
include_guard(GLOBAL)
gvk_enable_target(gvk-runtime)

include(gvk/cppgen)
include(gvk/string)
include(gvk/xml)
include(external/Vulkan)

if(gvk-build-tests)
    include(external/asio)
endif()

add_subdirectory(gvk-runtime)
