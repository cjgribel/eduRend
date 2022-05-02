
//
//	Mouse & keyboard via DirectInput
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//  Contributions by Alexander Baldwin, alexander.baldwin@mah.se
//

#include "InputHandler.h"

bool InputHandler::ReadKeyboard(){
	HRESULT result;

	result = keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	if (FAILED(result))
	{
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			keyboard->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool InputHandler::ReadMouse(){
	HRESULT result;
	prevMouseState = mouseState;
	result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouseState);
	if (FAILED(result))
	{
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			mouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

void InputHandler::ProcessInput(){
	mouseX += mouseState.lX;
	mouseY += mouseState.lY;
}

InputHandler::InputHandler(){
	mouse = nullptr;
	keyboard = nullptr;
	directInput = nullptr;
}

InputHandler::~InputHandler(){

}

bool InputHandler::Initialize(HINSTANCE hInstance, HWND hWnd, int screenWidth, int screenHeight){
	this->screenHeight = screenHeight;
	this->screenWidth = screenWidth;
	mouseX = 0;
	mouseY = 0;
	HRESULT result;
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	if (FAILED(result)){
		return false;
	}

	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	if (FAILED(result))
	{
		return false;
	}

	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	result = keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	result = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	result = mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	result = mouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// These not being aquired is a valid result and happens if the window starts minimized.
	result = keyboard->Acquire();
	result = mouse->Acquire();

	return true;
}

void InputHandler::Shutdown(){
	if (mouse)
	{
		mouse->Unacquire();
		mouse->Release();
		mouse = nullptr;
	}

	if (keyboard)
	{
		keyboard->Unacquire();
		keyboard->Release();
		keyboard = nullptr;
	}

	if (directInput)
	{
		directInput->Release();
		directInput = nullptr;
	}

	return;
}

bool InputHandler::Update(){
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

void InputHandler::GetMouseLocation(int& mouseX, int& mouseY){
	mouseX = this->mouseX;
	mouseY = this->mouseY;
}

bool InputHandler::IsKeyPressed(Keys key){
	if (keyboardState[key] & 0x80){
		return true;
	}
	return false;
}

LONG InputHandler::GetMouseDeltaX(){
	return mouseState.lX;
}

LONG InputHandler::GetMouseDeltaY(){
	return mouseState.lY;
}