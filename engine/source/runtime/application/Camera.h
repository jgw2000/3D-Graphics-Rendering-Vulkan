#pragma once

#include "Macro.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace jgw
{
    class Camera final
    {
    public:
        enum CameraType { lookat, firstpersion };

        struct
        {
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
        } keys;

    public:
        CLASS_COPY_MOVE_DELETE(Camera)

        Camera() = default;

        void Update(float delta);

        void SetPerspective(float fov, float aspect, float znear, float zfar);
        void SetAspectRatio(float aspect);
        void SetPosition(glm::vec3 position);
        void SetForward(glm::vec3 forward);
        void SetMovementSpeed(float movementSpeed);
        void SetRotationSpeed(float rotationSpeed);

        inline bool IsMoving() const
        {
            return keys.left || keys.right || keys.up || keys.down;
        }

        inline float GetNearClip() const { return znear; }
        inline float GetFarClip() const { return zfar; }
        inline glm::mat4 GetView() const { return matrices.view; }
        inline glm::mat4 GetProj() const { return matrices.perspective; }

    private:
        void UpdateViewMatrix();

    private:
        CameraType type = firstpersion;

        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);

        float movementSpeed = 1.0f;
        float rotationSpeed = 1.0f;
        float fov;
        float znear;
        float zfar;

        struct
        {
            glm::mat4 view;
            glm::mat4 perspective;
        } matrices;
    };
}
