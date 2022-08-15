
include_guard()

find_package(Git REQUIRED)
find_package(Vulkan REQUIRED)

include(CMakeDependentOption)
include(CmakeParseArguments)
include(CTest)
include(FetchContent)

option                (GVK_BUILD_TESTS   "" ON)
cmake_dependent_option(GVK_PACKAGE_TESTS "" ON GVK_BUILD_TESTS OFF)
cmake_dependent_option(GVK_RUN_TESTS     "" ON GVK_BUILD_TESTS OFF)
option                (GVK_BUILD_SAMPLES "" ON)
option                (GVK_NO_PROTOTYPES "" OFF)

set(Vulkan_SDK_DIR "${Vulkan_INCLUDE_DIRS}/../")
set(Vulkan_XML "${Vulkan_SDK_DIR}/share/vulkan/registry/vk.xml")

function(gvk_create_file_group files)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    foreach(file ${files})
        get_filename_component(directory "${file}" DIRECTORY)
        string(REPLACE "${PROJECT_SOURCE_DIR}" "" groupName "${directory}")
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" groupName "${groupName}")
        if(MSVC)
            string(REPLACE "/" "\\" groupName "${groupName}")
        endif()
        source_group("${groupName}" FILES "${file}")
    endforeach()
endfunction()

function(gvk_setup_target)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions" ${ARGN})
    target_include_directories(${args_target} PUBLIC "${args_includeDirectories}")
    target_compile_definitions(${args_target} PUBLIC "${args_compileDefinitions}")
    target_link_libraries(${args_target} PUBLIC "${args_linkLibraries}")
    set_target_properties(${args_target} PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(${args_target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)
    gvk_create_file_group("${args_includeFiles}")
    gvk_create_file_group("${args_sourceFiles}")
    if(GVK_NO_PROTOTYPES)
        target_compile_definitions(${args_target} PUBLIC VK_NO_PROTOTYPES)
    endif()
    if(args_folder)
        set_target_properties(${args_target} PROPERTIES FOLDER "gvk/${args_folder}")
    else()
        set_target_properties(${args_target} PROPERTIES FOLDER "gvk/")
    endif()
endfunction()

function(gvk_add_static_library)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions" ${ARGN})
    add_library(${args_target} STATIC "${args_includeFiles}" "${args_sourceFiles}")
    gvk_setup_target(${ARGN})
endfunction()

function(gvk_add_executable)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions" ${ARGN})
    add_executable(${args_target} "${args_includeFiles}" "${args_sourceFiles}")
    gvk_setup_target(${ARGN})
endfunction()

function(gvk_add_code_generator)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;inputFiles;outputFiles;compileDefinitions" ${ARGN})
    add_executable(${args_target} "${args_includeFiles}" "${args_sourceFiles}")
    gvk_setup_target(${ARGN})
    add_custom_command(
        OUTPUT ${args_outputFiles}
        COMMAND "${args_target}" "${args_inputFiles}"
        DEPENDS ${args_target} ${args_inputFiles}
    )
endfunction()

macro(gvk_add_target_test)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions" ${ARGN})
    if(GVK_BUILD_TESTS)
        gvk_add_executable(
            target ${args_target}.test
            folder "tests/"
            linkLibraries ${args_target} "${args_linkLibraries}" gtest_main
            includeDirectories "${args_includeDirectories}"
            includeFiles "${args_includeFiles}"
            sourceFiles "${args_sourceFiles}"
        )
        if(GVK_PACKAGE_TESTS)
            set(testPackage "${CMAKE_BINARY_DIR}/gvk-test-package/")
            if(NOT EXISTS "${testPackage}")
                file(MAKE_DIRECTORY "${testPackage}")
            endif()
            add_custom_command(
                TARGET ${args_target}.test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${args_target}.test> "${testPackage}"
            )
        endif()
        if(GVK_RUN_TESTS)
            add_test(NAME ${args_target}.test COMMAND ${args_target}.test)
            add_custom_command(
                TARGET ${args_target}.test POST_BUILD
                COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --verbose --output-on-failures
            )
        endif()
    endif()
endmacro()
