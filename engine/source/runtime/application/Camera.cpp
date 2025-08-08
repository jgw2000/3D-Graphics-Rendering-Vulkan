#include "Camera.h"

namespace jgw
{
    void Camera::Update(float delta)
    {
        if (type == firstpersion && IsMoving())
        {
            float movement = delta * movementSpeed;

            if (keys.up)
                position += forward * movement;
            if (keys.down)
                position -= forward * movement;
            if (keys.left)
                position -= glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))) * movement;
            if (keys.right)
                position += glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))) * movement;
        }

        UpdateViewMatrix();
    }

    void Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->fov = fov;
        this->znear = znear;
        this->zfar = zfar;

        matrices.perspective = glm::perspective(fov, aspect, znear, zfar);
        matrices.perspective[1][1] *= -1.0f;
    }

    void Camera::SetAspectRatio(float aspect)
    {
        matrices.perspective = glm::perspective(fov, aspect, znear, zfar);
        matrices.perspective[1][1] *= -1.0f;
    }

    void Camera::SetPosition(glm::vec3 position)
    {
        this->position = position;
        UpdateViewMatrix();
    }

    void Camera::SetForward(glm::vec3 forward)
    {
        this->forward = forward;
        UpdateViewMatrix();
    }

    void Camera::SetMovementSpeed(float movementSpeed)
    {
        this->movementSpeed = movementSpeed;
    }

    void Camera::SetRotationSpeed(float rotationSpeed)
    {
        this->rotationSpeed = rotationSpeed;
    }

    void Camera::UpdateViewMatrix()
    {
        matrices.view = glm::lookAt(position, position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}
