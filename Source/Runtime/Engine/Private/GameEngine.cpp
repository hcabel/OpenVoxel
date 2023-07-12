#include "GameEngine.h"
#include "Profiling/ProfilingMacros.h"
#include "HAL/PlatformTime.h"
#include "Renderer.h"
#include "RendererModule.h"

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
	Renderer::Shutdown();
}

void GameEngine::OnInitialize()
{
	CREATE_SCOPE_NAMED_TIMER_CONSOLE(EngineStartup);
	OV_LOG(GameEngineLog, Display, "Init Engine");
}

void GameEngine::EngineLoop()
{
	OV_LOG(GameEngineLog, Display, "Engine Running");

	PlatformTime::Init();
	while (EngineShouldStop() == false)
	{
		CLEAR_ALL_PERFRAME_TIMER_DATA();

		Renderer::Get()->PrepareNewFrame();

		float timeStep = PlatformTime::GetTimeStep();
		Tick(timeStep);

		Renderer::Get()->RenderNewFrame();

		PlatformTime::CalculateNewTiming();
	}
}

void GameEngine::Tick(float timeStep)
{
	CREATE_SCOPE_NAMED_TIMER_CONSOLE(GameEngineTick);

	Renderer::Get()->Tick();

	// Stop();
}

bool GameEngine::EngineShouldStop()
{
	GLFWwindow* glfwWindow = RendererModule::GetWindow();
	return (
		m_State == EngineState::Type::Stopping
		|| Renderer::Get()->IsWindowClosed()
	);
}
