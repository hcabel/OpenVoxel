#pragma once

#include "UI_API.h"

#include <vector>
#include <imgui.h>

/**
 * This is the interface that every UI should implement.
*/
class UI_API UI
{
public:
	UI() = default;
	virtual ~UI()
	{
		for (auto& child : m_Children)
			delete child;
		m_Children.clear();
	}

protected:
	virtual void Tick(float deltaTime) = 0;
	virtual void Draw() = 0;

	void TickChildren(float deltaTime)
	{
		for (auto& child : m_Children)
			child->Tick(deltaTime);
	}
	void DrawChildren()
	{
		if (!m_DockSpaceId)
			m_DockSpaceId = ImGui::GetWindowDockID();

		for (auto& child : m_Children)
		{
			ImGui::SetNextWindowDockID(m_DockSpaceId, ImGuiCond_FirstUseEver);
			child->Draw();
		}
	}

protected:
	__forceinline void AddChild(UI* child) { m_Children.push_back(child); }
	std::vector<UI*>& GetChildren() { return m_Children; }

private:
	std::vector<UI*> m_Children;

protected:
	ImGuiID m_DockSpaceId = 0;
};


