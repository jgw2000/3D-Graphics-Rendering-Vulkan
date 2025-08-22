#pragma once

#include "Common.h"
#include "VulkanContext.h"

#include <deque>

namespace jgw
{
    class LinearGraph
    {
    public:
        CLASS_COPY_MOVE_DELETE(LinearGraph)

        explicit LinearGraph(const char* name, size_t maxGraphPoints = 256);

        void AddPoint(float value);
        void Render(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const glm::vec4 color = glm::vec4(1.0f));

    private:
        const char* name;
        const size_t maxPoints;
        std::deque<float> graph;
    };

    class LineCanvas2D
    {
    public:
        CLASS_COPY_MOVE_DELETE(LineCanvas2D)

        LineCanvas2D() = default;

        void Clear();
        void Line(const glm::vec2 p1, const glm::vec2 p2, const glm::vec4 c);
        void Render(const char* nameImGuiWindow);

    private:
        struct LineData
        {
            glm::vec2 p1, p2;
            glm::vec4 color;
        };
        std::vector<LineData> lines;
    };

    class LineCanvas3D
    {
    public:
        CLASS_COPY_MOVE_DELETE(LineCanvas3D)

        LineCanvas3D() = default;

        bool Initialize(VulkanContext& context);
        void Render(VulkanContext& context);

        void SetMatrix(glm::mat4 m);
        void Clear();
        void Line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec4 c);
        void Plane(const glm::vec3 o, const glm::vec3 v1, const glm::vec3 v2, int n1, int n2, float s1, float s2, const glm::vec4 color, const glm::vec4 outlineColor);
        void Box(const glm::mat4 m, const glm::vec3 size, const glm::vec4 c);
        void Frustum(const glm::mat4 camView, const glm::mat4 camProj, const glm::vec4 c);

    private:
        struct LineData
        {
            glm::vec4 pos;
            glm::vec4 color;
        };

        struct PushConstantData
        {
            glm::mat4 mvp;
            vk::DeviceAddress addr;
        } pushConstantData;

        std::vector<LineData> lines;
        std::unique_ptr<VulkanBuffer> lineBuffer;
        std::unique_ptr<VulkanPipeline> linePipeline;
    };
}
