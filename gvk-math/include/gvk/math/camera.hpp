
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
#include "gvk/math/transform.hpp"

#include <algorithm>

namespace gvk {
namespace math {

struct Camera
{
    class Controller;

    glm::mat4 view() const;
    glm::mat4 view(const glm::vec3& lookAt) const;
    glm::mat4 projection(bool flip_y = true) const;

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
    glm::vec2 lookSpeed{ 1, 1 };
    float verticalLookMin{ -glm::radians(90.0f) };
    float verticalLookMax{ glm::radians(90.0f) };
    bool fieldOfViewEnabled{ true };
    float fieldOfViewSpeed{ 64 };
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
