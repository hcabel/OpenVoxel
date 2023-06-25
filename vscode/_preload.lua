local p = premake

newaction
{
	trigger         = "vscode",
	shortname       = "VsCode",
	description     = "Generate Visual Studio Code workspace",

	-- The capabilities of this action

	valid_kinds     = { "ConsoleApp", "SharedLib", "Utility", "StaticLib" },
	valid_languages = { "C", "C++" },

	-- Events

	onStart = function()
		p.modules.vscode.initLaunchFileBuffer();
		p.modules.vscode.initTaskFileBuffer();
	end,

	onWorkspace = function(wks)
		p.generate(wks, ".code-workspace", p.modules.vscode.generateWorkspace)
	end,

	onProject = function(prj)
		p.modules.vscode.generateProject(prj, launchFileBuffer, taskFileBuffer)
	end,

	onEnd = function()
		p.modules.vscode.deinitLaunchFileBuffer();
		p.modules.vscode.deinitTaskFileBuffer();

		p.generate({ location = './.vscode', filename = 'launch' }, ".json", function() p.w(launchFileBuffer.string) end)
		p.generate({ location = './.vscode', filename = 'tasks' }, ".json", function() p.w(taskFileBuffer.string) end)
	end,
}

return function(cfg)
	return (_ACTION == "vscode")
end