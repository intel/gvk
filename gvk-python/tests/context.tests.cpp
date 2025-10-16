
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
#include "gvk-python/module.hpp"

#include "gtest/gtest.h"

bool error_callback(GvkPythonResult gvkPythonResult, const char* pFileLine, const char* pGvkPythonCall)
{
    (void)gvkPythonResult;
    (void)pFileLine;
    (void)pGvkPythonCall;
    return false;
}

TEST(GvkPython, python_expressions)
{
    gvk::python::gPfnGvkResultScopeCallback = error_callback;

    // Create gvk::python::Context
    GvkPythonContextCreateInfo pythonContextCreateInfo{ };
    gvk::python::Context pythonContext;
    ASSERT_EQ(gvk::python::Context::create(&pythonContextCreateInfo, &pythonContext), GVK_PYTHON_SUCCESS);

    // Evaluate Python Expression
    pybind11::object returnedObject;
    EXPECT_EQ(pythonContext.evaluate("7 * 9 + 5", &returnedObject), GVK_PYTHON_SUCCESS);

    // Validate returnedObject
    int returnedInt = pybind11::cast<int>(returnedObject);
    EXPECT_EQ(returnedInt, 68);
}

TEST(GvkPython, python_statements)
{
    gvk::python::gPfnGvkResultScopeCallback = error_callback;

    // Create gvk::python::Context
    GvkPythonContextCreateInfo pythonContextCreateInfo{ };
    gvk::python::Context pythonContext;
    ASSERT_EQ(gvk::python::Context::create(&pythonContextCreateInfo, &pythonContext), GVK_PYTHON_SUCCESS);
    pybind11::object returnedObject;

    // Execute Python Statement tests:
    //None 
    EXPECT_EQ(pythonContext.execute("s = None; print(s)", &returnedObject), GVK_PYTHON_SUCCESS);
    bool returnedBool = pybind11::isinstance<pybind11::none>(returnedObject);
    EXPECT_EQ(returnedBool, 1);

    //Boolean
    EXPECT_EQ(pythonContext.execute("s = True; print(s)", &returnedObject), GVK_PYTHON_SUCCESS);
    returnedBool = pybind11::cast<int>(returnedObject);
    EXPECT_EQ(returnedBool, 1);

    //String
    EXPECT_EQ(pythonContext.execute("s = 'Hello World'; print(s)", &returnedObject), GVK_PYTHON_SUCCESS);
    std::string returnedString = pybind11::cast<std::string>(returnedObject);
    EXPECT_EQ(returnedString, "Hello World");

    //Float 
    EXPECT_EQ(pythonContext.execute("s = 0.5543; print(s)", &returnedObject), GVK_PYTHON_SUCCESS);
    double returnedDouble = pybind11::cast<double>(returnedObject);
    EXPECT_EQ(returnedDouble, 0.5543);

    //Integer
    EXPECT_EQ(pythonContext.execute("s = 7 * 9 + 4; print(s)", &returnedObject), GVK_PYTHON_SUCCESS);
    int returnedInt = pybind11::cast<int>(returnedObject);
    EXPECT_EQ(returnedInt, 67);

    //List
    EXPECT_EQ(pythonContext.execute("s = [3, 1, 0.5, 3]; print(s)", &returnedObject), GVK_PYTHON_SUCCESS);
    std::vector<double> returnedVector = pybind11::cast<std::vector<double>>(returnedObject);
    std::vector<double> groundTruthVector = { 3, 1, 0.5, 3 };
    EXPECT_EQ(returnedVector, groundTruthVector);

    //Dict
    EXPECT_EQ(pythonContext.execute("input_dict = {'metric1': 50,'metric2' : 2.4,'metric3' : 0.002};print(input_dict)", &returnedObject), GVK_PYTHON_SUCCESS);
    std::map<std::string, double> returnedMap = pybind11::cast<std::map<std::string, double>>(returnedObject);
    std::map<std::string, double> groundTruthMap;
    groundTruthMap["metric1"] = 50;
    groundTruthMap["metric2"] = 2.4;
    groundTruthMap["metric3"] = 0.002;
    EXPECT_EQ(returnedMap, groundTruthMap);

    //Tuple, Set tests not deemed necessary
}

TEST(GvkPython, pyd_testRun)
{
    gvk::python::gPfnGvkResultScopeCallback = error_callback;

    // Create gvk::python::Context
    GvkPythonContextCreateInfo pythonContextCreateInfo{ };
    gvk::python::Context pythonContext;
    ASSERT_EQ(gvk::python::Context::create(&pythonContextCreateInfo, &pythonContext), GVK_PYTHON_SUCCESS);

    //Pyd tests using gvk::python::Module and the context created above
    int returnedInt = 15;
    GvkPythonModuleCreateInfo createInfo;
    createInfo.pSource = "unitTests.pyd";
    createInfo.sourceType = GvkPythonModuleSourceType::GVK_PYTHON_MODULE_SOURCE_TYPE_FILE;
    gvk::python::Module pydModule;
    ASSERT_EQ(gvk::python::Module::create(pythonContext, &createInfo, &pydModule), GVK_PYTHON_SUCCESS);
    pybind11::module unitTests = pydModule.get_pybind_module();

    pybind11::object mathOps = unitTests.attr("MathOps")();
    pybind11::int_ multiplyBy2_result = mathOps.attr("multiplyBy2")(returnedInt);
    returnedInt = pybind11::cast<int>(multiplyBy2_result);

    EXPECT_EQ(returnedInt, 30);
}
