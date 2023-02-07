
################################################################################
# gvk-format-info.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-format-info/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-format-info/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/enumerate-formats.hpp"
    "${generatedIncludePath}/format-info-enumerations-to-string.hpp"
    "${generatedIncludePath}/format-info-structure-cerealization.hpp"
    "${generatedIncludePath}/format-info-structure-comparison-operators.hpp"
    "${generatedIncludePath}/format-info-structure-create-copy.hpp"
    "${generatedIncludePath}/format-info-structure-decerealization.hpp"
    "${generatedIncludePath}/format-info-structure-deserialization.hpp"
    "${generatedIncludePath}/format-info-structure-destroy-copy.hpp"
    "${generatedIncludePath}/format-info-structure-get-stype.hpp"
    "${generatedIncludePath}/format-info-structure-make-tuple.hpp"
    "${generatedIncludePath}/format-info-structure-serialization.hpp"
    "${generatedIncludePath}/format-info-structure-to-string.hpp"
    "${generatedIncludePath}/format-info.h"
)
set(generatedSourceFiles
    "${generatedSourcePath}/format-info-enumerations-to-string.cpp"
    "${generatedSourcePath}/format-info-structure-comparison-operators.cpp"
    "${generatedSourcePath}/format-info-structure-create-copy.cpp"
    "${generatedSourcePath}/format-info-structure-deserialization.cpp"
    "${generatedSourcePath}/format-info-structure-destroy-copy.cpp"
    "${generatedSourcePath}/format-info-structure-serialization.cpp"
    "${generatedSourcePath}/format-info-structure-to-string.cpp"
    "${generatedSourcePath}/get-format-info.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    target
        gvk-format-info.generator
    folder
        "gvk-format-info/"
    linkLibraries
        gvk-cppgen
    includeDirectories
        "${generatorSourcePath}/"
    includeFiles
        "${generatorSourcePath}/enumerate-formats.generator.hpp"
        "${generatorSourcePath}/get-format-info.generator.hpp"
    sourceFiles
        "${generatorSourcePath}/main.cpp"
    outputFiles
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    compileDefinitions
        GVK_FORMAT_INFO_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_FORMAT_INFO_GENERATED_INCLUDE_PREFIX="gvk-format-info/generated/"
        GVK_FORMAT_INFO_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk-format-info
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(sourceDirectory "${CMAKE_CURRENT_LIST_DIR}/source/")
gvk_add_static_library(
    target
        gvk-format-info
    folder
        "gvk-format-info/"
    linkLibraries
        gvk-runtime
        gvk-structures
    includeDirectories
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    includeFiles
        "${generatedIncludeFiles}"
        "${includeDirectory}/gvk-format-info.hpp"
    sourceFiles
        "${generatedSourceFiles}"
        "${sourceDirectory}/gvk-format-info.cpp"
)
