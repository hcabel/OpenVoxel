#pragma once

#include "Logging/LoggingMacros.h"
#include "EngineModule.h"
#include "Engine.h"

#undef GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

DECLARE_LOG_CATEGORY(GameEngineLog);

class ENGINE_API GameEngine : public Engine
{
public:
	GameEngine() = default;
	virtual ~GameEngine();

public:
	//~ Begin Engine Interface
	virtual void OnInitialize() override;
	virtual void EngineLoop() override;
	bool EngineShouldStop() override;
	//~ End Engine Interface

private:
	/**
	 * Tick function call at every frame. (update function if you like)
	 *
	 * \param timestep The time step between the last frame and the current frame. (in second)
	 */
	void Tick(float timestep);
};
