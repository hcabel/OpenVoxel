#pragma once

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

struct ImGui_ImplVulkan_Data_COPY
{
    ImGui_ImplVulkan_InitInfo   VulkanInitInfo;
    VkRenderPass                RenderPass;
    VkDeviceSize                BufferMemoryAlignment;
    VkPipelineCreateFlags       PipelineCreateFlags;
    VkDescriptorSetLayout       DescriptorSetLayout;
    VkPipelineLayout            PipelineLayout;
    VkPipeline                  Pipeline;
    uint32_t                    Subpass;
    VkShaderModule              ShaderModuleVert;
    VkShaderModule              ShaderModuleFrag;

    // Font data
    VkSampler                   FontSampler;
    VkDeviceMemory              FontMemory;
    VkImage                     FontImage;
    VkImageView                 FontView;
    VkDescriptorSet             FontDescriptorSet;
    VkDeviceMemory              UploadBufferMemory;
    VkBuffer                    UploadBuffer;
};
