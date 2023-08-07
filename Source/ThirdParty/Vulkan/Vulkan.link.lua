local VulkanSDK = os.getenv("VULKAN_SDK")
VulkanSDK = string.gsub(VulkanSDK, '\\', '/')

function VulkanThirdParty(config)
	local Vulkan = {}

	-- This will be used to override the root directory of the library.
	-- so every other path will be relative to this one.
	Vulkan.RootDirectoryOverride = VulkanSDK

	-- We dont use vulkan sources, so we link it to whatever the user has installed.
	-- (not affected by the RootDirectoryOverride)
	Vulkan.LinkName = VulkanSDK .. "/Lib/vulkan-1.lib"

	Vulkan.IncludeDirs = {
		"/Include"
	}

	return Vulkan
end

return VulkanThirdParty;