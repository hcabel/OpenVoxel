
include "../GlobalValues.lua"

local LibraryDataCache = {}

function LoadLibraryFromDisk(libraryName)
	print ("Loading '" .. libraryName .. "' library from disk")

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

function GetLibraryData(libraryName)
	if LibraryDataCache[libraryName] == nil then
		LibraryDataCache[libraryName] = LoadLibraryFromDisk(libraryName)
	end

	return LibraryDataCache[libraryName]
end

function LinkToLibrary(libraryName, buildData)
	local libraryData = GetLibraryData(libraryName)

	-- Add library public include directories
	buildData.Resolved.Public_IncludeDirs = buildData.Resolved.Public_IncludeDirs or {}
	for _, includeDir in ipairs(libraryData.IncludeDirs or {}) do
		table.insert(buildData.Resolved.Public_IncludeDirs, libraryData.RootDirectory .. includeDir)
	end
	table.insert(buildData.Resolved.Public_IncludeDirs, libraryData.RootDirectory)

	buildData.Resolved.LibrariesDependencies = buildData.Resolved.LibrariesDependencies or {}
	table.insert(buildData.Resolved.LibrariesDependencies, libraryData.LinkName)
end