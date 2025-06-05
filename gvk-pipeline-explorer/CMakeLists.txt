
################################################################################
# gvk-pipeline-explorer.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-pipeline-explorer/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-pipeline-explorer/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/basic-pipeline-explorer.hpp"
    "${generatedIncludePath}/pipeline-explorer.h"
    "${generatedIncludePath}/pipeline-explorer-enumerations-to-string.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-cerealization.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-comparison-operators.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-create-copy.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-decerealization.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-deserialization.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-destroy-copy.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-get-stype.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-make-tuple.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-serialization.hpp"
    "${generatedIncludePath}/pipeline-explorer-structure-to-string.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/basic-pipeline-explorer.cpp"
    "${generatedSourcePath}/pipeline-explorer-enumerations-to-string.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-comparison-operators.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-create-copy.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-deserialization.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-destroy-copy.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-serialization.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-to-string.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    TARGET
        gvk-pipeline-explorer.generator
    FOLDER
        "gvk-pipeline-explorer/"
    LINK_LIBRARIES
        gvk-cppgen
    INCLUDE_DIRECTORIES
        "${generatorSourcePath}/"
    INCLUDE_FILES
        "${generatorSourcePath}/basic-pipeline-explorer.generator.hpp"
    SOURCE_FILES
        "${generatorSourcePath}/main.cpp"
    OUTPUT_FILES
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PREFIX="gvk-pipeline-explorer/generated/"
        GVK_PIPELINE_EXPLORER_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk-pipeline-explorer
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-pipeline-explorer/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-pipeline-explorer/")
gvk_add_static_library(
    TARGET
        gvk-pipeline-explorer
    FOLDER
        "gvk-pipeline-explorer/"
    LINK_LIBRARIES
        gvk-command-structures
        gvk-containers
        gvk-handles
        gvk-layer
        gvk-reference
        gvk-runtime
        gvk-spirv
        gvk-structures
        gvk-system
        boost-multiprecision
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/cmd-tracker.hpp"
        "${includePath}/device-address-tracker.hpp"
        "${includePath}/handle-info.hpp"
        "${includePath}/pipeline-explorer.hpp"
        "${includePath}/utilities.hpp"
        "${includeDirectory}/gvk-pipeline-explorer.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/detail/to-string-manual.cpp"
        "${sourcePath}/buffer.cpp"
        "${sourcePath}/cmd-tracker.cpp"
        "${sourcePath}/command-buffer.cpp"
        "${sourcePath}/descriptor-set-layout.cpp"
        "${sourcePath}/device-memory.cpp"
        "${sourcePath}/device-address-tracker.cpp"
        "${sourcePath}/device.cpp"
        "${sourcePath}/instance.cpp"
        "${sourcePath}/pipeline-layout.cpp"
        "${sourcePath}/pipeline.cpp"
        "${sourcePath}/pipeline-explorer.cpp"
        "${sourcePath}/queue.cpp"
        "${sourcePath}/render-pass.cpp"
        "${sourcePath}/sampler.cpp"
        "${sourcePath}/shader-module.cpp"
        "${sourcePath}/utilities.cpp"
)

################################################################################
# gvk-pipeline-explorer-gui
gvk_add_executable(
    TARGET
        gvk-pipeline-explorer-gui
    FOLDER
        "gvk-pipeline-explorer/"
    LINK_LIBRARIES
        gvk-gui
        gvk-pipeline-explorer
    INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_LIST_DIR}/gui/"
    INCLUDE_FILES
        "${includePath}/gui/console-window.hpp"
        "${includePath}/gui/file-window.hpp"
        "${includePath}/gui/gui-info.hpp"
        "${includePath}/gui/metrics-window.hpp"
        "${includePath}/gui/pipelines-window.hpp"
        "${includePath}/gui/selected-pipeline-window.hpp"
        "${includePath}/gui/window-manager.hpp"
        "${includePath}/gui/window.hpp"
        "${includePath}/gui/workspace-window.hpp"
    SOURCE_FILES
        "${sourcePath}/gui/console-window.cpp"
        "${sourcePath}/gui/file-window.cpp"
        "${sourcePath}/gui/main.cpp"
        "${sourcePath}/gui/metrics-window.cpp"
        "${sourcePath}/gui/pipelines-window.cpp"
        "${sourcePath}/gui/selected-pipeline-window.cpp"
        "${sourcePath}/gui/window-manager.cpp"
        "${sourcePath}/gui/window.cpp"
        "${sourcePath}/gui/workspace-window.cpp"
)

################################################################################
# VK_LAYER_INTEL_gvk_pipeline_explorer
gvk_add_layer(
    TARGET
        VK_LAYER_INTEL_gvk_pipeline_explorer
    FOLDER
        "gvk-pipeline-explorer/"
    LINK_LIBRARIES
        gvk-pipeline-explorer
    SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/VK_LAYER_INTEL_gvk_pipeline_explorer.cpp"
    DESCRIPTION
        "Intel(R) GPA Utilities for Vulkan* Pipeline Explorer"
)

################################################################################
# gvk-pipeline-explorer install
if(gvk-pipeline-explorer_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-pipeline-explorer)
    gvk_install_artifacts(TARGET gvk-pipeline-explorer-gui)
    gvk_install_layer(TARGET VK_LAYER_INTEL_gvk_pipeline_explorer)
endif()
if(gvk-pipeline-explorer_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-pipeline-explorer)
endif()
