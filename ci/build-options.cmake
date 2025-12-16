
################################################################################
# Vulkan SDK options
set(gvk-Vulkan-SDK_VERSION 1.4.335.0 CACHE STRING "")
set(gvk-Vulkan-SDK_URL_LINUX
    "https://dependency.server.com/vulkan/sdk/linux"
    "https://sdk.lunarg.com/sdk/download/${gvk-Vulkan-SDK_VERSION}/linux"
    CACHE STRING "" FORCE
)
set(gvk-Vulkan-SDK_URL_WIN32
    "https://dependency.server.com/vulkan/sdk/windows"
    "https://sdk.lunarg.com/sdk/download/${gvk-Vulkan-SDK_VERSION}/windows"
    CACHE STRING "" FORCE
)

################################################################################
# Default options
set(gvk-default_ENABLED           OFF CACHE BOOL "" FORCE)
set(gvk-default_INSTALL_ARTIFACTS OFF CACHE BOOL "" FORCE)
set(gvk-default_INSTALL_HEADERS   OFF CACHE BOOL "" FORCE)

macro(gvk_set_build_options module enabled installArtifacts installHeaders)
    set(${module}_ENABLED           ${enabled}          CACHE BOOL "" FORCE)
    set(${module}_INSTALL_ARTIFACTS ${installArtifacts} CACHE BOOL "" FORCE)
    set(${module}_INSTALL_HEADERS   ${installHeaders}   CACHE BOOL "" FORCE)
endmacro()

################################################################################
# External modules
gvk_set_build_options(boost-asio                ON  ON  OFF)
gvk_set_build_options(boost-multiprecision      ON  ON  ON )
gvk_set_build_options(cereal                    ON  ON  OFF)
gvk_set_build_options(glfw                      ON  ON  OFF)
gvk_set_build_options(glm                       ON  OFF OFF)
gvk_set_build_options(glslang                   ON  ON  OFF)
gvk_set_build_options(imgui                     ON  OFF OFF)
gvk_set_build_options(SPIRV-Cross               ON  ON  ON )
gvk_set_build_options(SPIRV-Headers             ON  ON  OFF)
gvk_set_build_options(SPIRV-Tools               ON  ON  OFF)
gvk_set_build_options(stb                       ON  ON  OFF)
gvk_set_build_options(tinyxml2                  ON  ON  ON )
gvk_set_build_options(Vulkan                    ON  OFF OFF)
gvk_set_build_options(VulkanMemoryAllocator     ON  ON  ON )

################################################################################
# GVK modules
gvk_set_build_options(gvk-command-structures    ON  ON  ON )
gvk_set_build_options(gvk-containers            ON  ON  ON )
gvk_set_build_options(gvk-cppgen                ON  OFF OFF)
gvk_set_build_options(gvk-format-info           ON  ON  ON )
gvk_set_build_options(gvk-gui                   ON  OFF OFF)
gvk_set_build_options(gvk-handles               ON  ON  ON )
gvk_set_build_options(gvk-layer                 ON  ON  ON )
gvk_set_build_options(gvk-math                  ON  OFF OFF)
gvk_set_build_options(gvk-pipeline-explorer     ON  ON  ON )
gvk_set_build_options(gvk-reference             ON  ON  ON )
gvk_set_build_options(gvk-restore-info          ON  ON  ON )
gvk_set_build_options(gvk-restore-point         ON  ON  ON )
gvk_set_build_options(gvk-runtime               ON  ON  ON )
gvk_set_build_options(gvk-spirv                 ON  ON  ON )
gvk_set_build_options(gvk-state-tracker         ON  ON  ON )
gvk_set_build_options(gvk-string                ON  ON  ON )
gvk_set_build_options(gvk-structures            ON  ON  ON )
gvk_set_build_options(gvk-system                ON  ON  ON )
gvk_set_build_options(gvk-virtual-swapchain     ON  ON  OFF)
gvk_set_build_options(gvk-xml                   ON  ON  ON )

################################################################################
# GVK samples and tests
set(gvk-build-samples ON  CACHE BOOL "")
set(gvk-build-tests   ON  CACHE BOOL "")
set(gvk-run-tests     OFF CACHE BOOL "")
