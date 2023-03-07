#include "GameEngine.h"

#include <iostream>

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
}

void GameEngine::OnStart()
{
	OV_LOG(Verbosity::Type::Display, GameEngineLog, "Starting Engine");
}

void GameEngine::EngineLoop()
{

	while (IsEngineRequestedToStop() == false)
	{
		Stop();
	}

}

bool GameEngine::IsEngineRequestedToStop()
{
	return (m_State == EngineState::Type::Stopping);
}