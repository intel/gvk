
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

#include "gvk-state-tracker/device-address-tracker.hpp"

namespace gvk {
namespace state_tracker {

void DeviceAddressTracker::reset()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mBuffers.clear();
    mDeviceAddresses.clear();
}

void DeviceAddressTracker::add(VkBuffer buffer, VkDeviceAddress deviceAddress)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mBuffers[deviceAddress] = buffer;
    mDeviceAddresses[buffer] = deviceAddress;
}

void DeviceAddressTracker::erase(VkBuffer buffer)
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto deviceAddressItr = mDeviceAddresses.find(buffer);
    if (deviceAddressItr != mDeviceAddresses.end()) {
        auto bufferItr = mBuffers.find(deviceAddressItr->second);
        assert(bufferItr != mBuffers.end());
        mBuffers.erase(bufferItr);
        mDeviceAddresses.erase(deviceAddressItr);
    }
}

VkDeviceAddress DeviceAddressTracker::get_device_address(VkBuffer buffer) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto deviceAddressItr = mDeviceAddresses.find(buffer);
    return deviceAddressItr != mDeviceAddresses.end() ? deviceAddressItr->second : VkDeviceAddress{ };
}

VkBuffer DeviceAddressTracker::get_buffer(VkDeviceAddress deviceAddress) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto bufferItr = mBuffers.find(deviceAddress);
    return bufferItr != mBuffers.end() ? bufferItr->second : VkBuffer{ };
}

} // namespace state_tracker
} // namespace gvk
