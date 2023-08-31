#pragma once

#include "Editor_API.h"
#include "Engine.h"
#include "Window.h"

class EDITOR_API EditorEngine : public Engine
{

public:
	EditorEngine() = default;
	virtual ~EditorEngine();

public:
	//~ Begin Engine interface
	virtual void OnInitialize() override;
	virtual void EngineLoop() override;
	virtual bool EngineShouldStop() override;
	//~ End Engine interface

protected:
	Window* m_Window;

};
