
################################################################################
# gvk-layer.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-layer/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-layer/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/basic-layer.hpp"
    "${generatedIncludePath}/layer-hooks.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/basic-layer.cpp"
    "${generatedSourcePath}/layer-hooks.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    TARGET
        gvk-layer.generator
    FOLDER
        "gvk-layer/"
    LINK_LIBRARIES
        gvk-cppgen
    INCLUDE_DIRECTORIES
        "${generatorSourcePath}/"
    INCLUDE_FILES
        "${generatorSourcePath}/basic-layer.generator.hpp"
        "${generatorSourcePath}/layer-hooks.generator.hpp"
    SOURCE_FILES
        "${generatorSourcePath}/main.cpp"
    OUTPUT_FILES
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_LAYER_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_LAYER_GENERATED_INCLUDE_PREFIX="gvk-layer/generated/"
        GVK_LAYER_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk-layer
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-layer/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-layer/")
gvk_add_static_library(
    TARGET
        gvk-layer
    FOLDER
        "gvk-layer/"
    LINK_LIBRARIES
        gvk-runtime
        gvk-structures
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/registry.hpp"
        "${includeDirectory}/gvk-layer.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/registry.cpp"
)

################################################################################
# gvk-layer install
gvk_install_library(TARGET gvk-layer)
