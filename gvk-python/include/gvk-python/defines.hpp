
/*******************************************************************************

MIT License

Copyright (c) Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#pragma once

#include "pybind11/embed.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include <cassert>

enum GvkPythonResult
{
    GVK_PYTHON_SUCCESS = 0,
    GVK_PYTHON_ERROR_UNKNOWN = 1
};

#define gvk_python_stringify(STR) #STR
#define gvk_python_expand(STR) gvk_python_stringify(STR)
#define gvk_python_file_line (__FILE__ "(" gvk_python_expand(__LINE__) ")")

#define gvk_python_result_scope_begin(GVK_PYTHON_RESULT) GvkPythonResult gvkPythonResult = GVK_PYTHON_RESULT; (void)gvkPythonResult; {
#define gvk_python_result_scope_break(GVK_PYTHON_RESULT) { gvkPythonResult = GVK_PYTHON_RESULT; goto GVK_PYTHON_BREAK; }
#define gvk_python_result(GVK_PYTHON_CALL)                                                                                 \
{                                                                                                                          \
    gvkPythonResult = (GvkPythonResult)(GVK_PYTHON_CALL);                                                                  \
    if (gvkPythonResult != GVK_PYTHON_SUCCESS) {                                                                           \
        if (!gvk::python::detail::process_result_scope_failure(gvkPythonResult, gvk_python_file_line, #GVK_PYTHON_CALL)) { \
            assert(gvkPythonResult == GVK_PYTHON_SUCCESS && #GVK_PYTHON_CALL);                                             \
        }                                                                                                                  \
        goto GVK_PYTHON_BREAK;                                                                                             \
    }                                                                                                                      \
}
#define gvk_python_result_scope_end } GVK_PYTHON_BREAK:

namespace gvk {
namespace python {

/**
Callback for processing gvk_python_result_scope failures
@param [in] gvkPythonResult The GvkPythonResult of the failed gvk_python_result_scope
@param [in] pFileLine A string with the file and line number where the error occured
@param [in[ pGvkPythonCall A string with the expression that triggered the error
@return Whether or not to continue execution in Debug configurations
    @note In Debug configurations returning false will trigger an assert()
*/
typedef bool (*PFN_result_scope_callback)(GvkPythonResult gvkPythonResult, const char* pFileLine, const char* pGvkPythonCall);

/**
Global gvk_python_result_scope callback
*/
extern PFN_result_scope_callback gPfnGvkResultScopeCallback;

/**
thread_local gvk_python_result_scope callback
    @note thread_local gvk_python_result_scope callbacks take precedence over the global gvk_python_result_scope
    @note If a thread_local gvk_python_result_scope calback isn't set for a particular thread, the global callback will be used if set
*/
extern thread_local PFN_result_scope_callback tlPfnGvkResultScopeCallback;

namespace detail {

bool process_result_scope_failure(GvkPythonResult gvkPythonResult, const char* pFileLine, const char* pGvkPythonCall);

} // namespace detail
} // namespace python
} // namespace gvk
