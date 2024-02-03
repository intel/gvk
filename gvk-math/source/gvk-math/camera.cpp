
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

#include "gvk-math/camera.hpp"

namespace gvk {
namespace math {

glm::mat4 Camera::view() const
{
    return view(transform.translation + transform.forward());
}

glm::mat4 Camera::view(const glm::vec3& lookAt) const
{
    return glm::lookAt(transform.translation, lookAt, transform.up());
}

glm::mat4 Camera::projection(bool flipY) const
{
    glm::mat4 m { };
    switch (projectionMode) {
    case ProjectionMode::Perspective: {
        m = glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
    } break;
    case ProjectionMode::Orthographic: {
        auto w = fieldOfView * 0.5f;
        auto h = fieldOfView / aspectRatio * 0.5f;
        m = glm::ortho(-w, w, -h, h, nearPlane, farPlane);
    } break;
    default: {
    } break;
    }
    if (flipY) {
        m[1][1] *= -1;
    }
    return m;
}

Camera::Controller::~Controller()
{
}

const Camera* Camera::Controller::get_camera() const
{
    return mpCamera;
}

Camera* Camera::Controller::get_camera()
{
    return mpCamera;
}

void Camera::Controller::set_camera(Camera* pCamera)
{
    mpCamera = pCamera;
}

void FreeCameraController::update(const UpdateInfo& updateInfo)
{
    if (mpCamera) {
        if (moveEnabled) {
            glm::vec3 move { };
            move += updateInfo.moveUp ? mpCamera->transform.up() : glm::vec3 { };
            move += updateInfo.moveDown ? mpCamera->transform.down() : glm::vec3 { };
            move += updateInfo.moveLeft ? mpCamera->transform.left() : glm::vec3 { };
            move += updateInfo.moveRight ? mpCamera->transform.right() : glm::vec3 { };
            move += updateInfo.moveForward ? mpCamera->transform.forward() : glm::vec3 { };
            move += updateInfo.moveBackward ? mpCamera->transform.backward() : glm::vec3 { };
            move = (move.x || move.y || move.z) ? glm::normalize(move) : glm::vec3 { };
            mpCamera->transform.translation += move * moveSpeed * updateInfo.moveSpeedMultiplier * updateInfo.deltaTime;
        }
        if (lookEnabled) {
            auto look = updateInfo.lookDelta * lookSpeed;
            if (mVerticalLook + look.y > verticalLookMax) {
                look.y = verticalLookMax - mVerticalLook;
            } else if (mVerticalLook + look.y < verticalLookMin) {
                look.y = verticalLookMin - mVerticalLook;
            }
            mVerticalLook += look.y;
            auto horizontalRotation = glm::angleAxis(-look.x, glm::vec3{ 0, 1, 0 });
            auto verticalRotation = glm::angleAxis(look.y, glm::vec3{ 1, 0, 0 });
            mpCamera->transform.rotation = glm::normalize(horizontalRotation * mpCamera->transform.rotation * verticalRotation);
        }
        if (fieldOfViewEnabled) {
            mpCamera->fieldOfView -= updateInfo.fieldOfViewDelta * fieldOfViewSpeed;
            mpCamera->fieldOfView = glm::clamp(mpCamera->fieldOfView, fieldOfViewMin, fieldOfViewMax);
        }
    }
}

} // namespace math
} // namespace gvk
