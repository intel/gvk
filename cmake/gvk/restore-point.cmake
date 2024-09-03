
include_guard(GLOBAL)
gvk_enable_target(gvk-restore-point)

include(gvk/cppgen)
include(gvk/command-structures)
include(gvk/format-info)
include(gvk/handles)
include(gvk/reference)
include(gvk/restore-info)
include(gvk/runtime)
include(gvk/state-tracker)
include(external/asio)
include(external/stb)

add_subdirectory(gvk-restore-point)
