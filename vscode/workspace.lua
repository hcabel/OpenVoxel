
local p = premake;

p.modules.vscode.workspace = {}
local workspace = p.modules.vscode.workspace

function workspace.generate(wks)
	p.push('{')
	p.push('"folders": [')
	p.push('{')
	p.w('"path": "."')
	p.pop('}')
	p.pop('],')
	p.pop('}')
end


