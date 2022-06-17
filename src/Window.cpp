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

bool Window::Init(uint16_t width, uint16_t height) noexcept
{
	m_width = width;
	m_height = height;
	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CallbackWrapper;
	wcex.hInstance = GetModuleHandle(0);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"DA307A_eduRend";
	if (!RegisterClassEx(&wcex))
	{
		OutputDebugString(L"Window class creation failed");
		return false;
	}

	// Adjust and create window
	RECT rc = { 0, 0, (LONG)m_width, (LONG)m_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	if (!(m_windowHandle = CreateWindow(
		L"DA307A_eduRend",
		L"DA307A - eduRend",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		wcex.hInstance,
		nullptr)))
	{
		OutputDebugString(L"Window creation failed");
		return false;
	}
	s_instance = this;

	ShowWindow(m_windowHandle, SW_SHOW);
	return true;
}

void Window::Shutdown() noexcept
{
	s_instance = nullptr;

	DestroyWindow(m_windowHandle);

	ReleaseCapture();
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
