#pragma once

#include "Renderer_API.h"

#include <cstdint>

/**
 * This class represent a window, in the operating system.
 */
class RENDERER_API Window
{
public:
	using AxisSize = uint16_t;

public:
	Window(AxisSize width, AxisSize height, const char* title)
		: m_Width(width), m_Height(height), m_Title(title), m_HasBeenResized(false)
	{}
	virtual ~Window() = default;

public:
	virtual void Draw() = 0;
	virtual void Tick(float deltaTime) = 0;

public:
	__forceinline virtual bool HasBeenResized() const { return m_HasBeenResized; }
	__forceinline virtual bool IsMinimized() const { return m_Width <= 0 || m_Height <= 0; }
	virtual bool IsClosed() const = 0;

	__forceinline AxisSize GetWidth() const { return m_Width; }
	__forceinline AxisSize GetHeight() const { return m_Height; }
	__forceinline const char* GetTitle() const { return m_Title; }

	virtual void SetSize(AxisSize width, AxisSize height) = 0;
	void SetTitle(const char* title);

protected:
	virtual void OnTitleUpdate() {}

protected:
	AxisSize m_Width;
	AxisSize m_Height;
	const char* m_Title;
	bool m_HasBeenResized;
};
