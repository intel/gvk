
################################################################################
# gvk-python
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-python/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-python/")
gvk_add_static_library(
    TARGET
        gvk-python
    FOLDER
        "gvk-python/"
    LINK_LIBRARIES
        gvk-reference
        pybind11::embed
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/module.hpp"
        "${includePath}/context.hpp"
        "${includePath}/defines.hpp"
    SOURCE_FILES
        "${sourcePath}/module.cpp"
        "${sourcePath}/context.cpp"
        "${sourcePath}/defines.cpp"
    COMPILE_DEFINITIONS
        HAVE_SNPRINTF=1
)

if(UNIX)
    # NOTE : pybind11 sets hidden visibility on its symbols, which causes a warning
    #   when trying to use them in structures with no explicit visibility setting.
    #   pybind11 symbols aren't being used across dll/so boundaries so it shouldn't
    #   cause any problems to simply suppress this warning.
    target_compile_options(gvk-python PUBLIC -Wno-attributes)
endif()

################################################################################
# gvk-python.test
if(gvk-python_TEST_ENABLED)
    set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
    gvk_add_target_test(
        TARGET
            gvk-python
        FOLDER
            "gvk-python/"
        SOURCE_FILES
            "${testsPath}/context.tests.cpp"
    )

    add_custom_command(
        TARGET gvk-python POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${pythonRuntime}" "$<TARGET_FILE_DIR:gvk-python>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${testsPath}/unitTests.pyd" "$<TARGET_FILE_DIR:gvk-python>/.."
    )
endif()
