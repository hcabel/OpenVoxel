#include "GameEngine.h"
#include "Module.h"

#ifdef WITH_EDITOR
# include "EditorEngine.h"
#else
# include "GameEngine.h"
#endif

int main(int argc, char** argv)
{
	ModuleManager::LoadModules(LoadingPhase::PreDefault);
	ModuleManager::LoadModules(LoadingPhase::Default);
	ModuleManager::LoadModules(LoadingPhase::PostDefault);

#ifdef WITH_EDITOR
	g_Engine = new EditorEngine();
#else
	g_Engine = new GameEngine();
#endif

	g_Engine->Start();

	delete g_Engine;
	g_Engine = nullptr;

	return (0);
}