
WKS = nil
function SetProjectWorkspace(workspace)
	WKS = workspace
end

function GetProjectWorkspace()
	if WKS == nil then
		error("WKS is nil, did you forget to call SetProjectWorkspace?")
	end
	return WKS
end