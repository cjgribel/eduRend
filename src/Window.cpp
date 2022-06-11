//
// Win32 Window class
//
// Written by Oliver Öhrström
//

#include "Window.h"
#include <windowsx.h>

Window* Window::instance = nullptr;

bool Window::Update() noexcept
{
	sizeChanged = false;
	//m_PreviousMousePos = m_mousePos;
	MSG msg = { 0 };

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			return true;
		}
	}

	return false;
}

HWND Window::GetHandle() noexcept
{
	return windowHandle;
}

linalg::Vec2i Window::GetSize() const noexcept
{
	return { static_cast<int>(width), static_cast<int>(height) };
}

Rect Window::GetBounds() const noexcept
{
	RECT rect;
	if (GetWindowRect(windowHandle, &rect))
		return { static_cast<int>(rect.left),static_cast<int>(rect.top), static_cast<int>(rect.right), static_cast<int>(rect.bottom) };

	return {};
}

bool Window::SizeChanged() const noexcept
{
	return sizeChanged;
}

//bool Window::KeyDown(Keys key) const noexcept
//{
//	return m_keys[static_cast<int>(key)];
//}
//
//bool Window::KeyUp(Keys key) const noexcept
//{
//	return !m_keys[static_cast<int>(key)];
//}

Window::Window(HINSTANCE hInstance, int nCmdShow, int width, int height) : windowHandle(nullptr), width(width), height(height), sizeChanged(false)
{
	// Static instance to handle window callbacks
	if (instance)
		throw std::exception("Window already created");

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CallbackWrapper;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"DA307A_eduRend";
	wcex.hIconSm = 0;

	if (!RegisterClassEx(&wcex))
		throw std::exception("Class creation failed");

	// Adjust and create window
	RECT rc = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	if (!(windowHandle = CreateWindow(
		L"DA307A_eduRend",
		L"DA307A - eduRend",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr)))
	{
		throw std::exception("Window creation failed");
	}

	keys.reset();
	instance = this;

	ShowWindow(windowHandle, nCmdShow);
}

Window::~Window() noexcept
{
	ReleaseCapture();
	instance = nullptr;
}

LRESULT Window::WindowCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		keys.set(wParam);
		break;
	case WM_KEYUP:
		keys.reset(wParam);
		break;

	case WM_SIZE:
		width = static_cast<size_t>(LOWORD(lParam));
		height = static_cast<size_t>(HIWORD(lParam));
		sizeChanged = true;
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

LRESULT Window::CallbackWrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (instance)
		return instance->WindowCallback(hWnd, message, wParam, lParam);

	return DefWindowProc(hWnd, message, wParam, lParam);
}
