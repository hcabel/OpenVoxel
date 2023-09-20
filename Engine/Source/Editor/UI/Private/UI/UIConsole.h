#pragma once

#include "UI.h"
#include "Logging/Logger.h"

#include <string>

class UIConsole : public UI
{
public:
	UIConsole();
	~UIConsole();

protected:
	void Tick(float deltaTime) override;
	void Draw() override;

protected:
	void OnLogMessage(LogCategory& category, Verbosity::Type verbosity, std::string_view fullyFormattedMessage);

protected:
	std::vector<std::string> m_Logs;

};
