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
        virtual void Update(double delta, const glm::vec2& newMousePos) = 0;
        virtual glm::mat4 GetViewMatrix() const = 0;

        void SetPerspective(float fov, float aspect, float znear, float zfar);

        inline glm::vec3 GetPosition() const { return cameraPosition; }
        inline glm::mat4 GetProjMatrix() const { return projMatrix; }

        struct
        {
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
        } keyStates;

        struct
        {
            bool left = false;
            bool right = false;
        } mouseStates;

    protected:
        glm::vec3 cameraPosition = glm::vec3(0.0f);
        glm::quat cameraOrientation = glm::quat(glm::vec3(0.0f));
        glm::vec3 cameraUp = glm::vec3(0.0f);

        glm::vec2 mousePosition = glm::vec2(0.0f);
        float moveSpeed = 1.0f;
        float mouseSpeed = 4.0f;

        glm::mat4 projMatrix = glm::mat4(1.0f);
    };

    class FirstPersonCamera final : public Camera
    {
    public:
        CLASS_COPY_MOVE_DELETE(FirstPersonCamera)

        FirstPersonCamera() = default;
        FirstPersonCamera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up);

        void Update(double delta, const glm::vec2& mousePos) override;
        glm::mat4 GetViewMatrix() const override;

    private:
        inline bool IsMoving() const
        {
            return keyStates.left || keyStates.right || keyStates.up || keyStates.down;
        }
    };
}
