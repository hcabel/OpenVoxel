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

		float timeStep = Time::GetTimeStep();

		m_Window->Tick(timeStep);

		const uint16_t fpsCount = (uint16_t)std::round(1.0f / timeStep);
		m_Window->SetTitle(std::format("Open Voxel Editor - {:.2f}ms = {:d}fps", timeStep * 100.0f, fpsCount).c_str());

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
