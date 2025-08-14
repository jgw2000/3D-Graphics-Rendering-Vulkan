#pragma once

#include "Macro.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace jgw
{
    // Right-Handed
    class Camera
    {
    public:
        void Update(double delta);

        void SetPosition(glm::vec3 position);
        void SetRotation(glm::vec3 rotation);
        void SetAspectRatio(float aspect);
        void SetPerspective(float fov, float aspect, float znear, float zfar);
        
        void Translate(glm::vec3 delta);
        void Rotate(glm::vec3 delta);
        void Scroll(float delta);

        inline glm::vec3 GetPosition() const { return cameraPosition; }
        inline glm::mat4 GetViewMatrix() const { return viewMatrix; }
        inline glm::mat4 GetProjMatrix() const { return projMatrix; }
        inline float RotationSpeed() const { return rotationSpeed; }

        struct
        {
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
            bool acc = false;
        } keyState;

    protected:
        void UpdateViewMatrix();

        inline bool IsMoving() const
        {
            return keyState.left || keyState.right || keyState.up || keyState.down;
        }

        glm::vec3 cameraPosition = glm::vec3();
        glm::vec3 cameraRotation = glm::vec3();

        float movementSpeed = 1.0f;
        float rotationSpeed = 90.0f;

        glm::mat4 viewMatrix = glm::mat4(1.0f);
        glm::mat4 projMatrix = glm::mat4(1.0f);

        float fov;
        float aspect;
        float znear;
        float zfar;
    };
}
