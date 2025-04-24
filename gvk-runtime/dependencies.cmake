
include_guard(GLOBAL)
gvk_enable_module(gvk-cppgen)
gvk_enable_module(gvk-string)
gvk_enable_module(gvk-xml)
gvk_enable_external_module(Vulkan)
if(gvk-build-tests)
    gvk_enable_external_module(boost-asio)
endif()
