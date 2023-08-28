#include "Window.h"

void Window::SetTitle(const char* title)
{
	m_Title = title;
	OnTitleUpdate();
}
