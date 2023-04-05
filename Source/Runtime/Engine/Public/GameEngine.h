#pragma once

#include "Logging/LoggingMacros.h"
#include "EngineModule.h"
#include "Engine.h"

DECLARE_LOG_CATEGORY(GameEngineLog);

class ENGINE_API GameEngine : public Engine
{
public:
	GameEngine() = default;
	virtual ~GameEngine();

public:
	//~ Begin Engine Interface
	virtual void OnStart() override;
	virtual void EngineLoop() override;
	bool IsEngineRequestedToStop();
	//~ End Engine Interface

};
