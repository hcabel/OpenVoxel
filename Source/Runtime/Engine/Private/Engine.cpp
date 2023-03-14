#include "Engine.h"
#include "GameEngine.h"

#if WITH_EDITOR
# include "EditorEngine.h"
#endif

#include <iostream>

std::shared_ptr<Engine>& Engine::Get()
{
	static std::shared_ptr<Engine> instance = nullptr;
	if (!instance)
		instance = CreateInstance();
	return (instance);
}

std::shared_ptr<Engine> Engine::CreateInstance()
{
#if WITH_EDITOR
	return std::make_shared<EditorEngine>();
#else
	return std::make_shared<GameEngine>();
#endif
}

void Engine::Start()
{
	m_State = EngineState::Type::Starting;
	OnStart();
	m_State = EngineState::Type::Running;
	EngineLoop();
}

void Engine::Stop()
{
	m_State = EngineState::Type::Stopping;
}