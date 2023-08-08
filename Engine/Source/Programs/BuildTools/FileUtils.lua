
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
