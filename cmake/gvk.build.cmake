
include_guard()

include(CMakeParseArguments)
include(CTest)
include(FetchContent)

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

function(gvk_create_file_package target files destination)
    foreach(file ${files})
        get_filename_component(directory "${file}" DIRECTORY)
        string(REPLACE "${PROJECT_SOURCE_DIR}" "" groupName "${directory}")
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" groupName "${groupName}")
        if(NOT EXISTS "${destination}/${groupName}/")
            file(MAKE_DIRECTORY "${destination}/${groupName}/")
        endif()
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${file} "${destination}/${groupName}/"
        )
    endforeach()
endfunction()

macro(gvk_set_target_option target option)
    if(${option})
        target_compile_definitions(${target} PUBLIC ${option})
    endif()
endmacro()

function(gvk_setup_target)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;inputFiles;outputFiles;compileDefinitions" ${ARGN})
    target_include_directories(${args_target} PUBLIC "${args_includeDirectories}")
    target_compile_definitions(${args_target} PUBLIC "${args_compileDefinitions}")
    target_link_libraries(${args_target} PUBLIC "${args_linkLibraries}")
    set_target_properties(${args_target} PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(${args_target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)
    gvk_set_target_option(${args_target} GVK_GLFW_ENABLED)
    gvk_set_target_option(${args_target} GVK_GLM_ENABLED)
    gvk_set_target_option(${args_target} GVK_GLSLANG_ENABLED)
    gvk_set_target_option(${args_target} GVK_SPIRV_CROSS_ENABLED)
    gvk_set_target_option(${args_target} GVK_STB_ENABLED)
    gvk_create_file_group("${args_includeFiles}")
    gvk_create_file_group("${args_sourceFiles}")
    if(GVK_NO_PROTOTYPES)
        target_compile_definitions(${args_target} PUBLIC VK_NO_PROTOTYPES)
    endif()
    if(args_folder)
        set(args_folder "gvk/${args_folder}")
    else()
        set(args_folder "gvk/")
    endif()
    set_target_properties(${args_target} PROPERTIES FOLDER "${args_folder}")
    if(GVK_CREATE_CI_SCAN_PACKAGE)
        if(NOT "${args_folder}" STREQUAL "gvk/external/")
            set(package "${CMAKE_BINARY_DIR}/gvk-ci-scan-package/")
            gvk_create_file_package(${args_target} "${args_includeFiles}" "${package}/")
            gvk_create_file_package(${args_target} "${args_sourceFiles}" "${package}/")
            if(args_includeDirectories)
                string(REPLACE ";" "\n" args_includeDirectories "${args_includeDirectories}")
                string(REPLACE "//" "/" args_includeDirectories "${args_includeDirectories}")
                file(APPEND "${package}/includeDirectories" "${args_includeDirectories}\n")
            endif()
        endif()
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
    if(GVK_TESTS_ENABLED)
        gvk_add_executable(
            target ${args_target}.test
            folder "tests/"
            linkLibraries ${args_target} "${args_linkLibraries}" gtest_main
            includeDirectories "${args_includeDirectories}"
            includeFiles "${args_includeFiles}"
            sourceFiles "${args_sourceFiles}"
        )
        if(GVK_RUN_TESTS)
            add_test(NAME ${args_target}.test COMMAND ${args_target}.test)
            add_custom_command(
                TARGET ${args_target}.test POST_BUILD
                COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --verbose --output-on-failures
            )
        endif()
        if(GVK_CREATE_CI_TEST_PACKAGE)
            set(package "${CMAKE_BINARY_DIR}/gvk-ci-test-package/")
            if(NOT EXISTS "${package}")
                file(MAKE_DIRECTORY "${package}")
            endif()
            add_custom_command(
                TARGET ${args_target}.test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${args_target}.test> "${package}/"
            )
        endif()
    endif()
endmacro()
