local p = premake

p.modules.vscode = {}
local vscode = p.modules.vscode

include("buffer.lua")
launchFileBuffer = CreateNewBuffer()
taskFileBuffer = CreateNewBuffer()

function vscode.initLaunchFileBuffer()
	launchFileBuffer.push('{')
	launchFileBuffer.w('"version": "0.2.0",')
	launchFileBuffer.push('"configurations": [')
end

function vscode.initTaskFileBuffer()
	taskFileBuffer.push('{')
	taskFileBuffer.w('"version": "2.0.0",')
	taskFileBuffer.push('"tasks": [')
end

function vscode.generateWorkspace(wks)
	vscode.workspace.generate(wks)
end

function vscode.generateProject(prj)
	return {
		vscode.project.generateLaunch(prj),
		vscode.project.generateTasks(prj)
	}
end

function vscode.deinitLaunchFileBuffer()
	launchFileBuffer.pop(']')
	launchFileBuffer.pop('}')
end

function vscode.deinitTaskFileBuffer()
	taskFileBuffer.pop(']')
	taskFileBuffer.pop('}')
end

include("workspace.lua")
include("project.lua")

include("_preload.lua")

return vscode;