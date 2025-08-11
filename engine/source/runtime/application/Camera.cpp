#include "Camera.h"

namespace jgw
{
    void Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
    {
        projMatrix = glm::perspective(fov, aspect, znear, zfar);
        projMatrix[1][1] *= -1.0f;
    }

    FirstPersonCamera::FirstPersonCamera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up)
    {
        cameraPosition = pos;
        cameraOrientation = glm::lookAt(pos, target, up);
        cameraOrientation = glm::normalize(cameraOrientation);
        cameraUp = up;
    }

    void FirstPersonCamera::Update(double delta, const glm::vec2& newMousePos)
    {
        if (mouseStates.right)
        {
            const glm::vec2 mouseDelta = newMousePos - mousePosition;
            const glm::quat mouseDeltaQuat = glm::quat(glm::vec3(-mouseSpeed * mouseDelta.y, mouseSpeed * mouseDelta.x, 0.0f));
            cameraOrientation = mouseDeltaQuat * cameraOrientation;
            cameraOrientation = glm::normalize(cameraOrientation);
        }

        mousePosition = newMousePos;

        if (IsMoving())
        {
            const glm::mat4 v = glm::mat4_cast(cameraOrientation);
            const glm::vec3 right = glm::vec3(v[0][0], v[1][0], v[2][0]);   // +x
            const glm::vec3 up = glm::vec3(v[0][1], v[1][1], v[2][1]);      // +y
            const glm::vec3 forward = glm::cross(up, right);                // -z

            glm::vec3 dir = glm::vec3(0.0f);
            if (keyStates.up)
                dir += forward;
            if (keyStates.down)
                dir -= forward;
            if (keyStates.left)
                dir -= right;
            if (keyStates.right)
                dir += right;

            cameraPosition += glm::normalize(dir) * (float)delta * moveSpeed;
        }
    }

    glm::mat4 FirstPersonCamera::GetViewMatrix() const
    {
        const glm::mat4 t = glm::translate(glm::mat4(1.0f), -cameraPosition);
        const glm::mat4 r = glm::mat4_cast(cameraOrientation);
        return r * t;
    }
}
