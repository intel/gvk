
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
#include "gvk-binding-info/detail/info-object-registry.hpp"
#include "gvk-containers/interval-tree.hpp"
#include "gvk-containers/utilities.hpp"

#include <array>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace gvk {
namespace detail {

template <typename InfoObjectType>
using BindingTracker = std::unordered_map<VkPipelineBindPoint, gvk::IntervalTree<uint64_t, std::unordered_set<const InfoObjectType*>>>;

class BindingRegistry final
{
public:
    class BindingMonitor;

    BindingRegistry() = default;
    BindingRegistry(BindingRegistry&& other) noexcept;
    BindingRegistry& operator=(BindingRegistry&& other) noexcept;
    ~BindingRegistry();
    void reset();

    uint64_t get_command_count() const;
    void increment_command_count();
    gvk::PtrArrayEnumerator<GvkResourceInfo> get_resource_infos(VkPipelineBindPoint bindPoint, uint64_t callIndex) const;
    gvk::PtrArrayEnumerator<GvkBindingInfoBaseStructure> get_binding_infos(VkPipelineBindPoint bindPoint, uint64_t callIndex) const;
    gvk::PtrArrayEnumerator<GvkBindingInfoBaseStructure> get_binding_infos(const GvkResourceInfo* pResourceInfo, VkPipelineBindPoint bindPoint, uint64_t callIndex) const;
    const GvkResourceInfo* register_resource_info(const GvkResourceInfo* pResourceInfo);
    const GvkResourceInfo* get_registered_resource_info(const GvkResourceInfo* pResourceInfo) const;
    const GvkBindingInfoBaseStructure* register_binding_info(const GvkBindingInfoBaseStructure* pBindingInfo);
    const GvkBindingInfoBaseStructure* get_registered_binding_info(const GvkBindingInfoBaseStructure* pBindingInfo) const;
    void bind(VkPipelineBindPoint bindPoint, const GvkResourceInfo* pResourceInfo, const GvkBindingInfoBaseStructure* pBindingInfo, BindingMonitor* pBindingMonitor);

private:
    void on_binding_monitor_reset(BindingMonitor& bindingMonitor);

    uint64_t mCommandCount{ };
    InfoObjectRegistry<gvk::Auto, GvkResourceInfo> mResourceInfos;
    InfoObjectRegistry<gvk::TypeErasedAuto, GvkBindingInfoBaseStructure> mBindingInfos;
    BindingTracker<GvkResourceInfo> mResourceInfoBindings;
    BindingTracker<GvkBindingInfoBaseStructure> mBindingInfoBindings;
    std::unordered_map<const GvkResourceInfo*, BindingTracker<GvkBindingInfoBaseStructure>> mBindingMap;

    BindingRegistry(const BindingRegistry&) = delete;
    BindingRegistry& operator=(BindingRegistry&) = delete;
};

} // namespace detail
} // namespace gvk
