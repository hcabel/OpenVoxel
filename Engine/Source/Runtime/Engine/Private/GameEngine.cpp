#include "GameEngine.h"
#include "Profiling/ProfilingMacros.h"
#include "HAL/Time.h"
// #include "Renderer.h"
#include "GlfwWindow.h"

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
}

void GameEngine::OnInitialize()
{
	CREATE_SCOPE_NAMED_TIMER_CONSOLE(EngineStartup);
	OV_LOG(GameEngineLog, Display, "Init Engine");

	m_Window = new GlfwWindow(1280, 720, "Open Voxel");
}

void GameEngine::EngineLoop()
{
	OV_LOG(GameEngineLog, Display, "Engine Running");

	Time::Init();
	while (EngineShouldStop() == false)
	{
		CLEAR_ALL_PERFRAME_TIMER_DATA();

		float timeStep = Time::GetTimeStep();
		Tick(timeStep);

		m_Window->Draw();

		Time::CalculateNewTiming();
	}
}

void GameEngine::Tick(float timeStep)
{
 	m_Window->Tick(timeStep);
}

bool GameEngine::EngineShouldStop()
{
	return (
		m_State == EngineState::Type::Stopping
		|| m_Window->IsClosed()
	);
}
