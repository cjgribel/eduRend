
//
//	Mouse & keyboard via DirectInput
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//  Contributions by Alexander Baldwin, alexander.baldwin@mah.se
//

#pragma once
#include "stdafx.h"

enum Keys
{
	Left = DIK_LEFT,
	Right = DIK_RIGHT,
	Up = DIK_UP,
	Down = DIK_DOWN,
	W = DIK_W,
	A = DIK_A,
	S = DIK_S,
	D = DIK_D
};

class InputHandler 
{
private:

	IDirectInput8* directInput;
	IDirectInputDevice8* keyboard;
	IDirectInputDevice8* mouse;

	unsigned char keyboardState[256];
	DIMOUSESTATE mouseState, prevMouseState;
	int screenWidth, screenHeight;
	int mouseX, mouseY;

	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

public:

	InputHandler();
	~InputHandler();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Update();
	void GetMouseLocation(int&, int&);
	bool IsKeyPressed(Keys);
	LONG GetMouseDeltaX();
	LONG GetMouseDeltaY();
};