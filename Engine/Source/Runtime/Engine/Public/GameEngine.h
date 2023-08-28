#pragma once

#include "Engine_API.h"
#include "Logging/LoggingMacros.h"
#include "Engine.h"
#include "Window.h"

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

protected:
	/**
	 * Tick function call at every frame. (update function if you like)
	 *
	 * \param timestep The time step between the last frame and the current frame. (in second)
	 */
	void Tick(float timestep);

protected:
	/* The application window */
	Window* m_Window;

};
