#pragma once

#include "UI.h"

#include <string>

class UIConsole : public UI
{
public:
	UIConsole();

protected:
	void Tick(float deltaTime) override;
	void Draw() override;

protected:
	std::vector<std::string> m_Logs;

};
