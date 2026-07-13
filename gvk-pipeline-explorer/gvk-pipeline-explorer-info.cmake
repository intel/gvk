
################################################################################
# gvk-pipeline-explorer-info
gvk_add_static_library(
    TARGET
        gvk-pipeline-explorer-info
    FOLDER
        "gvk-pipeline-explorer/"
    LINK_LIBRARIES
        gvk-command-structures
        gvk-runtime
        gvk-structures
        boost-multiprecision
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${infoGeneratedIncludeFiles}"
        "${includeDirectory}/gvk-pipeline-explorer.hpp"
        "${includeDirectory}/gvk-pipeline-explorer.hpp"
    SOURCE_FILES
        "${infoGeneratedSourceFiles}"
        "${sourcePath}/detail/to-string-manual.cpp"
        "${sourcePath}/detail/to-string-manual.cpp"
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/pipeline-explorer-structure-deserialization.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()
