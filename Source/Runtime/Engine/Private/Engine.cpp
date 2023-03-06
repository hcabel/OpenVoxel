#include "Engine.h"
#include "GameEngine.h"

#if WITH_EDITOR
# include "EditorEngine.h"
#endif

#include <iostream>

Engine::~Engine()
{
	std::cout << "Engine::~Engine()" << std::endl;
}

std::shared_ptr<Engine>& Engine::Get()
{
	static std::shared_ptr<Engine> instance = nullptr;
	if (!instance)
		instance = CreateInstance();
	return (instance);
}

std::shared_ptr<Engine> Engine::CreateInstance()
{
#if WITH_EDITOR
	return std::make_shared<EditorEngine>();
#endif
	return std::make_shared<GameEngine>();
}

void Engine::Start()
{
	std::cout << "Engine::Start()" << std::endl;

	OnStart();
	m_IsRequestToStop = false;
	EngineLoop();

	std::cout << "Engine::Start() - End" << std::endl;
}

void Engine::Stop()
{
	std::cout << "Engine::Stop()" << std::endl;
	m_IsRequestToStop = true;
}