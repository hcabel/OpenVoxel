function dump(object)
	if type(object) == 'table' then
		local s = '{ '
		for k,v in pairs(object) do
			if type(k) ~= 'number' then k = '"'..k..'"' end
			s = s .. '['..k..'] = ' .. dump(v) .. ','
		end
		return s .. '} '
	else
		return tostring(object)
	end
end

function DEBUG_PrintModuleData(moduleData)
	print("Module: " .. moduleData.Name)
	print("\tFiles: ")
	for _, filePath in ipairs(moduleData.Files) do
		print("\t\t" .. filePath)
	end
	print("\tPublic_IncludeDirs: ")
	for _, includeDirPath in ipairs(moduleData.Public_IncludeDirs) do
		print("\t\t" .. includeDirPath)
	end
	print("\tPrivate_IncludeDirs: ")
	for _, includeDirPath in ipairs(moduleData.Private_IncludeDirs) do
		print("\t\t" .. includeDirPath)
	end
	print("\tModulesDependencies: " .. table.concat(moduleData.ModulesDependencies, ", "))
	print("\tLibrariesDependencies: " .. table.concat(moduleData.LibrariesDependencies, ", "))
end
