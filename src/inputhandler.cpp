#include "InputHandler.h"

InputHandler::InputHandler(InputHandler&& other) noexcept : InputHandler()
{
	*this = std::move(other);
}

InputHandler& InputHandler::operator=(InputHandler&& other) noexcept
{
	if (this == &other) return *this;

	std::swap(m_directInput, other.m_directInput);
	std::swap(m_keyboard, other.m_keyboard);
	std::swap(m_mouse, other.m_mouse);
	std::swap(m_keyboardState, other.m_keyboardState);
	std::swap(m_mouseState, other.m_mouseState);
	std::swap(m_prevMouseState, other.m_prevMouseState);
	std::swap(m_directInput, other.m_directInput);
	std::swap(m_screenWidth, other.m_screenWidth);
	std::swap(m_screenHeight, other.m_screenHeight);
	std::swap(m_mouseX, other.m_mouseX);

	return *this;
}

bool InputHandler::Initialize(HINSTANCE hInstance, HWND hWnd, int screenWidth, int screenHeight) noexcept
{
	m_screenHeight = screenHeight;
	m_screenWidth = screenWidth;
	m_mouseX = 0;
	m_mouseY = 0;
	HRESULT result;
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, nullptr);
	if (FAILED(result)){
		return false;
	}

	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
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

	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
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

	if (m_directInput)
	{
		m_directInput->Release();
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
	mouseX = this->m_mouseX;
	mouseY = this->m_mouseY;
}

bool InputHandler::IsKeyPressed(Keys key) const noexcept
{
	if (m_keyboardState[(int)key] & 0x80)
	{
		return true;
	}
	return false;
}

LONG InputHandler::GetMouseDeltaX() const noexcept
{
	return m_mouseState.lX;
}

LONG InputHandler::GetMouseDeltaY() const noexcept
{
	return m_mouseState.lY;
}

bool InputHandler::ReadKeyboard() noexcept
{
	HRESULT result;

	result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
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
	m_prevMouseState = m_mouseState;
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
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
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;
}