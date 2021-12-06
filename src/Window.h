//
// Win32 Window class
//
// Written by Oliver Öhrström
//

#pragma once
#include "stdafx.h"
#include "vec/vec.h"
//#include "Keycodes.h"

#include <bitset>

struct Rect
{
	union
	{
		struct
		{
			int XMin;
			int YMin;
			int XMax;
			int YMax;
		};
		struct
		{
			linalg::int2 LowerBounds;
			linalg::int2 UpperBounds;
		};
	};
};

class Window
{
public:
	Window(HINSTANCE instance, int nCmdShow, int width = 728, int height = 728);
	~Window() noexcept;

	bool Update() noexcept;
	HWND GetHandle() noexcept;

	linalg::vec2i GetSize() const noexcept;
	Rect GetBounds() const noexcept;
	bool SizeChanged() const noexcept;

public: // Input methods
	//bool KeyDown(Keys key) const noexcept;
	//bool KeyUp(Keys key) const noexcept;
private:
	LRESULT CALLBACK WindowCallback(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK CallbackWrapper(HWND, UINT, WPARAM, LPARAM);

private:
	static Window* s_instance;
	HWND m_windowHandle;
	size_t m_width;
	size_t m_height;
	bool m_sizeChanged;

private: // Input management
	std::bitset<256> m_keys;
};