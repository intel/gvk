
################################################################################
# gvk-pipeline-explorer.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-pipeline-explorer/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-pipeline-explorer/generated/")
set(infoGeneratedIncludeFiles
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
set(infoGeneratedSourceFiles
    "${generatedSourcePath}/pipeline-explorer-enumerations-to-string.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-comparison-operators.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-create-copy.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-deserialization.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-destroy-copy.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-serialization.cpp"
    "${generatedSourcePath}/pipeline-explorer-structure-to-string.cpp"
)
set(backendGeneratedIncludeFiles
    "${generatedIncludePath}/basic-pipeline-explorer.hpp"
)
set(backendGeneratedSourceFiles
    "${generatedSourcePath}/basic-pipeline-explorer.cpp"
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
        "${backendGeneratedIncludeFiles}"
        "${backendGeneratedSourceFiles}"
        "${infoGeneratedIncludeFiles}"
        "${infoGeneratedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_PIPELINE_EXPLORER_GENERATED_INCLUDE_PREFIX="gvk-pipeline-explorer/generated/"
        GVK_PIPELINE_EXPLORER_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk-pipeline-explorer components
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-pipeline-explorer/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-pipeline-explorer/")
include(gvk-pipeline-explorer-backend.cmake)
include(gvk-pipeline-explorer-info.cmake)
include(gvk-pipeline-explorer-gui.cmake)
include(gvk-pipeline-explorer-layer.cmake)
include(gvk-pipeline-explorer-plugin-factory.cmake)

################################################################################
# GITS
set(GITS_INSTALL_DIRECTORY "" CACHE STRING "")
if(GITS_INSTALL_DIRECTORY)
    target_include_directories(gvk-pipeline-explorer-gui PRIVATE "${GITS_INSTALL_DIRECTORY}/Player/")
    target_compile_definitions(gvk-pipeline-explorer-gui PUBLIC "GVK_GITS_ENABLED=1")
    ################################################################################
    # VK_LAYER_INTEL_gvk_virtual_swapchain
    add_custom_command(
        TARGET gvk-pipeline-explorer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:VK_LAYER_INTEL_gvk_virtual_swapchain>" "$<TARGET_FILE_DIR:gvk-pipeline-explorer>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:VK_LAYER_INTEL_gvk_virtual_swapchain>/VK_LAYER_INTEL_gvk_virtual_swapchain.json" "$<TARGET_FILE_DIR:gvk-pipeline-explorer>"
    )
endif()

################################################################################
# gvk-pipeline-explorer install
if(gvk-pipeline-explorer_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-pipeline-explorer)
    gvk_install_artifacts(TARGET gvk-pipeline-explorer-backend)
    gvk_install_artifacts(TARGET gvk-pipeline-explorer-info)
    gvk_install_artifacts(TARGET gvk-pipeline-explorer-plugin-factory)
    gvk_install_layer(TARGET VK_LAYER_INTEL_gvk_pipeline_explorer)
endif()
if(gvk-pipeline-explorer_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-pipeline-explorer-backend)
    gvk_install_headers(TARGET gvk-pipeline-explorer-info)
    gvk_install_headers(TARGET gvk-pipeline-explorer-plugin-factory)
endif()
