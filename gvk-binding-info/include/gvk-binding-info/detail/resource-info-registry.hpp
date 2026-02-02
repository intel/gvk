
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
#include "gvk-structures.hpp"
#include "gvk-binding-info/generated/binding-info.h"
#include "gvk-binding-info/generated/binding-info-structure-comparison-operators.hpp"
#include "gvk-binding-info/generated/binding-info-structure-create-copy.hpp"
#include "gvk-binding-info/generated/binding-info-structure-destroy-copy.hpp"

#include <memory>
#include <set>

namespace gvk {
namespace detail {

class BindingRegistry final
{
public:
    class BindingMonitor;

    BindingRegistry() = default;
    ~BindingRegistry();
    void reset();
    BindingRegistry(BindingRegistry&& other) noexcept;
    BindingRegistry& operator=(BindingRegistry&& other) noexcept;
    void increment_command_count();

    const GvkResourceInfo* register_resource_info(const GvkResourceInfo& resourceInfo);
    const GvkResourceInfo* get_registered_resource_info(const GvkResourceInfo& resourceInfo) const;
    void bind(VkPipelineBindPoint bindPoint, const GvkResourceInfo* pResourceInfo, const GvkBindingInfoBaseStructure* pBindingInfo, BindingMonitor* pBindingMonitor);

private:
    void on_binding_monitor_reset(const BindingMonitor& bindingMonitor);

    uint64_t mCommandCount{ };
    std::set<gvk::Auto<GvkResourceInfo>> mResourceInfos;
    std::set<gvk::Auto<GvkBindingInfoBaseStructure>> mBindingInfos;

    BindingRegistry(const BindingRegistry&) = delete;
    BindingRegistry& operator=(BindingRegistry&) = delete;
};

} // namespace detail
} // namespace gvk
