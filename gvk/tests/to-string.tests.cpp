
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

#include "gvk/to-string.hpp"
#include "gvk/system/random.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

TEST(to_string, to_string)
{
    VkInstanceCreateInfo instanceCreateInfo{ };
    EXPECT_EQ(gvk::to_string(instanceCreateInfo), R"({
    "sType": {
        "identifier": "VK_STRUCTURE_TYPE_APPLICATION_INFO",
        "value": 0
    },
    "pNext": null,
    "flags": {
        "identifier": "",
        "value": 0
    },
    "pApplicationInfo": null,
    "enabledLayerCount": 0,
    "ppEnabledLayerNames": null,
    "enabledExtensionCount": 0,
    "ppEnabledExtensionNames": null
})");

    VkApplicationInfo applicationInfo{ };
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "GVK Tests";
    applicationInfo.pEngineName = "Intel GVK";
    std::vector<const char*> enabledLayerNames{
        "VkLayer_0",
        "VkLayer_1",
        "VkLayer_2",
    };
    std::vector<const char*> enabledExtensionNames{
        "VkExtension_0",
        nullptr,
        "VkExtension_1",
        "VkExtension_2",
    };
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayerNames.size();
    instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
    instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensionNames.size();
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();
    EXPECT_EQ(gvk::to_string(instanceCreateInfo), R"({
    "sType": {
        "identifier": "VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO",
        "value": 1
    },
    "pNext": null,
    "flags": {
        "identifier": "",
        "value": 0
    },
    "pApplicationInfo": {
        "sType": {
            "identifier": "VK_STRUCTURE_TYPE_APPLICATION_INFO",
            "value": 0
        },
        "pNext": null,
        "pApplicationName": "GVK Tests",
        "applicationVersion": 0,
        "pEngineName": "Intel GVK",
        "engineVersion": 0,
        "apiVersion": 0
    },
    "enabledLayerCount": 3,
    "ppEnabledLayerNames": [
        "VkLayer_0",
        "VkLayer_1",
        "VkLayer_2"
    ],
    "enabledExtensionCount": 4,
    "ppEnabledExtensionNames": [
        "VkExtension_0",
        null,
        "VkExtension_1",
        "VkExtension_2"
    ]
})");

    VkBufferViewCreateInfo bufferViewCreateInfo{ };
    bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bufferViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    EXPECT_EQ(gvk::to_string(bufferViewCreateInfo), R"({
    "sType": {
        "identifier": "VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO",
        "value": 13
    },
    "pNext": null,
    "flags": 0,
    "buffer": "VK_NULL_HANDLE",
    "format": {
        "identifier": "VK_FORMAT_R8G8B8A8_UNORM",
        "value": 37
    },
    "offset": 0,
    "range": 0
})");

    VkBuffer vkBuffer = (VkBuffer)64;
    bufferViewCreateInfo.buffer = vkBuffer;
    EXPECT_EQ(gvk::to_string(bufferViewCreateInfo), R"({
    "sType": {
        "identifier": "VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO",
        "value": 13
    },
    "pNext": null,
    "flags": 0,
    "buffer": "0x0000000000000040",
    "format": {
        "identifier": "VK_FORMAT_R8G8B8A8_UNORM",
        "value": 37
    },
    "offset": 0,
    "range": 0
})");

    VkBufferCreateInfo bufferCreateInfo{ };
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    EXPECT_EQ(gvk::to_string(bufferCreateInfo), R"({
    "sType": {
        "identifier": "VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO",
        "value": 12
    },
    "pNext": null,
    "flags": {
        "identifier": "VK_BUFFER_CREATE_SPARSE_BINDING_BIT|VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT",
        "value": 17
    },
    "size": 0,
    "usage": {
        "identifier": "",
        "value": 0
    },
    "sharingMode": {
        "identifier": "VK_SHARING_MODE_EXCLUSIVE",
        "value": 0
    },
    "queueFamilyIndexCount": 0,
    "pQueueFamilyIndices": null
})");

    VkBufferDeviceAddressCreateInfoEXT bufferDeviceAddressCreateInfo{ };
    bufferDeviceAddressCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT;
    bufferCreateInfo.pNext = &bufferDeviceAddressCreateInfo;
    EXPECT_EQ(gvk::to_string(bufferCreateInfo), R"({
    "sType": {
        "identifier": "VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO",
        "value": 12
    },
    "pNext": {
        "sType": {
            "identifier": "VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT",
            "value": 1000244002
        },
        "pNext": null,
        "deviceAddress": 0
    },
    "flags": {
        "identifier": "VK_BUFFER_CREATE_SPARSE_BINDING_BIT|VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT",
        "value": 17
    },
    "size": 0,
    "usage": {
        "identifier": "",
        "value": 0
    },
    "sharingMode": {
        "identifier": "VK_SHARING_MODE_EXCLUSIVE",
        "value": 0
    },
    "queueFamilyIndexCount": 0,
    "pQueueFamilyIndices": null
})");

    gvk::sys::RandomNumberGenerator rng;
    auto makeFloat = [](const char* pFloatStr)
    {
        float value;
        std::stringstream strStrm;
        strStrm << pFloatStr;
        strStrm >> value;
        return value;
    };
    VkTransformMatrixKHR transformMatrix{
                                  0,                           0, makeFloat("7.00214748e+37"), makeFloat("3.77726971e+37"),
        makeFloat("1.44432763e+38"),                           0,                           0,                           0,
                                  0, makeFloat("1.00168219e+38"),                           0, makeFloat("1.97888940e+38"),
    };
    EXPECT_EQ(gvk::to_string(transformMatrix), R"({
    "matrix": [
        [ 0.00000000e+00, 0.00000000e+00, 7.00214748e+37, 3.77726971e+37 ],
        [ 1.44432763e+38, 0.00000000e+00, 0.00000000e+00, 0.00000000e+00 ],
        [ 0.00000000e+00, 1.00168219e+38, 0.00000000e+00, 1.97888940e+38 ]
    ]
})");
}
