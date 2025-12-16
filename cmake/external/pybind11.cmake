
include_guard(GLOBAL)

################################################################################
# pybind11
set(PYBIND11_NOPYTHON   OFF CACHE BOOL "" FORCE)
set(PYBIND11_TEST       OFF CACHE BOOL "" FORCE)
set(PYBIND11_FINDPYTHON ON  CACHE BOOL "" FORCE)
set(pybind11_VERSION f5fbe867d2d26e4a0a9177a51f6e568868ad3dc8) # v3.0.1
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY "https://github.com/pybind/pybind11.git"
    GIT_TAG ${pybind11_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(pybind11)

################################################################################
# Python runtime
set(pythonRuntimeVersion "3.13.7")
set(pythonRuntimeMD5 77f294ec267596827a2ab06e8fa3f18c)
set(pythonRuntimeArchive "python-${pythonRuntimeVersion}-embed-amd64.zip")
set(pythonRuntimeURL "https://www.python.org/ftp/python/${pythonRuntimeVersion}/${pythonRuntimeArchive}")
set(pythonRuntimeDirectory "${FETCHCONTENT_BASE_DIR}/python")
set(pythonRuntime "${pythonRuntimeDirectory}/${pythonRuntimeVersion}")
set(pythonRuntime "${pythonRuntime}" PARENT_SCOPE)
file(DOWNLOAD "${pythonRuntimeURL}" "${pythonRuntimeDirectory}/${pythonRuntimeArchive}" EXPECTED_MD5 ${pythonRuntimeMD5})
file(ARCHIVE_EXTRACT INPUT "${pythonRuntimeDirectory}/${pythonRuntimeArchive}" DESTINATION "${pythonRuntime}")
