#include "Engine.h"

void Engine::Initialize()
{
	m_State = EngineState::Type::Starting;
	OnInitialize();
}

void Engine::Start()
{
	m_State = EngineState::Type::Running;
	EngineLoop();
}

void Engine::Stop()
{
	m_State = EngineState::Type::Stopping;
}