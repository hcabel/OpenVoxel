
local p = premake

p.modules.vscode.project = {}
local project = p.modules.vscode.project

local projectDefault = {
	targetdir = './target',
	objdir = './obj',

}

function get_visual_studio_path()
	local vswhere_output = os.outputof("vswhere -latest -property installationPath")
		if vswhere_output ~= "" then
			return vswhere_output
		else
			error("Visual Studio not found. Please install Visual Studio or set the VS_PATH environment variable.")
	end
end

function stringifyPath(str)
	str = path.translate(str)
	str = str:gsub("\\", "\\\\")
	return str;
end

function GetVisualStudioPath()
	local search_paths = {
		"C:\\Program Files (x86)\\Microsoft Visual Studio\\",
		"C:\\Program Files\\Microsoft Visual Studio\\",
	}

	for _, path in ipairs(search_paths) do
		-- list all the file in those directories
		local searchCmd = 'dir /b "' .. path .. '"'
		local searchOutput = os.outputof(searchCmd)

		-- if found something, means the path is valid, now we need to find a visual studio version directory (2022, 2019, 2017, etc.)
		if searchOutput ~= nil then
			local searchResult = split(searchOutput, "\n")

			-- loop over all the file under the directory, and find the one with 4 digits
			for _, result in ipairs(searchResult) do
				if string.find(result, "(%d%d%d%d)") then
					return path .. result .. "\\" -- return first one
				end
			end
		end
	end

	error("Visual Studio not found. Please install Visual Studio.")
end

function GetWindowsKitPath()
	local search_paths = {
		"C:\\Program Files (x86)\\Windows Kits\\",
		"C:\\Program Files\\Windows Kits\\",
	}

	for _, path in ipairs(search_paths) do
		-- list all the file in those directories
		local searchCmd = 'dir /b "' .. path .. '"'
		local searchOutput = os.outputof(searchCmd)

		-- if found something, means the path is valid, now we need to find a visual studio version directory (10, 8.1, 8.0, etc.)
		if searchOutput ~= nil then
			local searchResult = split(searchOutput, "\n")

			-- loop over all the file under the directory, and find the one with 4 digits
			for _, result in ipairs(searchResult) do
				if string.find(result, "(1%d)") then
					return path .. result .. "\\"
				end
			end
		end
	end

	error("Windows Kit not found. Please install Windows Kit.")
end

function project.generateLaunch(prj)
	local buffer = launchFileBuffer

	for cfg in p.project.eachconfig(prj) do
		local cfgName = prj.name .. ': '..cfg.buildcfg
		-- print(cfgName..' --------------------------------')
		-- print(dump(cfg, 0))
		buffer.push('{')
		buffer.w('"name": "'..cfgName..'",')
		buffer.w('"type": "cppdbg",')
		buffer.w('"request": "launch",')
		buffer.w('"program": "${workspaceRoot}/'..(cfg.targetdir or projectDefault.targetdir)..'/'..prj.name..'.exe",')
		buffer.w('"args": [],')
		buffer.w('"stopAtEntry": false,')
		buffer.w('"cwd": "${workspaceRoot}",')
		buffer.w('"environment": [],')
		buffer.w('"externalConsole": false,')
		buffer.w('"preLaunchTask": "build '..cfgName..'",')
		buffer.pop('},')
	end
end

function split(inputstr, sep)
	if sep == nil then
		sep = "%s"
	end
	local t = {}
	for str in string.gmatch(inputstr, "([^" .. sep .. "]+)") do
		table.insert(t, str)
	end
	return t
end

function project.generateTasks(prj)
	local buffer = taskFileBuffer

	local vstudioPath = GetVisualStudioPath() .. "Professional\\"
	local windowsKitPath = GetWindowsKitPath()

	for cfg in p.project.eachconfig(prj) do
		local cfgName = prj.name .. ': '..cfg.buildcfg

		-- Genereate the target dir task, this task create the target directory
		buffer.push('{')
		buffer.w('"label": "Create target dir ' .. cfgName .. '",')
		buffer.w('"type": "shell",')
		buffer.w('"command": "New-Item",')
		buffer.push('"args": [')
		buffer.w('"-ItemType",')
		buffer.w('"directory",')
		buffer.w('"-Force",')
		buffer.w('"-Path",')
		buffer.w('"\'' .. stringifyPath(cfg.targetdir or projectDefault.targetdir) .. '\'",')
		buffer.pop('],')
		buffer.pop('},')

		-- Genereate the obj dir task, this task create the object directory
		buffer.push('{')
		buffer.w('"label": "Create obj dir ' .. cfgName .. '",')
		buffer.w('"type": "shell",')
		buffer.w('"command": "New-Item",')
		buffer.push('"args": [')
		buffer.w('"-ItemType",')
		buffer.w('"directory",')
		buffer.w('"-Force",')
		buffer.w('"-Path",')
		buffer.w('"\'' .. stringifyPath(cfg.objdir or projectDefault.objdir) .. '\'"')
		buffer.pop('],')
		buffer.pop('},')

		-- Generate the compile task
		buffer.push('{')
		buffer.w('"label": "compile ' .. cfgName .. '",')
		buffer.w('"type": "shell",')
		buffer.w('"command": "' .. stringifyPath(vstudioPath .. 'VC\\Tools\\MSVC\\14.34.31933\\bin\\Hostx64\\x64\\cl.exe') .. '",')
		buffer.push('"args": [')
		buffer.w('"/c",')
		buffer.w('"/Zi",')
		buffer.w('"/nologo",')
		buffer.w('"/W3",')
		buffer.w('"/WX-",')
		buffer.w('"/diagnostics:column",')
		buffer.w('"/Od",')

		for _, define in ipairs(cfg.defines) do
			buffer.w('"/D' .. define .. '",')
		end

		buffer.w('"/Gm-",')
		buffer.w('"/EHsc",')
		buffer.w('"/MT",') -- Static runtime
		buffer.w('"/GS",')
		buffer.w('"/fp:precise",')
		buffer.w('"/Zc:wchar_t",')
		buffer.w('"/Zc:forScope",')
		buffer.w('"/Zc:inline",')

		for _, incdir in ipairs(cfg.includedirs) do
			buffer.w('"\'/I\\"' .. stringifyPath(incdir) .. '\\"\'",')
		end

		local searchCmd = 'dir /b "' .. windowsKitPath .. 'Include\\10.0.22000.0"'
		local searchOutput = os.outputof(searchCmd)
		if searchOutput then
			local searchResult = split(searchOutput, "\n")
			for _, result in ipairs(searchResult) do
				buffer.w('"\'/I\\"' .. stringifyPath(windowsKitPath .. 'Include\\10.0.22000.0\\' .. result) .. '\\"\'",')
			end
		end

		buffer.w('"\'/I\\"' .. stringifyPath(vstudioPath .. 'VC\\Tools\\MSVC\\14.34.31933\\include') .. '\\"\'",')

		buffer.w('"\'/Fo\\\"' .. stringifyPath(cfg.objdir) .. '/\\\"\'",')
		-- TODO: FD

		buffer.w('"/external:W3",')
		buffer.w('"/Gd",')
		buffer.w('"/TC",')
		buffer.w('"/FC",')
		buffer.w('"/errorReport:prompt",')

		for _, file in ipairs(cfg.files) do
			buffer.w('"' .. stringifyPath(file) .. '",')
		end

		buffer.pop('],')
		buffer.pop('},')

		-- Generate the link task
		if cfg.kind == p.STATICLIB then
			buffer.push('{')
			buffer.w('"label": "link ' .. cfgName .. '",')
			buffer.w('"type": "shell",')
			buffer.w('"command": "' .. stringifyPath(vstudioPath .. 'VC\\Tools\\MSVC\\14.34.31933\\bin\\HostX64\\x64\\lib.exe') .. '",')
			buffer.push('"args": [')
			buffer.w('"\'/OUT:\\"' .. stringifyPath(cfg.targetdir) .. '\\\\' .. cfg.name .. '.lib\\"\'",')
			buffer.w('"/NOLOGO",')
			buffer.w('"/MACHINE:X64",')

			for _, file in ipairs(cfg.files) do
				local objFile = cfg.objdir .. "\\" .. path.getbasename(file) .. '.obj'
				buffer.w('"' .. stringifyPath(objFile) .. '",')
			end

			buffer.pop('],')
			buffer.pop('},')
		end

		-- Generate the build task
		buffer.push('{')
		buffer.w('"label": "build ' .. cfgName .. '",')
		buffer.w('"type": "shell",')
		buffer.w('"command": "make",')
		buffer.w('"args": [],')
		buffer.push('"group": {')
		buffer.w('"kind": "build",')
		buffer.w('"isDefault": true')
		buffer.pop('},')
		buffer.w('"problemMatcher": [],')
		buffer.w('"dependsOrder": "sequence",')
		buffer.push('"dependsOn": [')

		buffer.w('"Create target dir ' .. cfgName .. '",')
		buffer.w('"Create obj dir ' .. cfgName .. '",')
		if cfg.kind == p.STATICLIB then
			buffer.w('"compile ' .. cfgName .. '",')
			buffer.w('"link ' .. cfgName .. '"')
		end

		buffer.pop(']')
		buffer.pop('},')
	end
end