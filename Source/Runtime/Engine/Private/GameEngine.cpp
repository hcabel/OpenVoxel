#include "GameEngine.h"
#include "Profiling/ProfilingMacros.h"
#include "HAL/PlatformTime.h"

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
}

void GameEngine::OnStart()
{
	CREATE_SCOPE_NAMED_TIMER_CONSOLE(EngineStartup);
	OV_LOG(Display, GameEngineLog, "Starting Engine");

}

void GameEngine::EngineLoop()
{
	OV_LOG(Display, GameEngineLog, "Starting Engine Loop");

	PlatformTime::Init();
	while (IsEngineRequestedToStop() == false)
	{
		RESET_ALL_PERFRAME_TIMER_DATA;

		// const uint16_t fpsCount = (uint16_t)std::round(1.0f / PlatformTime::GetTimeStep());
		// glfwSetWindowTitle(, std::format("OpenVoxel - {:.2f}ms = {:d}fps", PlatformTime::GetTimeStep() * 100.0f, fpsCount).c_str());

		// glfwPollEvents();

		// m_Window->NewFrame();

		Stop();

		PlatformTime::CalculateNewTiming();
	}
}

bool GameEngine::IsEngineRequestedToStop()
{
	return (
		m_State == EngineState::Type::Stopping
		// || glfwWindowShouldClose(m_Window)
	);
}