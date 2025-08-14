#include "Camera.h"

namespace jgw
{
    void Camera::Update(double delta)
    {
        if (IsMoving())
        {
            glm::vec3 camFront;
            camFront.x = -cos(glm::radians(cameraRotation.x)) * sin(glm::radians(cameraRotation.y));
            camFront.y = sin(glm::radians(cameraRotation.x));
            camFront.z = cos(glm::radians(cameraRotation.x)) * cos(glm::radians(cameraRotation.y));
            
            float moveSpeed = delta * movementSpeed;
            if (keyState.acc) moveSpeed *= 3;

            if (keyState.up)
                cameraPosition -= camFront * moveSpeed;
            if (keyState.down)
                cameraPosition += camFront * moveSpeed;
            if (keyState.left)
                cameraPosition += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            if (keyState.right)
                cameraPosition -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
        }

        UpdateViewMatrix();
    }

    void Camera::SetPosition(glm::vec3 position)
    {
        this->cameraPosition = position;
        UpdateViewMatrix();
    }

    void Camera::SetRotation(glm::vec3 rotation)
    {
        this->cameraRotation = rotation;
        UpdateViewMatrix();
    }

    void Camera::SetAspectRatio(float aspect)
    {
        SetPerspective(fov, aspect, znear, zfar);
    }

    void Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->fov = fov;
        this->aspect = aspect;
        this->znear = znear;
        this->zfar = zfar;

        projMatrix = glm::perspective(fov, aspect, znear, zfar);
        projMatrix[1][1] *= -1.0f;
    }

    void Camera::Translate(glm::vec3 delta)
    {
        this->cameraPosition += delta;
        UpdateViewMatrix();
    }

    void Camera::Rotate(glm::vec3 delta)
    {
        this->cameraRotation += delta;
        UpdateViewMatrix();
    }

    void Camera::Scroll(float delta)
    {
        fov -= 0.1f * delta;
        fov = glm::clamp(fov, glm::radians(15.0f), glm::radians(90.0f));
        SetPerspective(fov, aspect, znear, zfar);
    }

    void Camera::UpdateViewMatrix()
    {
        glm::mat4 rotM = glm::mat4(1.0f);
        rotM = glm::rotate(rotM, glm::radians(cameraRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(cameraRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(cameraRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 transM = glm::translate(glm::mat4(1.0f), -cameraPosition);
        viewMatrix = rotM * transM;
    }
}
