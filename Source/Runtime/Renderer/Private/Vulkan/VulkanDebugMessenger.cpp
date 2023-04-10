#include "Vulkan/VulkanDebugMessenger.h"

#if OV_DEBUG

DEFINE_LOG_CATEGORY(LogVulkanInternal);

namespace VulkanDebugMessenger
{
	VkBool32 VulkanDebugLogInternal(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			OV_LOG(VeryVerbose, LogVulkanInternal, "{:s}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			OV_LOG(VeryVerbose, LogVulkanInternal, "{:s}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			OV_LOG(Warning, LogVulkanInternal, "{:s}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			OV_LOG(Error, LogVulkanInternal, "{:s}", pCallbackData->pMessage);
			break;
		}
		return (VK_FALSE);
	}

	void Initialize(const vk::Instance& instance)
	{
		g_Dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

		vk::DebugUtilsMessengerCreateInfoEXT createInfo(
			vk::DebugUtilsMessengerCreateFlagsEXT(),
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
			VulkanDebugLogInternal
		);

		g_Messenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, g_Dldi);
	}

	void CleanUp(const vk::Instance& instance)
	{
		instance.destroyDebugUtilsMessengerEXT(g_Messenger, nullptr, g_Dldi);
	}
}

#endif