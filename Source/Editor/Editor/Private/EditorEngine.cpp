#include "EditorEngine.h"

EditorEngine::~EditorEngine()
{
}

void EditorEngine::OnStart()
{
}

void EditorEngine::EngineLoop()
{
	while (IsEngineRequestedToStop() == false)
	{
		Stop();
	}
}