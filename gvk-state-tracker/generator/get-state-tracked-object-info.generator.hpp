
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

#include "state-tracked-handles.generator.hpp"
#include "gvk-cppgen.hpp"

namespace gvk {
namespace cppgen {

class GetStateTrackedObjectInfoGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_STATE_TRACKER_GENERATED_SOURCE_PATH "/get-state-tracked-object-info.cpp");
        file << std::endl;
        file << "#include \"gvk-state-tracker/generated/state-tracked-handles.hpp\"" << std::endl;
        file << "#include \"gvk-state-tracker/state-tracker.hpp\"" << std::endl;
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << "#include \"VK_LAYER_INTEL_gvk_state_tracker.h\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::state_tracker");
        file << std::endl;
        file << "void StateTracker::get_state_tracked_object_info(const GvkStateTrackedObject* pStateTrackedObject, GvkStateTrackedObjectInfo* pStateTrackedObjectInfo)" << std::endl;
        file << "{" << std::endl;
        file << "    assert(pStateTrackedObject);" << std::endl;
        file << "    assert(pStateTrackedObjectInfo);" << std::endl;
        file << "    switch (pStateTrackedObject->type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator compileGuard(file, handle.compileGuards);
                file << "    case " << handle.vkObjectType << ": {" << std::endl;
                if (handle.name == "VkPhysicalDevice") {
                    file << "        *pStateTrackedObjectInfo = " << string::strip_vk(handle.name) << "(get_loader_physical_device_handle((" << handle.name << ")pStateTrackedObject->handle)).get<GvkStateTrackedObjectInfo>();" << std::endl;
                } else if (handle.isDispatchable) {
                    file << "        *pStateTrackedObjectInfo = " << string::strip_vk(handle.name) << "((" << handle.name << ")pStateTrackedObject->handle).get<GvkStateTrackedObjectInfo>();" << std::endl;
                } else {
                    file << "        auto handle = pStateTrackedObject->handle;" << std::endl;
                    file << "        auto dispatchableHandle = pStateTrackedObject->dispatchableHandle;" << std::endl;
                    file << string::replace("        {gvkHandleType} gvkHandle({ ({gvkHandleType}::DispatchableVkHandleType)dispatchableHandle, ({gvkHandleType}::VkHandleType)handle });", "{gvkHandleType}", string::strip_vk(handle.name)) << std::endl;
                    file << "        if (gvkHandle) {" << std::endl;
                    file << "            *pStateTrackedObjectInfo = gvkHandle.get<GvkStateTrackedObjectInfo>();" << std::endl;
                    file << "        }" << std::endl;
                }
                file << "    } break;" << std::endl;
            }
        }
        file << "    default: {" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
