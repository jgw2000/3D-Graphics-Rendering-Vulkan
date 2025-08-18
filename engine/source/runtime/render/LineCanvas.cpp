#include "LineCanvas.h"

#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_vulkan.h"

#include "implot.h"

namespace jgw
{
    LinearGraph::LinearGraph(const char* name, size_t maxGraphPoints)
        : name(name)
        , maxPoints(maxGraphPoints)
    { }

    void LinearGraph::AddPoint(float value)
    {
        graph.push_back(value);
        if (graph.size() > maxPoints)
            graph.erase(graph.begin());
    }

    void LinearGraph::Render(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const glm::vec4 color)
    {
        float minVal = std::numeric_limits<float>::max();
        float maxVal = std::numeric_limits<float>::min();

        for (float f : graph)
        {
            if (f < minVal) minVal = f;
            if (f > maxVal) maxVal = f;
        }

        const float range = maxVal - minVal;
        float valX = 0.0;

        std::vector<float> dataX;
        std::vector<float> dataY;
        dataX.reserve(graph.size());
        dataY.reserve(graph.size());

        for (float f : graph)
        {
            const float valY = (f - minVal) / range;
            valX += 1.0f / maxPoints;
            dataX.push_back(valX);
            dataY.push_back(valY);
        }

        ImGui::SetNextWindowPos(ImVec2(x, y));
        ImGui::SetNextWindowSize(ImVec2(width, height));
        ImGui::Begin(
            name, nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoInputs
        );

        if (ImPlot::BeginPlot(name, ImVec2(width, height), ImPlotFlags_CanvasOnly | ImPlotFlags_NoFrame | ImPlotFlags_NoInputs)) {
            ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color.r, color.g, color.b, color.a));
            ImPlot::PushStyleColor(ImPlotCol_PlotBg, ImVec4(0, 0, 0, 0));
            ImPlot::PlotLine("#line", dataX.data(), dataY.data(), (int)graph.size(), ImPlotLineFlags_None);
            ImPlot::PopStyleColor(2);
            ImPlot::EndPlot();
        }

        ImGui::End();
    }

    void LineCanvas2D::Clear()
    {
        lines.clear();
    }

    void LineCanvas2D::Line(const glm::vec2 p1, const glm::vec2 p2, const glm::vec4 c)
    {
        lines.push_back({ .p1 = p1, .p2 = p2, .color = c });
    }

    void LineCanvas2D::Render(const char* nameImGuiWindow)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
        ImGui::Begin(
            nameImGuiWindow, nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoInputs
        );

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        for (const LineData& l : lines)
        {
            drawList->AddLine(ImVec2(l.p1.x, l.p1.y), ImVec2(l.p2.x, l.p2.y), ImColor(l.color.r, l.color.g, l.color.b, l.color.a));
        }

        ImGui::End();
    }

    bool LineCanvas3D::Initialize(VulkanContext& context)
    {
        std::vector<vk::PushConstantRange> pushConstantRanges = {
            { .stageFlags = vk::ShaderStageFlagBits::eVertex, .offset = 0, .size = sizeof(PushConstantData) }
        };

        LinePipelineBuilder pd;
        pd.AddShader(vk::ShaderStageFlagBits::eVertex, "../engine/shaders/canvas.vert.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eFragment, "../engine/shaders/canvas.frag.spv");
        pd.SetPushConstantRanges(pushConstantRanges);

        linePipeline = context.CreateGraphicsPipeline(pd);
        if (linePipeline == nullptr)
        {
            return false;
        }

        return true;
    }

    void LineCanvas3D::Render(VulkanContext& context)
    {
        if (lines.empty())
            return;

        vk::DeviceSize requiredSize = lines.size() * sizeof(LineData);
        if (!lineBuffer || lineBuffer->TotalSize() < requiredSize)
        {
            lineBuffer = context.CreateBuffer(
                requiredSize,
                vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
                vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
            );

            vk::BufferDeviceAddressInfo addrInfo{
                .buffer = lineBuffer->Handle()
            };
            pushConstantData.addr = context.GetDevice().getBufferAddress(addrInfo);

            lineBuffer->Map();
        }

        lineBuffer->CopyFromHost(lines.data(), requiredSize);

        auto commandBuffer = context.GetCommandBuffer();
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, linePipeline->Handle());
        commandBuffer.pushConstants(linePipeline->Layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstantData), &pushConstantData);
        commandBuffer.draw(lines.size(), 1, 0, 0);
    }

    void LineCanvas3D::SetMatrix(glm::mat4 m)
    {
        pushConstantData.mvp = m;
    }

    void LineCanvas3D::Clear()
    {
        lines.clear();
    }

    void LineCanvas3D::Line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec4 c)
    {
        lines.push_back({ .pos = glm::vec4(p1, 1.0f), .color = c });
        lines.push_back({ .pos = glm::vec4(p2, 1.0f), .color = c });
    }

    void LineCanvas3D::Plane(const glm::vec3 o, const glm::vec3 v1, const glm::vec3 v2, int n1, int n2, float s1, float s2, const glm::vec4 color, const glm::vec4 outlineColor)
    {
        Line(o - s1 / 2.0f * v1 - s2 / 2.0f * v2, o - s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
        Line(o + s1 / 2.0f * v1 - s2 / 2.0f * v2, o + s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
        Line(o - s1 / 2.0f * v1 + s2 / 2.0f * v2, o + s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
        Line(o - s1 / 2.0f * v1 - s2 / 2.0f * v2, o + s1 / 2.0f * v1 - s2 / 2.0f * v2, outlineColor);

        for (int i = 1; i < n1; ++i)
        {
            float t = ((float)i - (float)n1 / 2.0f) * s1 / (float)n1;
            const glm::vec3 o1 = o + t * v1;
            Line(o1 - s2 / 2.0f * v2, o1 + s2 / 2.0f * v2, color);
        }

        for (int i = 1; i < n2; ++i)
        {
            float t = ((float)i - (float)n2 / 2.0f) * s2 / (float)n2;
            const glm::vec3 o2 = o + t * v2;
            Line(o2 - s1 / 2.0f * v1, o2 + s1 / 2.0f * v1, color);
        }
    }

    void LineCanvas3D::Box(const glm::mat4 m, const glm::vec3 size, const glm::vec4 c)
    {
        glm::vec3 pts[] = {
            glm::vec3(+size.x, +size.y, +size.z),
            glm::vec3(+size.x, +size.y, -size.z),
            glm::vec3(+size.x, -size.y, +size.z),
            glm::vec3(+size.x, -size.y, -size.z),
            glm::vec3(-size.x, +size.y, +size.z),
            glm::vec3(-size.x, +size.y, -size.z),
            glm::vec3(-size.x, -size.y, +size.z),
            glm::vec3(-size.x, -size.y, -size.z),
        };

        for (auto& p : pts) p = glm::vec3(m * glm::vec4(p, 1.f));

        Line(pts[0], pts[1], c);
        Line(pts[2], pts[3], c);
        Line(pts[4], pts[5], c);
        Line(pts[6], pts[7], c);

        Line(pts[0], pts[2], c);
        Line(pts[1], pts[3], c);
        Line(pts[4], pts[6], c);
        Line(pts[5], pts[7], c);

        Line(pts[0], pts[4], c);
        Line(pts[1], pts[5], c);
        Line(pts[2], pts[6], c);
        Line(pts[3], pts[7], c);
    }

    void LineCanvas3D::Frustum(const glm::mat4 camView, const glm::mat4 camProj, const glm::vec4 c)
    {
        const glm::vec3 corners[] = {
            glm::vec3(-1, -1, -1),
            glm::vec3(+1, -1, -1),
            glm::vec3(+1, +1, -1),
            glm::vec3(-1, +1, -1),
            glm::vec3(-1, -1, +1),
            glm::vec3(+1, -1, +1),
            glm::vec3(+1, +1, +1),
            glm::vec3(-1, +1, +1),
        };

        glm::vec3 pp[8];

        for (int i = 0; i < 8; ++i)
        {
            glm::vec4 q = glm::inverse(camView) * glm::inverse(camProj) * glm::vec4(corners[i], 1.0f);
            pp[i] = glm::vec3(q.x / q.w, q.y / q.w, q.z / q.w);
        }

        Line(pp[0], pp[4], c);
        Line(pp[1], pp[5], c);
        Line(pp[2], pp[6], c);
        Line(pp[3], pp[7], c);
        // near
        Line(pp[0], pp[1], c);
        Line(pp[1], pp[2], c);
        Line(pp[2], pp[3], c);
        Line(pp[3], pp[0], c);
        // x
        Line(pp[0], pp[2], c);
        Line(pp[1], pp[3], c);
        // far
        Line(pp[4], pp[5], c);
        Line(pp[5], pp[6], c);
        Line(pp[6], pp[7], c);
        Line(pp[7], pp[4], c);
        // x
        Line(pp[4], pp[6], c);
        Line(pp[5], pp[7], c);

        const glm::vec4 gridColor = c * 0.7f;
        const int gridLines = 100;

        // bottom
        {
            glm::vec3 p1 = pp[0];
            glm::vec3 p2 = pp[1];
            const glm::vec3 s1 = (pp[4] - pp[0]) / float(gridLines);
            const glm::vec3 s2 = (pp[5] - pp[1]) / float(gridLines);
            for (int i = 0; i != gridLines; i++, p1 += s1, p2 += s2)
                Line(p1, p2, gridColor);
        }
        // top
        {
            glm::vec3 p1 = pp[2];
            glm::vec3 p2 = pp[3];
            const glm::vec3 s1 = (pp[6] - pp[2]) / float(gridLines);
            const glm::vec3 s2 = (pp[7] - pp[3]) / float(gridLines);
            for (int i = 0; i != gridLines; i++, p1 += s1, p2 += s2)
                Line(p1, p2, gridColor);
        }
        // left
        {
            glm::vec3 p1 = pp[0];
            glm::vec3 p2 = pp[3];
            const glm::vec3 s1 = (pp[4] - pp[0]) / float(gridLines);
            const glm::vec3 s2 = (pp[7] - pp[3]) / float(gridLines);
            for (int i = 0; i != gridLines; i++, p1 += s1, p2 += s2)
                Line(p1, p2, gridColor);
        }
        // right
        {
            glm::vec3 p1 = pp[1];
            glm::vec3 p2 = pp[2];
            const glm::vec3 s1 = (pp[5] - pp[1]) / float(gridLines);
            const glm::vec3 s2 = (pp[6] - pp[2]) / float(gridLines);
            for (int i = 0; i != gridLines; i++, p1 += s1, p2 += s2)
                Line(p1, p2, gridColor);
        }
    }

    vk::PipelineInputAssemblyStateCreateInfo LinePipelineBuilder::BuildInputAssemblyState()
    {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{
            .topology = vk::PrimitiveTopology::eLineList,
            .primitiveRestartEnable = vk::False
        };

        return inputAssemblyStateCI;
    }

    vk::PipelineRasterizationStateCreateInfo LinePipelineBuilder::BuildRasterizationState()
    {
        vk::PipelineRasterizationStateCreateInfo rasterizationStateCI{
            .depthClampEnable = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = vk::PolygonMode::eLine,
            .cullMode = vk::CullModeFlagBits::eNone,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = vk::False,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };

        return rasterizationStateCI;
    }
}
