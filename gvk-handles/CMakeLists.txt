
################################################################################
# gvk-handles.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-handles/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-handles/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/forward-declarations.inl"
    "${generatedIncludePath}/handles.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/handles.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    TARGET
        gvk-handles.generator
    FOLDER
        "gvk-handles/"
    LINK_LIBRARIES
        gvk-cppgen
    INCLUDE_DIRECTORIES
        "${generatorSourcePath}/"
    INCLUDE_FILES
        "${generatorSourcePath}/handles.generator.hpp"
    SOURCE_FILES
        "${generatorSourcePath}/main.cpp"
    OUTPUT_FILES
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_HANDLES_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_HANDLES_GENERATED_INCLUDE_PREFIX="gvk-handles/generated/"
        GVK_HANDLES_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/comparison-operators.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# gvk-handles
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-handles/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-handles/")
gvk_add_static_library(
    TARGET
        gvk-handles
    FOLDER
        "gvk-handles/"
    LINK_LIBRARIES
        gvk-format-info
        gvk-reference
        gvk-runtime
        gvk-string
        gvk-structures
        VulkanMemoryAllocator
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/detail/handle-utilities.hpp"
        "${includePath}/context.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/handles.hpp"
        "${includePath}/mesh.hpp"
        "${includePath}/render-target.hpp"
        "${includePath}/utilities.hpp"
        "${includePath}/wsi-manager.hpp"
        "${includeDirectory}/gvk-handles.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/detail/handle-utilities.cpp"
        "${sourcePath}/context.cpp"
        "${sourcePath}/mesh.cpp"
        "${sourcePath}/render-target.cpp"
        "${sourcePath}/utilities.cpp"
        "${sourcePath}/wsi-manager.cpp"
)

################################################################################
# gvk-handles.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    TARGET
        gvk-handles
    FOLDER
        "gvk-handles/"
    SOURCE_FILES
        "${testsPath}/render-target.tests.cpp"
)

################################################################################
# gvk-handles install
gvk_install_library(TARGET gvk-handles)
