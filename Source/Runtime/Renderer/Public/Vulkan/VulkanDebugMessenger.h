#pragma once

#if OV_DEBUG

#include "Logging/LoggingMacros.h"

#include <vulkan/vulkan.hpp>

DECLARE_LOG_CATEGORY(LogVulkanInternal);

namespace VulkanDebugMessenger
{
	static vk::DebugUtilsMessengerEXT g_Messenger;
	static vk::DispatchLoaderDynamic g_Dldi;

	VkBool32 VulkanDebugLogInternal(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void Initialize(const vk::Instance& instance);

	void CleanUp(const vk::Instance& instance);
}

#endif
