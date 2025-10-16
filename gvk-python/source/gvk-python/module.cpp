
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

#include "gvk-python/module.hpp"

#include <filesystem>
#include <iostream>

namespace gvk {
namespace python {

GvkPythonResult Module::create(const Context& context, const GvkPythonModuleCreateInfo* pCreateInfo, Module* pModule)
{
    //Developer note: Whatever is in the module will be differnet for each module instance
    gvk_python_result_scope_begin(GVK_PYTHON_SUCCESS) {
        gvk_python_result(context ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(pCreateInfo ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(pCreateInfo->pSource ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(pModule ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);

        // Initialize new reference and context
        pModule->mReference.reset(newref);
        auto& controlBlock = pModule->mReference.get_obj();
        controlBlock.mContext = context;

        // Check source type and route to correct initialization method
        switch (pCreateInfo->sourceType) {
        case GVK_PYTHON_MODULE_SOURCE_TYPE_FILE: {

            // Check extension and route to correct initialization method
            auto extension = std::filesystem::path(pCreateInfo->pSource).extension();
            if (extension == ".py") {
                gvk_python_result(pModule->initialize_from_py(*pCreateInfo));
            } else if (extension == ".pyd") {
                gvk_python_result(pModule->initialize_from_pyd(*pCreateInfo));
            }
        } break;
        case GVK_PYTHON_MODULE_SOURCE_TYPE_CODE: {
            gvk_python_result(pModule->initialize_from_code(*pCreateInfo));
        } break;
        default: {
            gvk_python_result(GVK_PYTHON_ERROR_UNKNOWN);
        } break;
        }
    } gvk_python_result_scope_end;
    if (gvkPythonResult != GVK_PYTHON_SUCCESS) {
        pModule->reset();
    }
    return gvkPythonResult;
}

Module::~Module()
{
    reset();
}

void Module::reset()
{
    mReference.reset();
}

const Context& Module::get_context() const
{
    assert(mReference);
    return mReference.get_obj().mContext;
}

const pybind11::module& Module::get_pybind_module() const
{
    assert(mReference);
    return mReference.get_obj().mPyBindModule;
}

GvkPythonResult Module::initialize_from_py(const GvkPythonModuleCreateInfo& createInfo)
{
    gvk_python_result_scope_begin(GVK_PYTHON_SUCCESS) {
        gvk_python_result(createInfo.pSource ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(mReference ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        auto& controlBlock = mReference.get_obj();
        (void)controlBlock;

        // NOTE : This one should almost certainly read code from the file,
        // then route that to the initialize_from_code() method

        // Do initialization stuff here...
        gvk_python_result(GVK_PYTHON_ERROR_UNKNOWN);

    } gvk_python_result_scope_end;
    return gvkPythonResult;
}

GvkPythonResult Module::initialize_from_pyd(const GvkPythonModuleCreateInfo& createInfo)
{
    gvk_python_result_scope_begin(GVK_PYTHON_SUCCESS) {
        gvk_python_result(createInfo.pSource ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(mReference ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        auto& controlBlock = mReference.get_obj();
        (void)controlBlock;


        //Test pyd
        pybind11::module sys = pybind11::module::import("sys");
        pybind11::list   path = pybind11::cast<pybind11::list>(sys.attr("path"));
        std::filesystem::path pydPath = std::filesystem::current_path();
        path.insert(0, pydPath.string());

        // Check if file exists
        assert(std::filesystem::exists(createInfo.pSource));

        try {
            std::string filePath = std::filesystem::path(createInfo.pSource).filename().string();
            filePath = filePath.substr(0, filePath.find_first_of("."));
            mReference.get_obj().mPyBindModule = pybind11::module::import(filePath.c_str());
        } catch (const std::exception& e) {
            std::cout << "Import failed: " << e.what() << std::endl; //This will get routed to error callback eventually
            gvkPythonResult = GVK_PYTHON_ERROR_UNKNOWN;
        }
        gvk_python_result(gvkPythonResult);

    } gvk_python_result_scope_end;
    return gvkPythonResult;
}

GvkPythonResult Module::initialize_from_code(const GvkPythonModuleCreateInfo& createInfo)
{
    gvk_python_result_scope_begin(GVK_PYTHON_SUCCESS) {
        gvk_python_result(createInfo.pSource ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        gvk_python_result(mReference ? GVK_PYTHON_SUCCESS : GVK_PYTHON_ERROR_UNKNOWN);
        auto& controlBlock = mReference.get_obj();
        (void)controlBlock;

        // Do initialization stuff here...
        gvk_python_result(GVK_PYTHON_ERROR_UNKNOWN);

    } gvk_python_result_scope_end;
    return gvkPythonResult;
}

Module::ControlBlock::ControlBlock()
{
}

Module::ControlBlock::~ControlBlock()
{
}

} // namespace python
} // namespace gvk
