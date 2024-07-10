
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

#include <map>
#include <mutex>

namespace gvk {
namespace restore_point {

using CapturedObject = GvkStateTrackedObject;
using RestoredObject = GvkStateTrackedObject;

class ObjectMap final
{
public:
    bool register_object_restoration(const CapturedObject& capturedObject, const RestoredObject& restoredObject);
    void register_object_destruction(const RestoredObject& restoredObject);
    bool set_object_mapping(const CapturedObject& capturedObject, const RestoredObject& restoredObject);
    const std::map<CapturedObject, RestoredObject>& get_restored_objects() const;
    const std::map<RestoredObject, RestoredObject>& get_captured_objects() const;
    RestoredObject get_restored_object(const CapturedObject& capturedObject) const;
    CapturedObject get_captured_object(const RestoredObject& restoredObject) const;
    size_t size() const;
    void clear();

private:
    mutable std::mutex mMutex;
    std::map<CapturedObject, RestoredObject> mRestoredObjects;
    std::map<RestoredObject, CapturedObject> mCapturedObjects;
};

} // namespace restore_point
} // namespace gvk
