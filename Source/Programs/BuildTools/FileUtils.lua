
function FileExists(filePath)
	local file = io.open(filePath, "rb")
	if file then
		file:close()
	end
	return file ~= nil
end
