#include "GameEngine.h"
#include "Profiling/ProfilingMacros.h"
#include "HAL/PlatformTime.h"
#include "Renderer.h"

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
	Renderer::Shutdown();
}

void GameEngine::OnInitialize()
{
	CREATE_SCOPE_NAMED_TIMER_CONSOLE(EngineStartup);
	OV_LOG(Display, GameEngineLog, "Init Engine");
}

void GameEngine::EngineLoop()
{
	OV_LOG(Display, GameEngineLog, "Engine Running");

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

	glfwPollEvents();

	const uint16_t fpsCount = (uint16_t)std::round(1.0f / PlatformTime::GetTimeStep());
	GLFWwindow* glfwWindow = RendererModule::GetWindow();
	glfwSetWindowTitle(glfwWindow, std::format("OpenVoxel - {:.2f}ms = {:d}fps", PlatformTime::GetTimeStep() * 100.0f, fpsCount).c_str());

	Stop();
}

bool GameEngine::EngineShouldStop()
{
	GLFWwindow* glfwWindow = RendererModule::GetWindow();
	return (
		m_State == EngineState::Type::Stopping
		|| glfwWindowShouldClose(glfwWindow)
	);
}