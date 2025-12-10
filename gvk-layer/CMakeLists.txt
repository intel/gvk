
################################################################################
# gvk-layer.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-layer/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-layer/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/basic-api-call-handler.hpp"
    "${generatedIncludePath}/basic-layer.hpp"
    "${generatedIncludePath}/layer-hooks.hpp"
    "${generatedIncludePath}/unique-handles-layer-hooks.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/basic-api-call-handler.cpp"
    "${generatedSourcePath}/basic-layer.cpp"
    "${generatedSourcePath}/layer-hooks.cpp"
    "${generatedSourcePath}/unique-handles-layer-hooks.cpp"
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
        "${generatorSourcePath}/basic-api-call-handler.generator.hpp"
        "${generatorSourcePath}/basic-layer.generator.hpp"
        "${generatorSourcePath}/layer-hooks.generator.hpp"
        "${generatorSourcePath}/unique-handles-layer-hooks.generator.hpp"
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
        gvk-command-structures
        gvk-containers
        gvk-reference
        gvk-runtime
        gvk-structures
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/log.hpp"
        "${includePath}/registry.hpp"
        "${includePath}/unique-handles-manager.hpp"
        "${includeDirectory}/gvk-layer.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/log.cpp"
        "${sourcePath}/registry.cpp"
        "${sourcePath}/unique-handles-manager.cpp"
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/unique-handles-layer-hooks.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
    set_source_files_properties("${generatedSourcePath}/layer-hooks.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# gvk-layer install
if(gvk-layer_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-layer)
endif()
if(gvk-layer_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-layer)
endif()
