
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

#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace gvk {

/**
Tag dispatch for functions that nullify a reference
*/
struct nullref_t { };
extern nullref_t nullref;

/**
Tag dispatch for functions that allocate a new managed object
*/
struct newref_t { };
extern newref_t newref;

/**
Provides a mechanism for creating runtime unique ids for a given object type
@param <ObjType> The type of object to create a unique id for
*/
template <typename ObjType>
class RuntimeUID final
{
public:
    /**
    Constructs an instance of RuntimeUID with a specified value
    @param [in] value (optional = 0) This RuntimeUID object's value
    */
    inline RuntimeUID(uint64_t value = 0)
        : mValue { value }
    {
    }

    /**
    Constructs an instance of RuntimeUID with a unique value
    @param [in] (newref_t) Tag dispatch indicating that this function creates a new RuntimeUID<> value
    */
    inline RuntimeUID(newref_t)
    {
        static std::atomic_uint64_t sValue;
        mValue = ++sValue;
    }

    /**
    Converts this RuntimeUID to its underlying uint64_t value
    @return This RuntimeUID to its underlying uint64_t value
    */
    inline operator uint64_t() const
    {
        return mValue;
    }

private:
    uint64_t mValue { };
};

/**
Provides high level control over a ref counted, enumerable, managed object
@param <ObjType> The type of object to manage
@param <IdType> (optional = RuntimeUID<ObjType>) The type of id used to lookup the managed object
*/
template <typename ObjType, typename IdType = RuntimeUID<ObjType>>
class Reference final
{
public:
    /**
    Constructs an instance of Reference<>
    @param [in] (nullref_t = nullref) Tag dispatch indicating that this function clears an existing reference to a managed object
    */
    inline Reference(nullref_t = nullref)
    {
        reset(nullref);
    }

    /**
    @param [in] (newref_t) Tag dispatch indicating that this function allocates a new managed object
        @note This constructor is only available if IdType exposes a constructor that accepts only a single newref_t parameter
    */
    inline explicit Reference(newref_t)
    {
        reset(newref);
    }

    /**
    Constructs an instance of Reference<>
    @param [in] (newref_t) Tag dispatch indicating that this function allocates a new managed object
    @param [in] id The unique id of the managed object
    */
    inline explicit Reference(newref_t, const IdType& id)
    {
        reset(newref, id);
    }

    /**
    Constructs an instance of Reference<>
    @param [in] (nullref_t) Tag dispatch indicating that this function clears an existing reference to a managed object
    */
    inline Reference<ObjType, IdType>& operator=(nullref_t)
    {
        reset(nullref);
        return *this;
    }

    /**
    Copies an instance of Reference<>
    @param [in] other The Reference<> to copy
    */
    Reference(const Reference<ObjType, IdType>& other) = default;

    /**
    Copies an instance of Reference<>
    @param [in] other The Reference<> to copy
    @return A reference to this Reference<>
    */
    Reference<ObjType, IdType>& operator=(const Reference<ObjType, IdType>& other) = default;

    /**
    Moves an instance of Reference<>
    @param [in] other The Reference<> to move from
    */
    Reference(Reference<ObjType, IdType>&& other) = default;

    /**
    Moves an instance of Reference<>
    @param [in] other The Reference<> to move from
    @return A reference to this Reference<>
    */
    Reference<ObjType, IdType>& operator=(Reference<ObjType, IdType>&& other) = default;

    /**
    Destroys this instance of Reference<>
    */
    inline ~Reference()
    {
        reset(nullref);
    }

    /**
    Resets this instance of Reference<>
    @param [in] (newref_t) Tag dispatch indicating that this function allocates a new managed object
        @note This method is only available if IdType exposes a constructor that accepts only a single newref_t parameter
    */
    inline void reset(newref_t)
    {
        reset(nullref);
        mId = IdType(newref);
        mspLifetimeMonitor = std::make_shared<LifetimeMonitor>(mId);
        Enumerator::get_instance().insert(mspLifetimeMonitor);
    }

    /**
    Resets this instance of Reference<>
    @param [in] (newref_t) Tag dispatch indicating that this function allocates a new managed object
    @param [in] id The unique id of the managed object
    */
    inline void reset(newref_t, const IdType& id)
    {
        reset(nullref);
        mId = id;
        mspLifetimeMonitor = std::make_shared<LifetimeMonitor>(mId);
        Enumerator::get_instance().insert(mspLifetimeMonitor);
    }

    /**
    Resets this instance of Reference<>
    @param [in] (nullref_t = nullref) Tag dispatch indicating that this function clears an existing reference to a managed object
    */
    inline void reset(nullref_t = nullref)
    {
        mId = { };
        mspLifetimeMonitor.reset();
    }

    /**
    Dereferences this Reference<> object's referenced object
    @return A reference to this Reference<> object's referenced object
    */
    inline const ObjType* operator->() const
    {
        assert(mspLifetimeMonitor && "Attempting to dereference nullref Reference<>");
        return &mspLifetimeMonitor->get_obj();
    }

    /**
    Dereferences this Reference<> object's referenced object
    @return A reference to this Reference<> object's referenced object
    */
    inline ObjType* operator->()
    {
        assert(mspLifetimeMonitor && "Attempting to dereference nullref Reference<>");
        return &mspLifetimeMonitor->get_obj();
    }

    /**
    Dereferences this Reference<> object's referenced object
    @return A reference to this Reference<> object's referenced object
    */
    inline const ObjType& operator*() const
    {
        assert(mspLifetimeMonitor && "Attempting to dereference nullref Reference<>");
        return mspLifetimeMonitor->get_obj();
    }

    /**
    Dereferences this Reference<> object's referenced object
    @return A reference to this Reference<> object's referenced object
    */
    inline ObjType& operator*()
    {
        assert(mspLifetimeMonitor && "Attempting to dereference nullref Reference<>");
        return mspLifetimeMonitor->get_obj();
    }

    /**
    Gets a value indicating whether or not this Reference<> references a valid object
    @return A value indicating whether or not this Reference<> references a valid object
    */
    operator bool() const
    {
        return mspLifetimeMonitor != nullptr;
    }

    /**
    Gets this Reference<> object's id
    @return This Reference<> object's id
    */
    inline const IdType& get_id() const
    {
        return mId;
    }

    /**
    Gets this Reference<> object's managed object
    @return This Reference<> object's managed object
    */
    inline const ObjType& get_obj() const
    {
        assert(mspLifetimeMonitor && "Attempting to dereference nullref Reference<>");
        return mspLifetimeMonitor->get_obj();
    }

    /**
    Gets this Reference<> object's managed object
    @return This Reference<> object's managed object
    */
    inline ObjType& get_obj()
    {
        assert(mspLifetimeMonitor && "Attempting to dereference nullref Reference<>");
        return mspLifetimeMonitor->get_obj();
    }

    /**
    Gets this Reference<> object's reference count
    @return This Reference<> object's reference count
    */
    inline uint64_t get_ref_count() const
    {
        return (uint64_t)mspLifetimeMonitor.use_count();
    }

    /**
    Gets the Reference<> associated with the given id
    @param [in] id The id of the Reference<> to get
    @return The Reference<> associated with the given id
    */
    inline static Reference get(const IdType& id)
    {
        return Enumerator::get_instance().get(id);
    }

    /**
    Enumerates all outstanding Reference<> objects with the same ObjectType and IdType
        @note Reference<> functions that accept a newref_t parameter must not be called while this function is executing
        @note All outstanding Reference<> objects must maintain a positive ref count while this function is executing
    */
    template <typename ProcessReferenceFunctionType>
    inline static void enumerate(ProcessReferenceFunctionType processReference)
    {
        return Enumerator::get_instance().enumerate(processReference);
    }

private:
    class LifetimeMonitor final
    {
    public:
        inline LifetimeMonitor(const IdType& id)
            : mId { id }
        {
        }

        inline ~LifetimeMonitor()
        {
            Enumerator::get_instance().erase(mId);
        }

        inline const IdType& get_id() const
        {
            return mId;
        }

        inline const ObjType& get_obj() const
        {
            return mObj;
        }

        inline ObjType& get_obj()
        {
            return mObj;
        }

    private:
        IdType mId { 0 };
        ObjType mObj;
    };

    class Enumerator final
    {
    public:
        inline void insert(std::shared_ptr<LifetimeMonitor> spReference)
        {
            assert(spReference && "References to deleted objects must be cleared upon destruction; gvk maintenance required");
            std::lock_guard<std::mutex> lock(mMutex);
            auto success = mWeakReferences.insert({ spReference->get_id(), spReference }).second;
            (void)success;
            assert(success && "Failed to insert std::shared_ptr<LifetimeMonitor>; was a Reference<> initialized with a duplicate id?");
        }

        inline void erase(const IdType& id)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            auto success = mWeakReferences.erase(id);
            (void)success;
            assert(success && "References to deleted objects must be cleared upon destruction; gvk maintenance required");
        }

        inline Reference get(const IdType& id)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            auto itr = mWeakReferences.find(id);
            Reference reference;
            if (itr != mWeakReferences.end()) {
                reference.mId = itr->first;
                reference.mspLifetimeMonitor = itr->second.lock();
                assert(reference && "References to deleted objects must be cleared upon destruction; gvk maintenance required");
            }
            return reference;
        }

        template <typename ProcessReferenceFunctionType>
        inline void enumerate(ProcessReferenceFunctionType processReference)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            for (const auto& itr : mWeakReferences) {
                Reference reference;
                reference.mId = itr.first;
                reference.mspLifetimeMonitor = itr.second.lock();
                processReference(reference);
            }
        }

        static Enumerator& get_instance()
        {
            static Enumerator sEnumerator;
            return sEnumerator;
        }

    private:
        Enumerator() = default;
        std::mutex mMutex;
        std::unordered_map<IdType, std::weak_ptr<LifetimeMonitor>> mWeakReferences;
    };

    inline friend bool operator==(const Reference& lhs, const Reference& rhs)
    {
        return lhs.mspLifetimeMonitor == rhs.mspLifetimeMonitor;
    }

    inline friend bool operator!=(const Reference& lhs, const Reference& rhs)
    {
        return lhs.mspLifetimeMonitor != rhs.mspLifetimeMonitor;
    }

    inline friend bool operator<(const Reference& lhs, const Reference& rhs)
    {
        return lhs.mspLifetimeMonitor < rhs.mspLifetimeMonitor;
    }

    inline friend bool operator>(const Reference& lhs, const Reference& rhs)
    {
        return lhs.mspLifetimeMonitor > rhs.mspLifetimeMonitor;
    }

    inline friend bool operator<=(const Reference& lhs, const Reference& rhs)
    {
        return lhs.mspLifetimeMonitor <= rhs.mspLifetimeMonitor;
    }

    inline friend bool operator>=(const Reference& lhs, const Reference& rhs)
    {
        return lhs.mspLifetimeMonitor >= rhs.mspLifetimeMonitor;
    }

    std::shared_ptr<LifetimeMonitor> mspLifetimeMonitor;
    IdType mId { };
};

} // namespace gvk

namespace std {

template <typename ObjType>
struct hash<gvk::RuntimeUID<ObjType>>
{
    inline size_t operator()(const gvk::RuntimeUID<ObjType>& runtimeUid) const
    {
        return std::hash<uint64_t> { }(runtimeUid);
    }
};

} // namespace std
