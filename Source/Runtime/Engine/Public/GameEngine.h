#pragma once

#include "EngineModule.h"
#include "Engine.h"

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
