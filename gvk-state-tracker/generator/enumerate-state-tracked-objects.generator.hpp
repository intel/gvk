
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

class EnumerateStateTrackedObjectsGenerator final
{
public:
    static void generate(const xml::Manifest& manifest)
    {
        FileGenerator file(GVK_STATE_TRACKER_GENERATED_SOURCE_PATH "/enumerate-state-tracked-objects.cpp");
        file << std::endl;
        file << "#include \"gvk-layer/registry.hpp\"" << std::endl;
        file << "#include \"gvk-state-tracker/generated/state-tracked-handles.hpp\"" << std::endl;
        file << "#include \"gvk-state-tracker/dependency-enumerator.hpp\"" << std::endl;
        file << "#include \"gvk-state-tracker/state-tracker.hpp\"" << std::endl;
        file << "#include \"gvk-structures/get-object-type.hpp\"" << std::endl;
        file << std::endl;
        NamespaceGenerator namespaceGenerator(file, "gvk::state_tracker");
        file << std::endl;
        file << "void StateTracker::enumerate_state_tracked_objects(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)" << std::endl;
        file << "{" << std::endl;
        file << "    assert(pStateTrackedObject);" << std::endl;
        file << "    assert(pEnumerateInfo);" << std::endl;
        file << "    switch (pStateTrackedObject->type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator handleCompileGuardGenerator(file, handle.compileGuards);
                file << "    case " << handle.vkObjectType << ": {" << std::endl;
                auto tab = "        ";
                file << tab << string::replace("{gvkHandleType} handle(to_handle_id<{gvkHandleType}::HandleIdType>(*pStateTrackedObject));", "{gvkHandleType}", string::strip_vk(handle.name)) << std::endl;
                file << tab << "if (handle) {" << std::endl;
                StateTrackedHandleGenerator handleGenerator(manifest, handle);
                for (const auto& member : handleGenerator.get_members()) {
                    if (string::contains(member.storageType, "ObjectTracker")) {
                        CompileGuardGenerator memberCompileGuardGenerator(file, get_inner_scope_compile_guards(handle.compileGuards, member.compileGuards));
                        file << tab << "    handle.mReference.get_obj()." << member.storageName << ".enumerate(pEnumerateInfo->pfnCallback, pEnumerateInfo->pUserData);" << std::endl;
                    }
                }
                file << tab << "}" << std::endl;
                file << "    } break;" << std::endl;
            }
        }
        file << "    default: {" << std::endl;
        file << "        assert(false && \"Unrecognized VkObjectType\");" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
        file << "void StateTracker::enumerate_state_tracked_object_dependencies(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo)" << std::endl;
        file << "{" << std::endl;
        file << "    assert(pStateTrackedObject);" << std::endl;
        file << "    assert(pEnumerateInfo);" << std::endl;
        file << "    DependencyEnumerator dependencyEnuemrator(*pEnumerateInfo);" << std::endl;
        file << "    switch (pStateTrackedObject->type) {" << std::endl;
        for (const auto& handleItr : manifest.handles) {
            const auto& handle = handleItr.second;
            if (handle.alias.empty()) {
                CompileGuardGenerator handleCompileGuardGenerator(file, handle.compileGuards);
                file << "    case " << handle.vkObjectType << ": {" << std::endl;
                auto tab = "        ";
                file << tab << string::replace("{gvkHandleType} handle(to_handle_id<{gvkHandleType}::HandleIdType>(*pStateTrackedObject));", "{gvkHandleType}", string::strip_vk(handle.name)) << std::endl;
                file << tab << "if (handle) {" << std::endl;
                StateTrackedHandleGenerator handleGenerator(manifest, handle);
                for (const auto& member : handleGenerator.get_members()) {
                    if (member.storageType != handle.name) {
                        if (manifest.handles.count("Vk" + string::remove(string::remove(member.storageType, "std::vector<"), ">"))) {
                            CompileGuardGenerator memberCompileGuardGenerator(file, get_inner_scope_compile_guards(handle.compileGuards, member.compileGuards));
                            if (string::contains(member.storageType, "std::vector")) {
                                file << tab << "    for (const auto& dependency : handle.mReference.get_obj()." << member.storageName << ") {" << std::endl;
                                file << tab << "        dependency.enumerate_dependencies(DependencyEnumerator::enumerate, &dependencyEnuemrator);" << std::endl;
                                file << tab << "    }" << std::endl;
                            } else {
                                file << tab << "    handle.mReference.get_obj()." << member.storageName << ".enumerate_dependencies(DependencyEnumerator::enumerate, &dependencyEnuemrator);" << std::endl;
                            }
                        } else if (manifest.handles.count(member.storageType)) {
                            assert(member.storageType == member.accessorType);
                            file << tab << "    auto parentStateTrackedObject = *pStateTrackedObject;" << std::endl;
                            file << tab << "    parentStateTrackedObject.type = detail::get_object_type<" << member.storageType << ">();" << std::endl;
                            file << tab << "    parentStateTrackedObject.handle = (uint64_t)handle.get<" << member.accessorType << ">();" << std::endl;
                            file << tab << "    auto parentHandle = to_handle<" << string::strip_vk(member.storageType) << ">(parentStateTrackedObject);" << std::endl;
                            file << tab << "    parentHandle.enumerate_dependencies(DependencyEnumerator::enumerate, &dependencyEnuemrator);" << std::endl;
                        } else if (member.storageName == "mImmutableSamplers") {
                            file << "            for (const auto& immutableSamplersItr : handle.mReference.get_obj().mImmutableSamplers) {" << std::endl;
                            file << "                for (const auto& immutableSampler : immutableSamplersItr.second) {" << std::endl;
                            file << "                    immutableSampler.enumerate_dependencies(DependencyEnumerator::enumerate, &dependencyEnuemrator);" << std::endl;
                            file << "                }" << std::endl;
                            file << "            }" << std::endl;
                        } else if (member.storageName == "mCmdTracker") {
                            file << "            handle.mReference.get_obj().mCmdTracker.enumerate_dependencies(DependencyEnumerator::enumerate, &dependencyEnuemrator);" << std::endl;
                        }
                    }
                }
                file << tab << "}" << std::endl;
                file << "    } break;" << std::endl;
            }
        }
        file << "    default: {" << std::endl;
        file << "        assert(false && \"Unrecognized VkObjectType\");" << std::endl;
        file << "    } break;" << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }
};

} // namespace cppgen
} // namespace gvk
