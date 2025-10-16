
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

#include "gvk-python/defines.hpp"
#include "gvk-python/context.hpp"
#include "gvk-reference.hpp"

#include "pybind11/embed.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

enum GvkPythonModuleSourceType
{
    GVK_PYTHON_MODULE_SOURCE_TYPE_FILE,
    GVK_PYTHON_MODULE_SOURCE_TYPE_CODE,
};

struct GvkPythonModuleCreateInfo
{
    GvkPythonModuleSourceType sourceType;
    const char* pSource;
};

namespace gvk {
namespace python {

class Module final
{
public:
    static GvkPythonResult create(const Context& context, const GvkPythonModuleCreateInfo* pCreateInfo, Module* pModule);
    ~Module();
    void reset();

    const Context& get_context() const;

    const pybind11::module& get_pybind_module() const;

private:
    GvkPythonResult initialize_from_py(const GvkPythonModuleCreateInfo& createInfo);
    GvkPythonResult initialize_from_pyd(const GvkPythonModuleCreateInfo& createInfo);
    GvkPythonResult initialize_from_code(const GvkPythonModuleCreateInfo& createInfo);

    class ControlBlock final
    {
    public:
        ControlBlock();
        ~ControlBlock();
        Context mContext;
        pybind11::module mPyBindModule;
    private:
        ControlBlock(const ControlBlock&) = delete;
        ControlBlock& operator=(const ControlBlock&) = delete;
    };

    gvk_reference_type(Module)
};

} // namespace python
} // namespace gvk
