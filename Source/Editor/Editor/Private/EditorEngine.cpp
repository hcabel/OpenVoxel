#include "EditorEngine.h"

EditorEngine::~EditorEngine()
{
}

void EditorEngine::OnInitialize()
{
}

void EditorEngine::EngineLoop()
{
	while (EngineShouldStop() == false)
	{
		Stop();
	}
}