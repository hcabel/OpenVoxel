#include "GameEngine.h"
#include "Profiling/ProfilingMacros.h"
#include "HAL/Time.h"
#include "GameWindow.h"

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
	delete m_Window;
}

void GameEngine::OnInitialize()
{
	CREATE_SCOPE_NAMED_TIMER_CONSOLE(EngineStartup);
	OV_LOG(GameEngineLog, Display, "Init Engine");

	m_Window = new GameWindow(1280, 720, "Open Voxel");
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

	const uint16_t fpsCount = (uint16_t)std::round(1.0f / Time::GetTimeStep());
	m_Window->SetTitle(std::format("OpenVoxel - {:.2f}ms = {:d}fps", Time::GetTimeStep() * 100.0f, fpsCount).c_str());
}

bool GameEngine::EngineShouldStop()
{
	return (
		m_State == EngineState::Type::Stopping
		|| m_Window->IsClosed()
	);
}
