
################################################################################
# gvk-binding-info.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-binding-info/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-binding-info/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/binding-info.h"
    "${generatedIncludePath}/binding-info-enumerations-to-string.hpp"
    "${generatedIncludePath}/binding-info-structure-cerealization.hpp"
    "${generatedIncludePath}/binding-info-structure-comparison-operators.hpp"
    "${generatedIncludePath}/binding-info-structure-create-copy.hpp"
    "${generatedIncludePath}/binding-info-structure-decerealization.hpp"
    "${generatedIncludePath}/binding-info-structure-deserialization.hpp"
    "${generatedIncludePath}/binding-info-structure-destroy-copy.hpp"
    "${generatedIncludePath}/binding-info-structure-get-stype.hpp"
    "${generatedIncludePath}/binding-info-structure-make-tuple.hpp"
    "${generatedIncludePath}/binding-info-structure-serialization.hpp"
    "${generatedIncludePath}/binding-info-structure-to-string.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/binding-info-enumerations-to-string.cpp"
    "${generatedSourcePath}/binding-info-structure-comparison-operators.cpp"
    "${generatedSourcePath}/binding-info-structure-create-copy.cpp"
    "${generatedSourcePath}/binding-info-structure-deserialization.cpp"
    "${generatedSourcePath}/binding-info-structure-destroy-copy.cpp"
    "${generatedSourcePath}/binding-info-structure-serialization.cpp"
    "${generatedSourcePath}/binding-info-structure-to-string.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    TARGET
        gvk-binding-info.generator
    FOLDER
        "gvk-binding-info/"
    LINK_LIBRARIES
        gvk-cppgen
    SOURCE_FILES
        "${generatorSourcePath}/main.cpp"
    OUTPUT_FILES
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_BINDING_INFO_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_BINDING_INFO_GENERATED_INCLUDE_PREFIX="gvk-binding-info/generated/"
        GVK_BINDING_INFO_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk-binding-info
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-binding-info/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-binding-info/")
gvk_add_static_library(
    TARGET
        gvk-binding-info
    FOLDER
        "gvk-binding-info/"
    LINK_LIBRARIES
        gvk-command-structures
        gvk-containers
        gvk-format-info
        gvk-runtime
        gvk-structures
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/detail/binding-monitor.hpp"
        "${includePath}/detail/binding-registry.hpp"
        "${includePath}/detail/cerealization-manual.hpp"
        "${includePath}/detail/info-object-registry.hpp"
        "${includePath}/detail/make-tuple-manual.hpp"
        "${includePath}/detail/resource-info-registry.hpp"
        "${includeDirectory}/gvk-binding-info.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/detail/binding-monitor.cpp"
        "${sourcePath}/detail/binding-registry.cpp"
        "${sourcePath}/detail/copy-manual.cpp"
        "${sourcePath}/detail/to-string-manual.cpp"
)

################################################################################
# gvk-binding-info install
if(gvk-binding-info_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-binding-info)
endif()
if(gvk-binding-info_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-binding-info)
endif()
