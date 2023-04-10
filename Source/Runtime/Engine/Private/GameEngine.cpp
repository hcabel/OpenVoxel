#include "GameEngine.h"
#include "Profiling/ProfilingMacros.h"
#include "HAL/PlatformTime.h"
#include "Renderer.h"

DEFINE_LOG_CATEGORY(GameEngineLog);

GameEngine::~GameEngine()
{
	Renderer::Shutdown();
}

void GameEngine::OnStart()
{
	CREATE_SCOPE_NAMED_TIMER_CONSOLE(EngineStartup);
	OV_LOG(Display, GameEngineLog, "Starting Engine");

	// Create glfwWindow
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO
	m_Window = glfwCreateWindow(1280, 720, "OpenVoxel", nullptr, nullptr);

	Renderer::Initialize(m_Window);
}

void GameEngine::EngineLoop()
{
	OV_LOG(Display, GameEngineLog, "Starting Engine Loop");

	PlatformTime::Init();
	while (IsEngineRequestedToStop() == false)
	{
		RESET_ALL_PERFRAME_TIMER_DATA;

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
	glfwSetWindowTitle(m_Window, std::format("OpenVoxel - {:.2f}ms = {:d}fps", PlatformTime::GetTimeStep() * 100.0f, fpsCount).c_str());

	Stop();
}

bool GameEngine::IsEngineRequestedToStop()
{
	return (
		m_State == EngineState::Type::Stopping
		|| glfwWindowShouldClose(m_Window)
	);
}