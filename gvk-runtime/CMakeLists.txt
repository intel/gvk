
################################################################################
# gvk-runtime.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-runtime/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-runtime/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/dispatch-table.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/dispatch-table.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    target
        gvk-runtime.generator
    folder
        "gvk-runtime/"
    linkLibraries
        gvk-cppgen
    includeDirectories
        "${generatorSourcePath}/"
    includeFiles
        "${generatorSourcePath}/dispatch-table.generator.hpp"
    sourceFiles
        "${generatorSourcePath}/main.cpp"
    outputFiles
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    compileDefinitions
        GVK_RUNTIME_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_RUNTIME_GENERATED_INCLUDE_PREFIX="gvk-runtime/generated/"
        GVK_RUNTIME_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk-runtime
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(sourceDirectory "${CMAKE_CURRENT_LIST_DIR}/source/")
gvk_add_static_library(
    target
        gvk-runtime
    folder
        "gvk-runtime/"
    linkLibraries
        gvk-string
        Vulkan::Vulkan
        ${CMAKE_DL_LIBS}
    includeDirectories
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    includeFiles
        "${generatedIncludeFiles}"
        "${includeDirectory}/gvk-defines.hpp"
        "${includeDirectory}/gvk-dispatch-table.hpp"
        "${includeDirectory}/gvk-environment.hpp"
        "${includeDirectory}/gvk-runtime.hpp"
    sourceFiles
        "${generatedSourceFiles}"
        "${sourceDirectory}/gvk-defines.cpp"
        "${sourceDirectory}/gvk-environment.cpp"
        "${sourceDirectory}/gvk-runtime.cpp"
)
