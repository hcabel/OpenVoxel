#include "GameEngine.h"
#include "Profiling/ProfilingMacros.h"
#include "HAL/PlatformTime.h"

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
	m_Window.reset();
}

void GameEngine::OnStart()
{
	CREATE_SCOPE_NAMED_TIMER_CONSOLE(EngineStartup);

	OV_LOG(Display, GameEngineLog, "Starting Engine");

	m_Window = std::make_unique<MainWindow>(1080, 720, "OpenVoxel");
}

void GameEngine::EngineLoop()
{
	OV_LOG(Display, GameEngineLog, "Starting Engine Loop");

	PlatformTime::Init();
	while (IsEngineRequestedToStop() == false)
	{
		RESET_ALL_PERFRAME_TIMER_DATA;

		OV_LOG(Display, GameEngineLog, "Engine Loop Tick {:f}", PlatformTime::GetTimeStep());

		PlatformTime::CalculateNewTiming();
	}
}

bool GameEngine::IsEngineRequestedToStop()
{
	return (
		m_State == EngineState::Type::Stopping
		|| m_Window->ShouldClose()
	);
}