
include_guard(GLOBAL)

set(VTK_BUILD_ALL_MODULES                      OFF CACHE BOOL   "" FORCE)
set(VTK_MODULE_ENABLE_VTK_CommonCore           YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_CommonDataModel      YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_CommonMath           YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_CommonTransforms     YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_CommonExecutionModel YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_FiltersCore          YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_FiltersGeometry      YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_FiltersGeneral       YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_FiltersBoolean       YES CACHE STRING "" FORCE)
set(VTK_MODULE_ENABLE_VTK_IOCore               YES CACHE STRING "" FORCE)
set(VTK_WRAP_PYTHON                            OFF CACHE BOOL   "" FORCE)
set(VTK_BUILD_TESTING                          OFF CACHE BOOL   "" FORCE)
set(VTK_BUILD_EXAMPLES                         OFF CACHE BOOL   "" FORCE)
set(vtk_VERSION v9.5.0.rc0)
FetchContent_Declare(
    vtk
    GIT_REPOSITORY "https://github.com/Kitware/VTK.git"
    GIT_TAG ${vtk_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(vtk)
FetchContent_GetProperties(vtk)

gvk_get_directory_targets(${vtk_SOURCE_DIR} vtkTargets)
foreach(vtkTarget ${vtkTargets})
    string(REPLACE "::" ";" vtkTarget ${vtkTarget})
    list(LENGTH vtkTarget length)
    if(length EQUAL 2)
        list(GET vtkTarget 0 vtkDirectory)
        list(GET vtkTarget 1 vtkTarget)
        string(REPLACE "${vtk_SOURCE_DIR}" "" vtkDirectory "${vtkDirectory}")
        set_target_properties(${vtkTarget} PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/vtk/")
    else()
        message(FATAL_ERROR "TODO : Documentation")
    endif()
endforeach()
