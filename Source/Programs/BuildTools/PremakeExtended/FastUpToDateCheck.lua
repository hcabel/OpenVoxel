require "vstudio"
local vc = premake.vstudio.vc2010

premake.api.register {
	name = "FastUpToDateCheck",
	scope = "config",
	kind = "boolean",
	default = true
}

function FastUpToDateCheckFunction(prj, cfg)
	if cfg.FastUpToDateCheck == false then
		vc.element("DisableFastUpToDateCheck", nil, "true")
	end
end

premake.override(vc.elements, "globalsCondition",
	function(oldfn, prj, cfg)
		local elements = oldfn(prj, cfg)
		elements = table.join(elements, {FastUpToDateCheckFunction})
		return elements
	end)