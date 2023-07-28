#include "EditorEngine.h"
#include "OVModuleManager.h"
#include "Renderer.h"
#include "HAL/PlatformTime.h"
#include "UI.h"

EditorEngine::~EditorEngine()
{
}

void EditorEngine::OnInitialize()
{
	OVModuleManager::LoadModule("UI");
}

void EditorEngine::EngineLoop()
{
	while (EngineShouldStop() == false)
	{
		UI::Get().PrepareNewFrame();

		Renderer::Get().Tick();

		UI::Get().RenderNewFrame();

		PlatformTime::CalculateNewTiming();
	}
}

bool EditorEngine::EngineShouldStop()
{
	GLFWwindow* glfwWindow = RendererModule::GetWindow();
	return (
		m_State == EngineState::Type::Stopping
		|| Renderer::Get().IsWindowClosed()
	);
}
