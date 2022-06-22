#include "InputHandler.h"

InputHandler::InputHandler(InputHandler&& other) noexcept : InputHandler()
{
	*this = std::move(other);
}

InputHandler& InputHandler::operator=(InputHandler&& other) noexcept
{
	if (this == &other) return *this;

	std::swap(m_direct_input, other.m_direct_input);
	std::swap(m_keyboard, other.m_keyboard);
	std::swap(m_mouse, other.m_mouse);
	std::swap(m_keyboard_state, other.m_keyboard_state);
	std::swap(m_mouse_state, other.m_mouse_state);
	std::swap(m_previous_mouse_state, other.m_previous_mouse_state);
	std::swap(m_direct_input, other.m_direct_input);
	std::swap(m_screen_width, other.m_screen_width);
	std::swap(m_screen_height, other.m_screen_height);
	std::swap(m_mouse_x, other.m_mouse_x);

	return *this;
}

bool InputHandler::Initialize(HINSTANCE hInstance, HWND hWnd, int screenWidth, int screenHeight) noexcept
{
	m_screen_height = screenHeight;
	m_screen_width = screenWidth;
	m_mouse_x = 0;
	m_mouse_y = 0;
	HRESULT result;
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_direct_input, nullptr);
	if (FAILED(result)){
		return false;
	}

	result = m_direct_input->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(result))
	{
		return false;
	}

	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	result = m_keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	result = m_direct_input->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	result = m_mouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// These not being aquired is a valid result and happens if the window starts minimized.
	result = m_keyboard->Acquire();
	result = m_mouse->Acquire();

	return true;
}

void InputHandler::Shutdown() noexcept
{
	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
	}

	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
	}

	if (m_direct_input)
	{
		m_direct_input->Release();
	}

	return;
}

bool InputHandler::Update() noexcept
{
	bool result;

	result = ReadKeyboard();
	if (!result)
	{
		return false;
	}

	result = ReadMouse();
	if (!result)
	{
		return false;
	}

	ProcessInput();

	return true;
}

void InputHandler::GetMouseLocation(int& mouseX, int& mouseY) const noexcept
{
	mouseX = m_mouse_x;
	mouseY = m_mouse_y;
}

bool InputHandler::IsKeyPressed(Keys key) const noexcept
{
	return m_keyboard_state[(int)key] & 0x80;
}

LONG InputHandler::GetMouseDeltaX() const noexcept
{
	return m_mouse_state.lX;
}

LONG InputHandler::GetMouseDeltaY() const noexcept
{
	return m_mouse_state.lY;
}

bool InputHandler::ReadKeyboard() noexcept
{
	HRESULT result;

	result = m_keyboard->GetDeviceState(sizeof(m_keyboard_state), (LPVOID)&m_keyboard_state);
	if (FAILED(result))
	{
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_keyboard->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool InputHandler::ReadMouse() noexcept
{
	HRESULT result;
	m_previous_mouse_state = m_mouse_state;
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouse_state);
	if (FAILED(result))
	{
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

void InputHandler::ProcessInput() noexcept
{
	m_mouse_x += m_mouse_state.lX;
	m_mouse_y += m_mouse_state.lY;
}