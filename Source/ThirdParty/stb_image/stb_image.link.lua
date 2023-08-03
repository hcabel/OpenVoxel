
function stb_imageLibrary(config)
	local stb_image = {}

	-- stb_image is a header-only library, so we don't need to link to anything.
	stb_image.LinkName = nil

	-- We dont need to add any extra include directories, the root directory is the only one we need. (and it's added by default)
	stb_image.IncludeDirs = {}

	return stb_image
end

return stb_imageLibrary