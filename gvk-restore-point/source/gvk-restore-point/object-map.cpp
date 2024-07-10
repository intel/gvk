
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

#include "gvk-restore-point/object-map.hpp"
#include "gvk-restore-point/utilities.hpp"

namespace gvk {
namespace restore_point {

bool ObjectMap::register_object_restoration(const CapturedObject& capturedObject, const RestoredObject& restoredObject)
{
    return set_object_mapping(capturedObject, restoredObject);
}

void ObjectMap::register_object_destruction(const RestoredObject& restoredObject)
{
    assert(is_valid(restoredObject));
    std::lock_guard<std::mutex> lock(mMutex);
    auto itr = mCapturedObjects.find(restoredObject);
    if (itr != mCapturedObjects.end()) {
        mRestoredObjects.erase(itr->second);
        mCapturedObjects.erase(itr);
    }
}

bool ObjectMap::set_object_mapping(const CapturedObject& capturedObject, const RestoredObject& restoredObject)
{
    assert(is_valid(capturedObject));
    assert(is_valid(restoredObject));
    std::lock_guard<std::mutex> lock(mMutex);
    auto restoredObjectInserted = mRestoredObjects.insert({ capturedObject, restoredObject }).second;
    auto capturedObjectInserted = mCapturedObjects.insert({ restoredObject, capturedObject }).second;
    assert(restoredObjectInserted == capturedObjectInserted);
    return restoredObjectInserted && capturedObjectInserted;
}

const std::map<CapturedObject, RestoredObject>& ObjectMap::get_restored_objects() const
{
    return mRestoredObjects;
}

const std::map<RestoredObject, CapturedObject>& ObjectMap::get_captured_objects() const
{
    return mCapturedObjects;
}

RestoredObject ObjectMap::get_restored_object(const CapturedObject& capturedObject) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto itr = mRestoredObjects.find(capturedObject);
    return itr != mRestoredObjects.end() ? itr->second : RestoredObject{ };
}

CapturedObject ObjectMap::get_captured_object(const RestoredObject& restoredObject) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto itr = mCapturedObjects.find(restoredObject);
    return itr != mCapturedObjects.end() ? itr->second : RestoredObject{ };
}

size_t ObjectMap::size() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    assert(mRestoredObjects.size() == mCapturedObjects.size());
    return mRestoredObjects.size();
}

void ObjectMap::clear()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mRestoredObjects.clear();
    mCapturedObjects.clear();
}

} // namespace restore_point
} // namespace gvk
