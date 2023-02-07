
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

#include "gvk-defines.hpp"

#include <string>

namespace gvk {

/**
Gets the value of a given environment variable
@param [in] key The environment variable to get the value of
@return The value of the given environment variable
*/
std::string get_env_var(const std::string& key);

/**
Sets the value of a given environment variable
@param [in] key The environment variable to set the value of
@param [in] value The value to set for the given environment variable
*/
void set_env_var(const std::string& key, const std::string& value);

/**
Appends a given value to a given environment variable
@param [in] key The environment variable to append the given value to
@param [in] value The value to append to the given environment variable
*/
void append_value_to_env_var(const std::string& key, const std::string& value);

#ifdef GVK_PLATFORM_WINDOWS
/**
Sets the environment variable VK_LAYER_PATH with the explicit layer paths present in HKEY_LOCAL_MACHINE:"SOFTWARE\\Khronos\\Vulkan\\ExplicitLayers"
NOTE : If the environment variable VK_LAYER_PATH is already set when this function is called it noops
NOTE : After calling this function, append_value_to_env_var("VK_LAYER_PATH", "custom/layer/path") can be used to enable custom layers and built in layers simultaneously
*/
void set_vk_layer_path_from_windows_registry();
#endif

} // namespace gvk
