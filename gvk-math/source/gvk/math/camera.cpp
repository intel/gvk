
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

#include "gvk/math/camera.hpp"

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

glm::mat4 Camera::projection(bool flip_y) const
{
    auto m = glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
    if (flip_y) {
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
            auto look = updateInfo.lookDelta * lookSpeed * updateInfo.deltaTime;
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
            mpCamera->fieldOfView -= updateInfo.fieldOfViewDelta * fieldOfViewSpeed * updateInfo.deltaTime;
            mpCamera->fieldOfView = glm::clamp(mpCamera->fieldOfView, fieldOfViewMin, fieldOfViewMax);
        }
    }
}

} // namespace math
} // namespace gvk
