
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

#include "gvk/math/defines.hpp"

#ifdef GVK_GLM_ENABLED

namespace gvk {
namespace math {

struct Transform
{
    glm::mat4 world_from_local() const;
    glm::mat4 local_from_world() const;
    glm::vec3 up() const;
    glm::vec3 down() const;
    glm::vec3 left() const;
    glm::vec3 right() const;
    glm::vec3 forward() const;
    glm::vec3 backward() const;

    glm::vec3 translation { };
    glm::quat rotation { 1, 0, 0, 0 };
    glm::vec3 scale { 1, 1, 1 };
};

} // namespace math
} // namespace gvk

#endif // GVK_GLM_ENABLED
