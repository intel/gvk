
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

#include "gvk-binding-info/detail/binding-registry.hpp"
#include "gvk-binding-info/detail/binding-monitor.hpp"

#include <utility>

namespace gvk {
namespace detail {

template <typename InfoObjectType>
static gvk::PtrArrayEnumerator<InfoObjectType> get_info_objects(
    const BindingTracker<InfoObjectType>& bindingTracker,
    VkPipelineBindPoint bindPoint,
    gvk::Interval<uint64_t> interval
)
{
    thread_local std::vector<const InfoObjectType*> tlInfos;
    tlInfos.clear();
    interval.second = std::max(interval.first, interval.second);
    auto bindPointItr = bindingTracker.find(bindPoint);
    if (bindPointItr != bindingTracker.end()) {
        bindPointItr->second.enumerate(
            interval,
            [](const gvk::Interval<uint64_t>&, const std::unordered_set<const InfoObjectType*>& infos)
            {
                for (auto pInfo : infos) {
                    assert(pInfo && "TODO : Documentation; gvk maintenance required");
                    tlInfos.push_back(pInfo);
                }
            }
        );
    }

    if (!tlInfos.empty()) {
        // TODO : Filtering duplicates from tlInfos works fine, but it would be better
        //  to wrangle this at the BindingInfo level...when we're ready to expose
        //  binding intervals we should add some logic to merge intervals.  That will
        //  take care of this case and cases where we end up with split intervals due
        //  to multiple usages that overlap but have different begin/end points for the
        //  same resource.
        // NOTE : It would be pretty straight forward to simply merge any adjacent
        //  interavls that can be merged, but we want to make sure that we don't end up
        //  hiding any real bind/unbinds a workload might be doing.
        auto begin = tlInfos.data();
        auto end = begin + tlInfos.size();
        return gvk::PtrArrayEnumerator<InfoObjectType>(begin, end);
    }
    return { };
}

BindingRegistry::BindingRegistry(BindingRegistry&& other) noexcept
{
    *this = std::move(other);
}

BindingRegistry& BindingRegistry::operator=(BindingRegistry&& other) noexcept
{
    if (this != &other) {
        mCommandCount = std::exchange(other.mCommandCount, { });
        mResourceInfos = std::exchange(other.mResourceInfos, { });
        mBindingInfos = std::exchange(other.mBindingInfos, { });
        mResourceInfoBindings = std::exchange(other.mResourceInfoBindings, { });
        mBindingInfoBindings = std::exchange(other.mBindingInfoBindings, { });
    }
    return *this;
}

BindingRegistry::~BindingRegistry()
{
    reset();
}

void BindingRegistry::reset()
{
    mCommandCount = 0;
    mResourceInfos.reset();
    mBindingInfos.reset();
    mResourceInfoBindings.clear();
    mBindingInfoBindings.clear();
}

uint64_t BindingRegistry::get_command_count() const
{
    return mCommandCount;
}

void BindingRegistry::increment_command_count()
{
    ++mCommandCount;
}

gvk::PtrArrayEnumerator<GvkResourceInfo> BindingRegistry::get_resource_infos(VkPipelineBindPoint bindPoint, uint64_t callIndex) const
{
    return get_info_objects<GvkResourceInfo>(mResourceInfoBindings, bindPoint, { callIndex, callIndex });
}

gvk::PtrArrayEnumerator<GvkBindingInfoBaseStructure> BindingRegistry::get_binding_infos(VkPipelineBindPoint bindPoint, uint64_t callIndex) const
{
    return get_info_objects<GvkBindingInfoBaseStructure>(mBindingInfoBindings, bindPoint, { callIndex, callIndex });
}

gvk::PtrArrayEnumerator<GvkBindingInfoBaseStructure> BindingRegistry::get_binding_infos(const GvkResourceInfo* pResourceInfo, VkPipelineBindPoint bindPoint, uint64_t callIndex) const
{
    auto itr = mBindingMap.find(pResourceInfo);
    return itr != mBindingMap.end() ? get_info_objects<GvkBindingInfoBaseStructure>(itr->second, bindPoint, { callIndex, callIndex }) : gvk::PtrArrayEnumerator<GvkBindingInfoBaseStructure> { };
}

const GvkResourceInfo* BindingRegistry::register_resource_info(const GvkResourceInfo* pResourceInfo)
{
    return pResourceInfo ? mResourceInfos.register_info_object(*pResourceInfo) : nullptr;
}

const GvkResourceInfo* BindingRegistry::get_registered_resource_info(const GvkResourceInfo* pResourceInfo) const
{
    return pResourceInfo ? mResourceInfos.get_registered_info_object(*pResourceInfo) : nullptr;
}

const GvkBindingInfoBaseStructure* BindingRegistry::register_binding_info(const GvkBindingInfoBaseStructure* pBindingInfo)
{
    return pBindingInfo ? mBindingInfos.register_info_object(*pBindingInfo) : nullptr;
}

const GvkBindingInfoBaseStructure* BindingRegistry::get_registered_binding_info(const GvkBindingInfoBaseStructure* pBindingInfo) const
{
    return pBindingInfo ? mBindingInfos.get_registered_info_object(*pBindingInfo) : nullptr;
}

void BindingRegistry::bind(VkPipelineBindPoint bindPoint, const GvkResourceInfo* pResourceInfo, const GvkBindingInfoBaseStructure* pBindingInfo, BindingMonitor* pBindingMonitor)
{
    BindingMonitor bindingMonitor;
    if (pBindingMonitor) {
        pBindingMonitor->reset();
    } else {
        pBindingMonitor = &bindingMonitor;
    }
    pBindingMonitor->mBindPoint = bindPoint;
    pBindingMonitor->mInterval.first = mCommandCount;
    pBindingMonitor->mpResourceInfo = pResourceInfo;
    pBindingMonitor->mpBindingInfo = pBindingInfo;
    pBindingMonitor->mpResourceRegistry = this;
}

void BindingRegistry::on_binding_monitor_reset(BindingMonitor& bindingMonitor)
{
    // NOTE : If mInterval.first == mInterval.second then the binding is available for
    //  just that call index, otherwise the binding is removed from the current call
    //  index by decrementing mInterval.second.
    bindingMonitor.mInterval.second = mCommandCount;
    if (bindingMonitor.mInterval.first < bindingMonitor.mInterval.second) {
        --bindingMonitor.mInterval.second;
    }
    if (bindingMonitor.mpResourceInfo) {
        mResourceInfoBindings[bindingMonitor.mBindPoint][bindingMonitor.mInterval].insert(bindingMonitor.mpResourceInfo);
        if (bindingMonitor.mpBindingInfo) {
            mBindingMap[bindingMonitor.mpResourceInfo][bindingMonitor.mBindPoint][bindingMonitor.mInterval].insert(bindingMonitor.mpBindingInfo);
        }
    }
    if (bindingMonitor.mpBindingInfo) {
        mBindingInfoBindings[bindingMonitor.mBindPoint][bindingMonitor.mInterval].insert(bindingMonitor.mpBindingInfo);
    }
}

} // namespace detail
} // namespace gvk
