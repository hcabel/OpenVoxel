#include "UIConsole.h"
#include "Path.h"
#include "HAL/File.h"
#include "Logging/Logger.h"
#include "UIGlobals.h"

#include <imgui.h>

UIConsole::UIConsole()
{
	std::string fileContent = Logger::s_LogFile->ReadAll();
	std::string_view fileContentView = fileContent;
	uint32_t lineCount = std::count(fileContent.begin(), fileContent.end(), '\n');
	m_Logs.reserve(lineCount);
	while (fileContentView.find('\n') != std::string_view::npos)
	{
		m_Logs.emplace_back(fileContentView.substr(0, fileContentView.find('\n')));
		fileContentView.remove_prefix(fileContentView.find('\n') + 1);
	}

	// Remove all unwanted logs, we could do that in the loop above but since it will only happen once, I'll leave it here for now
	for (auto it = m_Logs.begin(); it != m_Logs.end();)
	{
		if (it->find("VeryVerbose:") != std::string::npos)
			it = m_Logs.erase(it);
		else
			it++;
	}

	Logger::s_OnLogMessage.BindLambda(
		[this](LogCategory& category, Verbosity::Type verbosity, std::string_view message)
		{
			m_Logs.emplace_back(std::format("[{:s}]: {:s}: {:s}", category.GetName(), Verbosity::ToString(verbosity), message));
		}
	);
}

void UIConsole::Tick(float deltaTime)
{
	TickChildren(deltaTime);
}

void UIConsole::Draw()
{
	static bool show = true;
	if (show)
		ImGui::ShowDemoWindow(&show);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Console"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 5)); // Set a small gap between lines so that we can see more logs

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		float lineHeight = ImGui::GetTextLineHeight();

		// Draw the logs, but only the ones that are visible
		ImGuiListClipper clipper;
		clipper.Begin(m_Logs.size());
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				ImVec2 cursorPos = ImGui::GetCursorScreenPos();

				ImVec2 rectMin = ImVec2(
					cursorPos.x,
					cursorPos.y - (ImGui::GetStyle().ItemSpacing.y / 2.0f)
				);
				ImVec2 rectMax = ImVec2(
					rectMin.x + ImGui::GetWindowWidth(),
					rectMin.y + lineHeight + ImGui::GetStyle().ItemSpacing.y
				);
				drawList->AddRectFilled(rectMin, rectMax,
					(i % 2 == 0) ? ImGui::GetColorU32(ImGuiCol_WindowBg) : ImGui::GetColorU32(ImGuiCol_FrameBg)
				);
				drawList->AddText(ImVec2(cursorPos.x + 20, cursorPos.y), ImGui::GetColorU32(ImGuiCol_Text), m_Logs[i].c_str());

				ImGui::Dummy(ImVec2(0.0f, lineHeight)); // Go to the next line
			}
		}
		clipper.End();

		ImGui::PopStyleVar();

		// If we're at the bottom of the console, scroll down in order to see the latest logs
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::End();

		DrawChildren();
	}

	ImGui::PopStyleVar();
}
