
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

#include "gvk-math/defines.hpp"
#include "gvk-math/transform.hpp"

#include <algorithm>

namespace gvk {
namespace math {

struct Camera
{
    class Controller;

    enum class ProjectionMode
    {
        Perspective,
        Orthographic,
    };

    glm::mat4 view() const;
    glm::mat4 view(const glm::vec3& lookAt) const;
    glm::mat4 projection(bool flipY = true) const;

    template <typename T>
    inline void set_aspect_ratio(T width, T height)
    {
        aspectRatio = height ? (float)width / (float)height : 0;
    }

    Transform transform{ };
    float aspectRatio{ 16.0f / 9.0f };
    float fieldOfView{ 60 };
    float nearPlane{ 0.001f };
    float farPlane{ 100.0f };
    ProjectionMode projectionMode{ ProjectionMode::Perspective };
};

class Camera::Controller
{
public:
    virtual ~Controller() = 0;
    virtual const Camera* get_camera() const;
    virtual Camera* get_camera();
    virtual void set_camera(Camera* pCamera);

protected:
    Camera* mpCamera{ nullptr };
};

class FreeCameraController
    : public Camera::Controller
{
public:
    struct UpdateInfo
    {
        float deltaTime{ };
        bool moveUp{ };
        bool moveDown{ };
        bool moveLeft{ };
        bool moveRight{ };
        bool moveForward{ };
        bool moveBackward{ };
        float moveSpeedMultiplier{ 1 };
        glm::vec2 lookDelta{ };
        float fieldOfViewDelta{ };
    };

    void update(const UpdateInfo& updateInfo);

    bool moveEnabled{ true };
    float moveSpeed{ 4.2f };
    bool lookEnabled{ true };
    glm::vec2 lookSpeed{ 0.01f, 0.01f };
    float verticalLookMin{ -glm::radians(90.0f) };
    float verticalLookMax{ glm::radians(90.0f) };
    bool fieldOfViewEnabled{ true };
    float fieldOfViewSpeed{ 1 };
    float fieldOfViewMin{ 20 };
    float fieldOfViewMax{ 120 };

private:
    float mVerticalLook{ };
};

template <typename T>
inline T aspect_ratio(const T& width, const T& height)
{
    return height ? width / height : 0;
}

} // namespace math
} // namespace gvk
