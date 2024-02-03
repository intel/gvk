
include_guard()

include(CMakePackageConfigHelpers)
include(CMakeParseArguments)
include(CTest)

find_package(Git REQUIRED)

set(gvkBuildModuleDirectory "${CMAKE_CURRENT_LIST_DIR}")

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

macro(gvk_set_target_option target option)
    if(${option})
        target_compile_definitions(${target} PUBLIC ${option})
    endif()
endmacro()

function(gvk_setup_target)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS" ${ARGN})
    target_include_directories(${ARGS_TARGET} PUBLIC "$<BUILD_INTERFACE:${ARGS_INCLUDE_DIRECTORIES}>" INTERFACE $<INSTALL_INTERFACE:include>)
    target_compile_definitions(${ARGS_TARGET} PUBLIC "${ARGS_COMPILE_DEFINITIONS}")
    target_link_libraries(${ARGS_TARGET} PUBLIC "${ARGS_LINK_LIBRARIES}")
    set_target_properties(${ARGS_TARGET} PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(${ARGS_TARGET} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -fPIC>)
    gvk_create_file_group("${ARGS_INCLUDE_FILES}")
    gvk_create_file_group("${ARGS_SOURCE_FILES}")
    set_target_properties(${ARGS_TARGET} PROPERTIES FOLDER "${GVK_IDE_FOLDER}/${ARGS_FOLDER}")
endfunction()

function(gvk_add_static_library)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS" ${ARGN})
    add_library(${ARGS_TARGET} STATIC "${ARGS_INCLUDE_FILES}" "${ARGS_SOURCE_FILES}")
    gvk_setup_target(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES "${ARGS_INCLUDE_DIRECTORIES}"
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS  ${ARGS_COMPILE_DEFINITIONS}
    )
endfunction()

function(gvk_add_executable)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS" ${ARGN})
    add_executable(${ARGS_TARGET} "${ARGS_INCLUDE_FILES}" "${ARGS_SOURCE_FILES}")
    gvk_setup_target(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES "${ARGS_INCLUDE_DIRECTORIES}"
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS  ${ARGS_COMPILE_DEFINITIONS}
    )
endfunction()

function(gvk_add_code_generator)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;INPUT_FILES;OUTPUT_FILES;COMPILE_DEFINITIONS" ${ARGN})
    gvk_add_executable(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES "${ARGS_INCLUDE_DIRECTORIES}"
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS  ${ARGS_COMPILE_DEFINITIONS}
    )
    add_custom_command(
        OUTPUT ${ARGS_OUTPUT_FILES}
        COMMAND "${ARGS_TARGET}" "${ARGS_INPUT_FILES}"
        DEPENDS ${ARGS_TARGET} ${ARGS_INPUT_FILES}
    )
endfunction()

function(gvk_add_layer)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INTERFACE_FILES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS;DESCRIPTION;VERSION;COMPANY;COPYRIGHT;ENTRY_POINTS" ${ARGN})
    if(NOT ARGS_VERSION)
        set(ARGS_VERSION 1)
    endif()
    if(NOT ARGS_COMPANY)
        set(ARGS_COMPANY "Intel Corporation")
    endif()
    if(NOT ARGS_COPYRIGHT)
        set(ARGS_COPYRIGHT "Copyright Intel Corporation")
    endif()
    if(MSVC)
        string(REPLACE ";" "\n" ARGS_ENTRY_POINTS "${ARGS_ENTRY_POINTS}")
        configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gvk-layer.def.in" "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.def")
        configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gvk-layer.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.rc")
        list(APPEND ARGS_SOURCE_FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.def"
            "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.rc"
        )
    endif()
    if(ARGS_INTERFACE_FILES)
        add_library(${ARGS_TARGET}-interface INTERFACE "${ARGS_INTERFACE_FILES}")
        target_link_libraries(${ARGS_TARGET}-interface INTERFACE Vulkan::Vulkan)
        target_include_directories(${ARGS_TARGET}-interface INTERFACE "$<BUILD_INTERFACE:${ARGS_INCLUDE_DIRECTORIES}>" INTERFACE $<INSTALL_INTERFACE:include>)
        gvk_create_file_group("${ARGS_INTERFACE_FILES}")
        set_target_properties(${ARGS_TARGET}-interface PROPERTIES FOLDER "${GVK_IDE_FOLDER}/${ARGS_FOLDER}")
        list(APPEND ARGS_LINK_LIBRARIES ${ARGS_TARGET}-interface)
    endif()
    add_library(${ARGS_TARGET} SHARED "${ARGS_INCLUDE_FILES}" "${ARGS_SOURCE_FILES}")
    list(APPEND ARGS_LINK_LIBRARIES gvk-layer)
    gvk_setup_target(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES "${ARGS_INCLUDE_DIRECTORIES}"
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS  ${ARGS_COMPILE_DEFINITIONS}
    )
    if(MSVC)
        target_compile_options(${ARGS_TARGET} PRIVATE /guard:cf)
        target_link_options(${ARGS_TARGET} PRIVATE /guard:cf /DYNAMICBASE)
        set(libraryPath ".\\\\${ARGS_TARGET}.dll")
    else()
        set(libraryPath "./lib${ARGS_TARGET}.so")
    endif()
    # HUH : Why doesn't CMAKE_CURRENT_FUNCTION_LIST_DIR work on Linux?
    configure_file("${gvkBuildModuleDirectory}/gvk-layer.json.in" "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json")
    add_custom_command(
        TARGET ${ARGS_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json" "$<TARGET_FILE_DIR:${ARGS_TARGET}>"
    )
endfunction()

macro(gvk_add_target_test)
    if(GVK_BUILD_TESTS)
        cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS" ${ARGN})
        list(APPEND ARGS_LINK_LIBRARIES gtest_main)
        get_target_property(type ${ARGS_TARGET} TYPE)
        if(type STREQUAL STATIC_LIBRARY)
            list(APPEND ARGS_LINK_LIBRARIES ${ARGS_TARGET})
        elseif(EXISTS ${ARGS_TARGET}-interface)
            list(APPEND ARGS_LINK_LIBRARIES ${ARGS_TARGET}-interface)
        endif()
        gvk_add_executable(
            TARGET               ${ARGS_TARGET}.tests
            FOLDER              "${ARGS_FOLDER}"
            LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
            INCLUDE_DIRECTORIES "${ARGS_INCLUDE_DIRECTORIES}"
            INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
            SOURCE_FILES        "${ARGS_SOURCE_FILES}"
            COMPILE_DEFINITIONS  ${ARGS_COMPILE_DEFINITIONS}
        )
        if(type STREQUAL SHARED_LIBRARY)
            add_dependencies(${ARGS_TARGET}.tests ${ARGS_TARGET})
        endif()
        if(GVK_RUN_TESTS)
            add_test(NAME ${ARGS_TARGET}.tests COMMAND ${ARGS_TARGET}.tests)
            add_custom_command(
                TARGET ${ARGS_TARGET}.tests POST_BUILD
                COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --verbose --output-on-failures
            )
        endif()
        if(GVK_CREATE_TEST_PACKAGE)
            set(package "${CMAKE_BINARY_DIR}/gvk-test-package/")
            if(NOT EXISTS "${package}")
                file(MAKE_DIRECTORY "${package}")
            endif()
            add_custom_command(
                TARGET ${ARGS_TARGET}.tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${ARGS_TARGET}.tests> "${package}/"
            )
            if(type STREQUAL SHARED_LIBRARY)
                add_dependencies(${ARGS_TARGET}.tests ${ARGS_TARGET})
                add_custom_command(
                    TARGET ${ARGS_TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${ARGS_TARGET}> "${package}/"
                )
                if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json")
                    add_custom_command(
                        TARGET ${ARGS_TARGET} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json" "${package}/"
                    )
                endif()
            endif()
        endif()
    endif()
endmacro()

macro(gvk_get_commit_hash commitHash)
    execute_process(
        WORKING_DIRECTORY "${gvkBuildModuleDirectory}/../"
        COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD
        OUTPUT_VARIABLE ${commitHash}
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endmacro()

function(gvk_install_artifacts)
    if(GVK_CREATE_INSTALL_PACKAGE)
        cmake_parse_arguments(ARGS "" "TARGET;VERSION" "" ${ARGN})
        if(NOT ARGS_VERSION)
            get_target_property(ARGS_VERSION ${ARGS_TARGET} VERSION)
            if(NOT ARGS_VERSION)
                gvk_get_commit_hash(ARGS_VERSION)
            endif()
        endif()
        install(
            TARGETS ${ARGS_TARGET}
            EXPORT ${ARGS_TARGET}Targets
            LIBRARY DESTINATION lib/$<CONFIG>/
            ARCHIVE DESTINATION lib/$<CONFIG>/
            RUNTIME DESTINATION bin/$<CONFIG>/
        )
        export(EXPORT ${ARGS_TARGET}Targets FILE "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}Targets.cmake")
        install(EXPORT ${ARGS_TARGET}Targets DESTINATION cmake/${ARGS_TARGET}/)
        set(configVersion "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}ConfigVersion.cmake")
        write_basic_package_version_file("${configVersion}" VERSION ${ARGS_VERSION} COMPATIBILITY ExactVersion)
        set(configTemplate "${gvkBuildModuleDirectory}/gvk-target.config.cmake.in")
        set(config "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}Config.cmake")
        get_target_property(interfaceLinkLibraries ${ARGS_TARGET} INTERFACE_LINK_LIBRARIES)
        list(REMOVE_ITEM interfaceLinkLibraries ${CMAKE_DL_LIBS} rt Threads::Threads Vulkan::Vulkan)
        configure_package_config_file("${configTemplate}" "${config}" INSTALL_DESTINATION {CMAKE_BINARY_DIR}/cmake/)
        install(FILES "${config}" "${configVersion}" DESTINATION cmake/${ARGS_TARGET}/)
    endif()
endfunction()

function(gvk_install_headers)
    if(GVK_CREATE_INSTALL_PACKAGE)
        cmake_parse_arguments(ARGS "" "TARGET" "" ${ARGN})
        get_target_property(interfaceIncludeDirectories ${ARGS_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
        if(interfaceIncludeDirectories)
            string(REPLACE "$" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
            string(REPLACE "<BUILD_INTERFACE:" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
            string(REPLACE "<INSTALL_INTERFACE:include" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
            string(REPLACE ">" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
            foreach(interfaceIncludeDirectory ${interfaceIncludeDirectories})
                install(DIRECTORY "${interfaceIncludeDirectory}" DESTINATION include/)
            endforeach()
        endif()
    endif()
endfunction()

function(gvk_install_library)
    if(GVK_CREATE_INSTALL_PACKAGE)
        gvk_install_artifacts(${ARGV})
        gvk_install_headers(${ARGV})
    endif()
endfunction()

function(gvk_install_layer)
    if(GVK_CREATE_INSTALL_PACKAGE)
        cmake_parse_arguments(ARGS "" "TARGET;COMPONENT" "" ${ARGN})
        gvk_install_library(${ARGV})
        install(FILES "$<TARGET_FILE_DIR:${ARGS_TARGET}>/${ARGS_TARGET}.json" DESTINATION bin/$<CONFIG>/)
    endif()
endfunction()

function(gvk_install_package)
    if(GVK_CREATE_INSTALL_PACKAGE)
        file(GLOB exportedConfigFiles "${CMAKE_BINARY_DIR}/cmake/*Config.cmake")
        foreach(exportedConfigFile ${exportedConfigFiles})
            get_filename_component(exportedConfigFileName "${exportedConfigFile}" NAME)
            string(REPLACE "Config.cmake" "" exportedTargetName "${exportedConfigFileName}")
            list(APPEND exportedTargets ${exportedTargetName})
        endforeach()
        list(REMOVE_ITEM exportedTargets gvk)
        gvk_get_commit_hash(version)
        set(configVersion "${CMAKE_BINARY_DIR}/cmake/gvkConfigVersion.cmake")
        write_basic_package_version_file("${configVersion}" VERSION ${version} COMPATIBILITY ExactVersion)
        set(configTemplate "${gvkBuildModuleDirectory}/gvk.config.cmake.in")
        set(config "${CMAKE_BINARY_DIR}/cmake/gvkConfig.cmake")
        configure_package_config_file("${configTemplate}" "${config}" INSTALL_DESTINATION ${CMAKE_BINARY_DIR}/cmake/)
        set(gvkVersion "${CMAKE_BINARY_DIR}/cmake/gvkVersion.cmake")
        file(WRITE "${gvkVersion}" "${version}")
        install(FILES "${config}" "${configVersion}" "${gvkVersion}" DESTINATION cmake/)
    endif()
endfunction()
