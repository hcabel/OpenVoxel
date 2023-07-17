#pragma once

#include "Editor_API.h"
#include "Engine.h"

class EDITOR_API EditorEngine : public Engine
{

public:
	EditorEngine() = default;
	virtual ~EditorEngine();

public:
	//~ Begin Engine interface
	virtual void OnInitialize() override;
	virtual void EngineLoop() override;
	//~ End Engine interface

};
