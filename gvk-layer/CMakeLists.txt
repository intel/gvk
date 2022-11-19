
################################################################################
# gvk-layer.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk/layer/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk/layer/generated/")
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
    target
        gvk-layer.generator
    folder
        "tools/"
    linkLibraries
        gvk-cppgen
    includeDirectories
        "${generatorSourcePath}/"
    includeFiles
        "${generatorSourcePath}/basic-layer.generator.hpp"
        "${generatorSourcePath}/layer-hooks.generator.hpp"
    sourceFiles
        "${generatorSourcePath}/main.cpp"
    outputFiles
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    compileDefinitions
        GVK_LAYER_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_LAYER_GENERATED_INCLUDE_PREFIX="gvk/layer/generated/"
        GVK_LAYER_GENERATED_SOURCE_PATH="${generatedSourcePath}"
)

################################################################################
# gvk-layer
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/layer/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/layer/")
gvk_add_static_library(
    target
        gvk-layer
    linkLibraries
        gvk-core
    includeDirectories
        "${includeDirectory}"
        "${generatedIncludeDirectory}"
    includeFiles
        "${includePath}/registry.hpp"
        "${generatedIncludeFiles}"
    sourceFiles
        "${sourcePath}/registry.cpp"
        "${generatedSourceFiles}"
)
