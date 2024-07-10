
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
#include "gvk-restore-info.hpp"
#include "gvk-command-structures.hpp"

#include "gvk-restore-point/object-map.hpp"

#define VK_LAYER_INTEL_gvk_restore_point_hpp_OMIT_ENTRY_POINT_DECLARATIONS
#include "VK_LAYER_INTEL_gvk_restore_point.hpp"

#include <map>
#include <set>

struct GvkRestorePoint_T
{
    GvkRestorePointCreateFlags createFlags{ };
    gvk::Auto<GvkRestorePointManifest> manifest;
    gvk::restore_point::ObjectMap objectMap;

    // TODO : Wrap these so accessors automatically handle associations correctly
    std::set<gvk::restore_point::CapturedObject> objectRestorationSubmitted;
    std::set<gvk::restore_point::CapturedObject> stateRestorationRequired;
    std::set<gvk::restore_point::CapturedObject> dataRestorationRequired;
    std::set<gvk::restore_point::CapturedObject> mappingRestorationRequired;
    std::set<gvk::restore_point::RestoredObject> objectDestructionRequired; // NOTE : Must always be the live object
    std::set<gvk::restore_point::RestoredObject> objectDestructionSubmitted; // NOTE : Must always be the live object
    std::set<GvkStateTrackedObject> createdObjects;
};
