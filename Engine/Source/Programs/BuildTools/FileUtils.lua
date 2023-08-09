
function ResolveRelativePaths(object, root)
	if type(object) == "table" then
		for key, value in pairs(object) do
			if type(value) == "string" then
				object[key] = root .. value
			else
				ResolveTableOfRelativePaths(value, root)
			end
		end
	elseif type(object) == "string" then
		object = root .. object
	end
end

function FileExists(filePath)
	local file = io.open(filePath, "rb")
	if file then
		file:close()
	end
	return file ~= nil
end

function GetFolders(directory)
	local command = 'ls -d "' .. directory .. '/*/"'
	if package.config:sub(1,1) == '\\' then
		command = 'dir /b /ad "' .. directory .. '"'
	end

	local folders = {}
	local handle = io.popen(command)
	local output = handle:read("*a")
	handle:close()

	for folder in output:gmatch("[^\r\n]+") do
		table.insert(folders, folder)
	end

	return folders
end