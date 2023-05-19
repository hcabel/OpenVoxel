#include "GameEngine.h"
#include "Module.h"

int main(int argc, char** argv)
{
	ModuleManager::LoadModules();

	Engine::Get()->Start();
	return (0);
}