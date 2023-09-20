#include "VulkanContext.h"
#include "Vulkan/Globals.h"
#include "Version.h"

#include <vulkan/vulkan.hpp>
#include <vector>
#include <GLFW/glfw3.h>

namespace VulkanDebug
{
	static VkBool32 VulkanDebugLogInternal(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			OV_LOG(LogVulkanInternal, Verbose, "{:s}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			OV_LOG(LogVulkanInternal, Verbose, "{:s}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			OV_LOG(LogVulkanInternal, Warning, "{:s}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			OV_LOG(LogVulkanInternal, Error, "{:s}", pCallbackData->pMessage);
			break;
		}
		return (VK_FALSE);
	}
}

VulkanContext::~VulkanContext()
{
	if (m_Device || m_Instance)
	{
		VULKAN_LOG(Warning, "Vulkan context was not explicitly destroyed!");
		Destroy();
	}
}

bool VulkanContext::CreateInstance(uint32_t major, uint32_t minor, uint32_t patch)
{
	if (m_Instance)
	{
		VULKAN_LOG(Warning, "Vulkan instance already created!");
		return (true);
	}

	if (m_Instance.Create(VulkanInstance::SelectVulkanApiVersion(major, minor, patch)) == false)
	{
		VULKAN_LOG(Error, "Failed to create Vulkan instance!");
		return (false);
	}

	m_Dispatcher = vk::DispatchLoaderDynamic(m_Instance, vkGetInstanceProcAddr);

#ifdef OV_DEBUG
	// Create the debug messenger
	vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo(
		vk::DebugUtilsMessengerCreateFlagsEXT(),
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
		&VulkanDebug::VulkanDebugLogInternal
	);

	m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(debugMessengerCreateInfo, nullptr, m_Dispatcher);
#endif // OV_DEBUG

	return (true);
}

bool VulkanContext::CreateDevice(vk::SurfaceKHR& surface, vk::PhysicalDeviceFeatures features)
{
	if (m_Device)
	{
		VULKAN_LOG(Warning, "Vulkan device already created!");
		return (true);
	}

	if (m_Device.Create(surface, VK_NULL_HANDLE, features) == false)
	{
		VULKAN_LOG(Error, "Failed to create Vulkan device!");
		return (false);
	}
	return (true);
}

void VulkanContext::Destroy()
{
	if (m_Device)
		m_Device.Destroy();

	if (m_Instance)
	{
#ifdef OV_DEBUG
		m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, m_Dispatcher);
#endif // OV_DEBUG

		m_Instance.Destroy();
	}
}
