//
// Win32 Window class
//
// Written by Oliver Öhrström
//

#pragma once
#include "stdafx.h"
#include "vec/Vec.h"
//#include "Keycodes.h"

#include <bitset>

struct Rect
{
	union
	{
		struct
		{
			int xMin;
			int yMin;
			int xMax;
			int yMax;
		};
		struct
		{
			linalg::int2 lowerBounds;
			linalg::int2 upperBounds;
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

	linalg::Vec2i GetSize() const noexcept;
	Rect GetBounds() const noexcept;
	bool SizeChanged() const noexcept;

public: // Input methods

	//bool KeyDown(Keys key) const noexcept;
	//bool KeyUp(Keys key) const noexcept;
private:

	LRESULT CALLBACK WindowCallback(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK CallbackWrapper(HWND, UINT, WPARAM, LPARAM);

private:

	static Window* instance;
	HWND windowHandle;
	size_t width;
	size_t height;
	bool sizeChanged;

private: // Input management

	std::bitset<256> keys;
};