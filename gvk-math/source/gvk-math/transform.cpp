
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

#include "gvk-math/transform.hpp"

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
