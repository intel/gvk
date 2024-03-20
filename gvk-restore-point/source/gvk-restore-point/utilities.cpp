
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

#include "gvk-restore-point/utilities.hpp"

namespace gvk {
namespace restore_point {

ObjectMap::ObjectMap(const GvkRestorePointManifest& manifest)
    : mManifest(manifest)
{
    // NOTE : This needs to be cleared if doing deferred...
    for (uint32_t i = 0; i < mManifest->objectCount; ++i) {
        const auto& capturedObject = mManifest->pObjects[i];
        assert(capturedObject.type);
        assert(capturedObject.handle);
        assert(capturedObject.dispatchableHandle);
        mRestoredObjects[capturedObject] = capturedObject;
    }
}

const GvkRestorePointManifest& ObjectMap::get_manifest() const
{
    return mManifest;
}

void ObjectMap::register_object_restoration(const CapturedObject& capturedObject, const RestoredObject& restoredObject)
{
    // TODO : Need to figure out how upper layers should create persistent objects
    (void)capturedObject;
    (void)restoredObject;
}

void ObjectMap::register_object_destruction(const GvkRestorePointObject& object)
{
    // TODO : Need to figure out how upper layers should create persistent objects
    (void)object;
}

void ObjectMap::set_object_mapping(const CapturedObject& capturedObject, const RestoredObject& restoredObject)
{
    // TODO : Need to figure out how upper layers should create persistent objects
    (void)capturedObject;
    (void)restoredObject;
#if 0
    auto itr = mRestoredObjects.find(capturedObject);
    assert(itr != mRestoredObjects.end());
    assert(!itr->second.type);
    assert(!itr->second.handle);
    assert(!itr->second.dispatchableHandle);
    itr->second = restoredObject;
    assert(!restoredObject.type == !restoredObject.handle);
    assert(!restoredObject.type == !restoredObject.dispatchableHandle);
    if (restoredObject.type) {
        mCapturedObjects[restoredObject] = capturedObject;
    }
#endif
}

const std::map<CapturedObject, RestoredObject>& ObjectMap::get_restored_objects() const
{
    return mRestoredObjects;
}

RestoredObject ObjectMap::get_restored_object(const CapturedObject& capturedObject) const
{
    auto itr = mRestoredObjects.find(capturedObject);
    return itr != mRestoredObjects.end() ? itr->second : RestoredObject{ };
}

CapturedObject ObjectMap::get_captured_object(const RestoredObject& restoredObject) const
{
    auto itr = mCapturedObjects.find(restoredObject);
    return itr != mCapturedObjects.end() ? itr->second : CapturedObject{ };
}

void LayerInfo::register_object_creation(const GvkRestorePointObject& restorePointObject)
{
    createdObjects.insert(restorePointObject);
}

void LayerInfo::register_object_destruction(const GvkRestorePointObject& restorePointObject)
{
    createdObjects.erase(restorePointObject);
}

} // namespace restore_point
} // namespace gvk
