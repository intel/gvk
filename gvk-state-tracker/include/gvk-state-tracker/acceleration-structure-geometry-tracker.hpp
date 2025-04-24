
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

#include "gvk-handles.hpp"

namespace gvk {
namespace state_tracker {

class AccelerationStructureGeometryTracker final
{
public:
    VkResult create_resources(const gvk::Device& device, const gvk::Queue& gvkQueue);

    // NOTE : These resources shouldn't really belong to individual acceleration
    //  structures.  The GPU memcpy pipeline should be created per device when
    //  devices are created with the necessary features enabled, and the command
    //  pool and command buffer should be managed per device per thread.  But for
    //  this first pass at geometry tracking it should be fine.
    gvk::Pipeline gpuMemcpyPipeline;
    gvk::CommandPool gvkCommandPool;
    VkCommandBuffer vkCommandBuffer{ };
};

} // namespace state_tracker
} // namespace gvk
