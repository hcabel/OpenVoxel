#pragma once

#include "Engine.h"

class EditorEngine : public Engine
{

public:
	EditorEngine() = default;
	virtual ~EditorEngine();

public:
	//~ Begin Engine interface
	virtual void OnStart() override;
	virtual void EngineLoop() override;
	//~ End Engine interface

};
