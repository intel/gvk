
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

#include <cassert>

namespace gvk {
namespace cppgen {

class GetStateTrackedObjectCreateInfoGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_STATE_TRACKER_GENERATED_SOURCE_PATH "/get-state-tracked-object-create-info.cpp");
        file << std::endl;
        file << "#include \"gvk-defines.hpp\"" << std::endl;
        file << "#include \"gvk-structures/defaults.hpp\"" << std::endl;
        file << "#include \"gvk-structures/get-stype.hpp\"" << std::endl;
        file << "#include \"gvk-state-tracker/generated/state-tracked-handles.hpp\"" << std::endl;
        file << "#include \"gvk-state-tracker/state-tracker.hpp\"" << std::endl;
        file << "#include \"VK_LAYER_INTEL_gvk_state_tracker.h\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::state_tracker");
        file << std::endl;
        file << "void StateTracker::get_state_tracked_object_create_info(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo)" << std::endl;
        file << "{" << std::endl;
        file << "    assert(pStateTrackedObject);" << std::endl;
        file << "    assert(pCreateInfoType);" << std::endl;
        file << "    switch (pStateTrackedObject->type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator handleCompileGuardGenerator(file, handle.compileGuards);
                file << "    case " << handle.vkObjectType << ": {" << std::endl;
                if (handle.isDispatchable) {
                    file << string::replace("        {gvkHandleType} gvkHandle(({gvkHandleType}::VkHandleType)pStateTrackedObject->handle);", {
                        { "{gvkHandleType}", string::strip_vk(handle.name) },
                    }) << std::endl;
                } else {
                    file << string::replace("        {gvkHandleType} gvkHandle({ ({gvkHandleType}::DispatchableVkHandleType)pStateTrackedObject->dispatchableHandle, ({gvkHandleType}::VkHandleType)pStateTrackedObject->handle });", {
                        { "{gvkHandleType}", string::strip_vk(handle.name) },
                    }) << std::endl;
                }
                auto createInfos = handle.createInfos;
                if (handle.name == "VkQueue") {
                    assert(createInfos.empty());
                    createInfos.insert("VkDeviceQueueCreateInfo");
                }
                for (const auto& createInfoName : createInfos) {
                    const auto& createInfoStructureItr = manifest.structures.find(createInfoName);
                    assert(createInfoStructureItr != manifest.structures.end());
                    const auto& createInfoStructure = createInfoStructureItr->second;
                    CompileGuardGenerator createInfoCompileGuardGenerator(file, get_inner_scope_compile_guards(handle.compileGuards, createInfoStructure.compileGuards));
                    file << "        if (gvkHandle && gvkHandle.get<" << createInfoStructure.name << ">().sType == get_stype<" << createInfoStructure.name << ">()) {" << std::endl;
                    file << "            if (pCreateInfo) {" << std::endl;
                    file << "                *pCreateInfo = { };" << std::endl;
                    file << "                if (*pCreateInfoType == gvkHandle.get<" << createInfoStructure.name << ">().sType) {" << std::endl;
                    file << "                    *(" << createInfoStructure.name << "*)pCreateInfo = gvkHandle.get<" << createInfoStructure.name << ">();" << std::endl;
                    file << "                }" << std::endl;
                    file << "            } else {" << std::endl;
                    file << "                *pCreateInfoType = gvkHandle.get<" << createInfoStructure.name << ">().sType;" << std::endl;
                    file << "            }" << std::endl;
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
