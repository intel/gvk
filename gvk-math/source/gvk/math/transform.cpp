
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

#include "gvk/math/transform.hpp"

#ifdef GVK_GLM_ENABLED

#include <cassert>

namespace gvk {
namespace math {

glm::mat4 Transform::world_from_local() const
{
    return glm::translate(translation) * glm::toMat4(rotation) * glm::scale(scale);
}

glm::mat4 Transform::local_from_world() const
{
    return glm::transpose(world_from_local());
}

glm::vec3 Transform::up() const
{
    return glm::normalize(rotation * glm::vec3{ 0, 1, 0 });
}

glm::vec3 Transform::down() const
{
    return glm::normalize(rotation * glm::vec3{ 0, -1, 0 });
}

glm::vec3 Transform::left() const
{
    return glm::normalize(rotation * glm::vec3{ 1, 0, 0 });
}

glm::vec3 Transform::right() const
{
    return glm::normalize(rotation * glm::vec3{ -1, 0, 0 });
}

glm::vec3 Transform::forward() const
{
    return glm::normalize(rotation * glm::vec3{ 0, 0, 1 });
}

glm::vec3 Transform::backward() const
{
    return glm::normalize(rotation * glm::vec3{ 0, 0, -1 });
}

} // namespace math
} // namespace gvk

#endif // GVK_GLM_ENABLED
