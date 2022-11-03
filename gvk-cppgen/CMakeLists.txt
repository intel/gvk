
################################################################################
# gvk-cppgen
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/cppgen/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/cppgen/")
gvk_add_static_library(
    target
        gvk-cppgen
    linkLibraries
        gvk-string
        gvk-xml
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/basic-structure-member-processor-generator.hpp"
        "${includePath}/compile-guard-generator.hpp"
        "${includePath}/file-generator.hpp"
        "${includePath}/module-generator.hpp"
        "${includePath}/namespace-generator.hpp"
        "${includePath}/utilities.hpp"
        "${includeDirectory}/gvk/cppgen.hpp"
    sourceFiles
        "${sourcePath}/basic-structure-member-processor-generator.cpp"
        "${sourcePath}/compile-guard-generator.cpp"
        "${sourcePath}/file-generator.cpp"
        "${sourcePath}/module-generator.cpp"
        "${sourcePath}/namespace-generator.cpp"
        "${sourcePath}/utilities.cpp"
)
