
################################################################################
# gvk-cppgen
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-cppgen/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-cppgen/")
gvk_add_static_library(
    target
        gvk-cppgen
    folder
        "gvk-cppgen/"
    linkLibraries
        gvk-string
        gvk-xml
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/basic-handle-generator.hpp"
        "${includePath}/basic-structure-member-processor-generator.hpp"
        "${includePath}/compile-guard-generator.hpp"
        "${includePath}/file-generator.hpp"
        "${includePath}/header-guard-generator.hpp"
        "${includePath}/include.hpp"
        "${includePath}/module-generator.hpp"
        "${includePath}/namespace-generator.hpp"
        "${includePath}/utilities.hpp"
    sourceFiles
        "${sourcePath}/basic-handle-generator.cpp"
        "${sourcePath}/basic-structure-member-processor-generator.cpp"
        "${sourcePath}/compile-guard-generator.cpp"
        "${sourcePath}/file-generator.cpp"
        "${sourcePath}/header-guard-generator.cpp"
        "${sourcePath}/module-generator.cpp"
        "${sourcePath}/namespace-generator.cpp"
        "${sourcePath}/utilities.cpp"
)
