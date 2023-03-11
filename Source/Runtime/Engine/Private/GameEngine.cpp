#include "GameEngine.h"
#include "Profiling/ProfilingMacros.h"

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
}

void GameEngine::OnStart()
{
	OV_LOG(Display, GameEngineLog, "Starting Engine");
}

void GameEngine::EngineLoop()
{

	while (IsEngineRequestedToStop() == false)
	{
		RESET_ALL_PERFRAME_TIMER_DATA;

		Stop();
	}

}

bool GameEngine::IsEngineRequestedToStop()
{
	return (m_State == EngineState::Type::Stopping);
}