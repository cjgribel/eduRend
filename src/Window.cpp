//
// Win32 Window class
//
// Written by Oliver Öhrström
//

#include "Window.h"
#include <windowsx.h>

Window* Window::s_instance = nullptr;

bool Window::Update() noexcept
{
	m_sizeChanged = false;
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
	return m_windowHandle;
}

linalg::vec2i Window::GetSize() const noexcept
{
	return { static_cast<int>(m_width), static_cast<int>(m_height) };
}

Rect Window::GetBounds() const noexcept
{
	RECT rect;
	if (GetWindowRect(m_windowHandle, &rect))
	{
		return { static_cast<int>(rect.left),static_cast<int>(rect.top), static_cast<int>(rect.right), static_cast<int>(rect.bottom) };
	}
	return {};
}

bool Window::SizeChanged() const noexcept
{
	return m_sizeChanged;
}

Window::Window(HINSTANCE instance, int nCmdShow, int width, int height) : m_windowHandle(nullptr), m_width(width), m_height(height), m_sizeChanged(false)
{
	// Static instance to handle window callbacks
	if (s_instance)
	{
		throw std::exception("Window already created");
	}
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CallbackWrapper;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"DA307A_eduRend";
	wcex.hIconSm = 0;
	if (!RegisterClassEx(&wcex))
	{
		throw std::exception("Class creation failed");
	}

	// Adjust and create window
	RECT rc = { 0, 0, (LONG)m_width, (LONG)m_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	m_windowHandle = CreateWindow(
		L"DA307A_eduRend",
		L"DA307A - eduRend",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		instance,
		nullptr);

	if (!m_windowHandle)
	{
		throw std::exception("Window creation failed");
	}

	s_instance = this;

	ShowWindow(m_windowHandle, nCmdShow);
}

Window::~Window() noexcept
{
	DestroyWindow(m_windowHandle);
	s_instance = nullptr;
}

LRESULT Window::WindowCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		m_width = static_cast<size_t>(LOWORD(lParam));
		m_height = static_cast<size_t>(HIWORD(lParam));
		m_sizeChanged = true;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

LRESULT Window::CallbackWrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (s_instance)
	{
		return s_instance->WindowCallback(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
