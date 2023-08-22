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

-- Target is the last part of the path (after the last /)
function GetPathTarget(path)
	local target = path:match("^.+/(.+)$")
	if target == nil then
		target = path
	end
	return target
end