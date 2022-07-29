
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

namespace gvk {
namespace sys {

template <typename ObjectType>
class Updateable
{
public:
    Updateable() = default;
    virtual ~Updateable() = 0;

    inline void update()
    {
        previous = current;
        current = staged;
    }

    ObjectType previous { };
    ObjectType current { };
    ObjectType staged { };
};

template <typename ObjectType>
inline Updateable<ObjectType>::~Updateable()
{
}

} // namespace sys
} // namespace gvk
