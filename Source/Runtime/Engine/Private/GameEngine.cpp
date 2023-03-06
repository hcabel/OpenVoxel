#include "GameEngine.h"

#include <iostream>

GameEngine::~GameEngine()
{
	std::cout << "GameEngine::OnCleanup()" << std::endl;
}

void GameEngine::OnStart()
{
	std::cout << "GameEngine::OnStart()" << std::endl;
}

void GameEngine::EngineLoop()
{
	std::cout << "GameEngine::EngineLoop()" << std::endl;

	while (IsEngineRequestedToStop() == false)
	{
		Stop();
	}

	std::cout << "GameEngine::EngineLoop() - END" << std::endl;
}

bool GameEngine::IsEngineRequestedToStop()
{
	return (m_IsRequestToStop);
}