#pragma once

#include "Renderer_API.h"
#include "Module.h"

struct GLFWwindow;

class RendererModule final : public Module
{

public:
	void StartupModule() override;
	void ShutdownModule() override;
};
