
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

#include "gvk-python/context.hpp"
#include <iostream>

namespace gvk {
namespace python {

GvkPythonResult Context::create(const GvkPythonContextCreateInfo* pCreateInfo, Context* pContext)
{

    //Developer note:
    //Whatever is in the context will be differnet for each context instance
    //Whatever is in the control block will be shared within context instances (will stay the same)

    gvk_python_result_scope_begin(GVK_PYTHON_SUCCESS) {
        gvk_python_result(pCreateInfo ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(pContext ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);

        // Enumerate gvk::python::Context's to find if one already exists
        gvk::Reference<Context::ControlBlock>::enumerate(
            [&](const auto& reference)
            {
                pContext->mReference = reference;
            }
        );

        // If no existing gvk::python::Context was found, create one
        if (!*pContext) {
            pContext->mReference.reset(newref);
            auto& controlBlock = pContext->mReference.get_obj();

            // TODO : Configuration should be supplied via GvkPythonContextCreateInfo then
            //  passed to std::make_unique<> here.
            controlBlock.mupPythonInterpreter = std::make_unique<pybind11::scoped_interpreter>();
        }
    } gvk_python_result_scope_end;
    return gvkPythonResult;
}

Context::~Context()
{
    reset();
}

void Context::reset()
{
    mReference.reset();
}

GvkPythonResult Context::evaluate(const char* pPythonSource, pybind11::object* returnedObject)
{
    gvk_python_result_scope_begin(GVK_PYTHON_SUCCESS) {
        gvk_python_result(pPythonSource ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(returnedObject ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);

        try {
            // Redirect stdout
            *returnedObject = pybind11::eval(pPythonSource);
        } catch (const pybind11::error_already_set& e) {
            (void)e;
            PyErr_Print();
            gvkPythonResult = GVK_PYTHON_ERROR_UNKNOWN;
        }
    } gvk_python_result_scope_end;
    return gvkPythonResult;
}

GvkPythonResult Context::execute(const char* pPythonSource, pybind11::object* returnedObject)
{
    gvk_python_result_scope_begin(GVK_PYTHON_SUCCESS) {
        gvk_python_result(pPythonSource ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(returnedObject ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        std::string pStdOut;
        pStdOut.clear();

        try {
            // Redirect stdout
            auto sys = pybind11::module_::import("sys");
            auto io = pybind11::module_::import("io");
            auto stringIo = io.attr("StringIO")();
            sys.attr("stdout") = stringIo;

            // Execute python statement (script)
            pybind11::exec(pPythonSource);

            // Get stdout
            pStdOut = pybind11::str(stringIo.attr("getvalue")()).cast<std::string>();

            // Remove trailing newline if present
            if (!pStdOut.empty() && pStdOut.back() == '\n') {
                pStdOut.pop_back();
            }

            // Try to evaluate the stdout string as a Python literal
            try {
                parsePyStdOut(pStdOut, returnedObject);
            } catch (const pybind11::error_already_set&) {
                // If literal_eval fails, return as string
                *returnedObject = pybind11::str(pStdOut);
            }

        } catch (const pybind11::error_already_set& e) {
            (void)e; //This 'e' is the exception. 
            //We can (after everything is done if we want to) interpret 'e' into gvkPythonResult (see TODO below)

            // TODO : We'll want to integrate python errors with GvkPythonResult and the
            //  associated callbacks, but that can wait til more is setup.
            PyErr_Print();
            gvkPythonResult = GVK_PYTHON_ERROR_UNKNOWN;
        }
    } gvk_python_result_scope_end;
    return gvkPythonResult;
}

GvkPythonResult Context::parsePyStdOut(std::string pPythonSource, pybind11::object* returnedObject)
{
    gvk_python_result_scope_begin(GVK_PYTHON_SUCCESS) {
        gvk_python_result(pPythonSource.c_str() ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(returnedObject ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);

        try {

            // Remove leading/trailing whitespace
            pPythonSource.erase(0, pPythonSource.find_first_not_of(" \t\n\r"));
            pPythonSource.erase(pPythonSource.find_last_not_of(" \t\n\r") + 1);

            //Edge case
            if (pPythonSource.empty()) {
                *returnedObject = pybind11::str("");
            }

            // Check for list format [...], dict format {...}, or  tuple format (...)
            else if ((pPythonSource.front() == '[' && pPythonSource.back() == ']') ||
               (pPythonSource.front() == '{' && pPythonSource.back() == '}') ||
               (pPythonSource.front() == '(' && pPythonSource.back() == ')')) {
                try {
                    *returnedObject = pybind11::eval(pPythonSource);
                } catch (...) {
                    *returnedObject = pybind11::str(pPythonSource);
                }
            }

            // Check for boolean
            else if (pPythonSource == "True" || pPythonSource == "False") {
                *returnedObject = pybind11::bool_(pPythonSource == "True");
            }

            // Check for None
            else if (pPythonSource == "None") {
                *returnedObject = pybind11::none();
            }

            // Check for integer
            else if (std::regex_match(pPythonSource, std::regex("^-?\\d+$"))) {
                *returnedObject = pybind11::int_(std::stoll(pPythonSource));
            }

            // Check for float
            else if (std::regex_match(pPythonSource, std::regex("^-?\\d*\\.\\d+$")) ||
                std::regex_match(pPythonSource, std::regex("^-?\\d+\\.\\d*$"))) {
                *returnedObject = pybind11::float_(std::stod(pPythonSource));
            }

            //If matches nothing else, treat as string
            else {
                *returnedObject = pybind11::str(pPythonSource);
            }
        } catch (const pybind11::error_already_set& e) {
            (void)e;
            PyErr_Print();
            gvkPythonResult = GVK_PYTHON_ERROR_UNKNOWN;
        }
    } gvk_python_result_scope_end;
    return gvkPythonResult;
}

Context::ControlBlock::ControlBlock()
{
}

Context::ControlBlock::~ControlBlock()
{
}

} // namespace python
} // namespace gvk
