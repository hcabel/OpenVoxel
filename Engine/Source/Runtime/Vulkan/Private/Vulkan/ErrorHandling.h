#pragma once

#include "Vulkan/Log.h"

/**
 * Checks the result of a Vulkan function call and logs an error if it failed.
 *
 * @param VulkanFunctionCall The result of the Vulkan function call.
 * @param Action A string describing the action that was attempted. (Will be concatenated with " failed with error code: " and the result code.)
 * @param Verbosity The verbosity of the log message.
 */
#define CHECK_VK_RESULT(VulkanFunctionCall, Action, Verbosity) \
	{ \
		vk::Result _VkResult = VulkanFunctionCall; \
		if (_VkResult != vk::Result::eSuccess) \
		{ \
			VULKAN_LOG(Verbosity, "{:s} failed with error code: {:s}", Action, vk::to_string(_VkResult)); \
		} \
	}
