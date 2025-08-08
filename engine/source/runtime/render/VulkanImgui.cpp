#include "VulkanImgui.h"

#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_vulkan.h"

#include <Volk/volk.h>

namespace jgw
{
    VulkanImgui::VulkanImgui()
    {
        volkInitialize();
    }

    VulkanImgui::~VulkanImgui()
    {
        if (initialized)
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        vkDestroyDescriptorPool(volkGetLoadedDevice(), descriptorPool, nullptr);
        
        volkFinalize();
    }

    bool VulkanImgui::Initialize(
        void* window,
        VkInstance            instance,
        VkPhysicalDevice      physicalDevice,
        VkDevice              device,
        VkQueue               queue,
        uint32_t              queueFamily,
        uint32_t              imageCount,
        VkFormat              colorFormat,
        VkFormat              depthFormat,
        uint32_t              apiVersion,
        VkSampleCountFlagBits msaaSamples
    )
    {
        volkLoadInstance(instance);
        volkLoadDevice(device);

        const int numElements = 500;
        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, numElements},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numElements},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, numElements},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, numElements},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, numElements},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, numElements},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numElements},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numElements},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, numElements},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, numElements},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, numElements},
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = numElements * (uint32_t)IM_ARRAYSIZE(poolSizes);
        poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
        poolInfo.pPoolSizes = poolSizes;
        vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(window), true);

        VkFormat colorFormats[] = { colorFormat };
        ImGui_ImplVulkan_InitInfo init_info{
            .ApiVersion = apiVersion,
            .Instance = instance,
            .PhysicalDevice = physicalDevice,
            .Device = device,
            .QueueFamily = queueFamily,
            .Queue = queue,
            .DescriptorPool = descriptorPool,
            .MinImageCount = imageCount,
            .ImageCount = imageCount,
            .MSAASamples = msaaSamples,
            .UseDynamicRendering = true,
            .PipelineRenderingCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = colorFormats,
                .depthAttachmentFormat = depthFormat
            }
        };

        bool result = ImGui_ImplVulkan_Init(&init_info);
        if (result)
            initialized = true;

        return result;
    }

    void VulkanImgui::BeginFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void VulkanImgui::EndFrame()
    {
        ImGui::EndFrame();
    }

    void VulkanImgui::Render(VkCommandBuffer commandBuffer)
    {
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
    }
}
