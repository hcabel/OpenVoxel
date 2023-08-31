#include "EditorEngine.h"
#include "OVModuleManager.h"
#include "HAL/Time.h"
#include "EditorWindow.h"

EditorEngine::~EditorEngine()
{
	delete m_Window;

	OVModuleManager::Get().Unload("UI");
}

void EditorEngine::OnInitialize()
{
	OVModuleManager::Get().Load("UI");

	m_Window = new EditorWindow(1280, 720, "Open Voxel Editor");
}

void EditorEngine::EngineLoop()
{
	while (EngineShouldStop() == false)
	{
		m_Window->Tick(0.0f);

		m_Window->Draw();

		Time::CalculateNewTiming();
	}
}

bool EditorEngine::EngineShouldStop()
{
	return (
		m_State == EngineState::Type::Stopping
		|| m_Window->IsClosed()
	);
}
