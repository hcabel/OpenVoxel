local function IsTableEmpty(table, depth)
	if depth > 5 then
		return false
	end

	for _, value in pairs(table) do
		if type(value) == "table" then
			if IsTableEmpty(value, depth + 1) == false then
				return false
			end
		else
			return false
		end
	end
	return true
end




function dump(object, showEmpty)
	function recursiveDump(obj, indent)
		if indent > 5 then
			return "..."
		end
		if type(obj) == "table" then
			local str = "{\n"
			for k, v in pairs(obj) do
				if type(k) ~= "number" then
					k = "\"" .. k .. "\""
				end

				-- If is a table, we recursively print only is they are not empty
				if type(v) == "table" then
					-- recursivly check whether the table is empty or not
					local isEmpty = IsTableEmpty(v, indent + 1)
					if isEmpty == false or showEmpty then
						str = str .. string.rep("\t", indent) .. "[" .. k .. "] = " .. recursiveDump(v, indent + 1) .. ",\n"
					end
				else
					str = str .. string.rep("\t", indent) .. "[" .. k .. "] = " .. recursiveDump(v, indent + 1) .. ",\n"
				end
			end
			str = str .. string.rep("\t", indent - 1) .. "}"
			return str
		else
			return tostring(obj)
		end
	end

	if type(object) == "table" then
		return recursiveDump(object, 1)
	else
		return tostring(object)
	end
end