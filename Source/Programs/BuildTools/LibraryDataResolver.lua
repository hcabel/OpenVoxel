include "GlobalValues.lua"
include "FileUtils.lua"

function GetLibraryData(libraryName)
	local libRoot = ROOT_DIR_PATH .. "Source/ThirdParty/" .. libraryName .. "/"
	local libPath = libRoot .. libraryName .. ".link.lua"

	local libraryData = {}
	if FileExists(libPath) then
		-- Load the library's link file
		local createLibraryFunc = dofile(libPath)
		if createLibraryFunc == nil then
			error("Failed to load library link file: " .. libraryName .. " at path: " .. libPath .. ". File does not return the create library function.")
		end

		-- Get the library data
		libraryData = createLibraryFunc()
		if not libraryData then
			error("Failed to load library link file: " .. libraryName .. " at path: " .. libPath .. ". Create library function did not return any valid library data.")
		end
	end

	-- Add extra handy data
	libraryData.Name = libraryName
	libraryData.RootDirectory = libraryData.RootDirectoryOverride or libRoot

	return libraryData
end

function LinkToLibrary(libraryName, currentModuleData)
	local libraryData = GetLibraryData(libraryName)
	print ("Linking to library: " .. libraryName)

	-- Add library public include directories
	currentModuleData.Linked.IncludeDirs = currentModuleData.Linked.IncludeDirs or {}
	for _, includeDir in ipairs(libraryData.IncludeDirs or {}) do
		table.insert(currentModuleData.Linked.IncludeDirs, libraryData.RootDirectory .. includeDir)
	end
	table.insert(currentModuleData.Linked.IncludeDirs, libraryData.RootDirectory)

	currentModuleData.Linked.Links = currentModuleData.Linked.Links or {}
	table.insert(currentModuleData.Linked.Links, libraryData.LinkName)
end