#include "Engine.h"

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