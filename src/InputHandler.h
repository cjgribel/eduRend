/**
 * @file InputHandler.h
 * @brief Mouse & keyboard via DirectInput
 * @author Carl Johan Gribel 2016-2021, cjgribel@gmail.com
 * @author Alexander Baldwin, alexander.baldwin@mah.se
 * @author Oliver Öhrström
*/

#pragma once
#include "stdafx.h"

/**
 * @brief Keycodes
*/
enum class Keys 
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

/**
 * @brief Class that handles mouse and keyboard input.
 * @details Uses DirectInput internally.
*/
class InputHandler 
{
public:
	/**
	 * @brief Zero initializes the class, for actual initalization see Initialize()
	 * @see Initialize(HINSTANCE, HWND, int, int)
	*/
	constexpr InputHandler() noexcept 
		: m_directInput(nullptr), m_keyboard(nullptr), m_mouse(nullptr), m_keyboardState(), m_mouseState(), m_prevMouseState(), m_screenWidth(0), m_screenHeight(0), m_mouseX(0), m_mouseY(0) {}

	/**
	 * @brief Destructor, does nothing, see Shutdown()
	*/
	~InputHandler() noexcept {};

	InputHandler(InputHandler&& other) noexcept;

	/**
	 * @brief Deleted, DirectInput handles can't be copied.
	*/
	InputHandler(const InputHandler&) = delete;

	InputHandler& operator=(InputHandler&& other) noexcept;

	/**
	 * @brief Deleted, DirectInput handles can't be copied.
	*/
	InputHandler& operator=(const InputHandler&) = delete;

	/**
	 * @brief Initialized the input handler.
	 * @param[in] hInstance Processor Instance to use.
	 * @param[in] hWnd Window handle to recieve inputs from.
	 * @param[in] screenWidth Width of the window.
	 * @param[in] screenHeight Height of the window.
	 * @return True if the initialization was successful.
	*/
	bool Initialize(HINSTANCE hInstance, HWND hWnd, int screenWidth, int screenHeight) noexcept;

	/**
	 * @brief Releases all input handles used by the class.
	*/
	void Shutdown() noexcept;

	/**
	 * @brief Processes all events that has taken place since the last call.
	 * @return True if the processing worked, False if any error occured.
	*/
	bool Update() noexcept;

	/**
	 * @brief Gets the current X and Y location of the mouse cursor.
	 * @param[out] mouseX Will be set to the X coordinate of the mouse. 
	 * @param[out] mouseY Will be set to the Y coordinate of the mouse. 
	*/
	void GetMouseLocation(int& mouseX, int& mouseY) const noexcept;

	/**
	 * @brief Check if the given key if currently pressed.
	 * @param[in] key Keycode of the key to check @see Keys
	 * @return True if the key is currently held down.
	*/
	bool IsKeyPressed(Keys key) const noexcept;

	/**
	 * @brief Gets the mouse X delta since last Update()
	 * @return Pixels moved in X since last Update()
	*/
	LONG GetMouseDeltaX() const noexcept;

	/**
	 * @brief Gets the mouse Y delta since last Update()
	 * @return Pixels moved in Y since last Update()
	*/
	LONG GetMouseDeltaY() const noexcept;

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;
	unsigned char m_keyboardState[256];
	DIMOUSESTATE m_mouseState, m_prevMouseState;
	int m_screenWidth, m_screenHeight;
	int m_mouseX, m_mouseY;

	bool ReadKeyboard() noexcept;
	bool ReadMouse() noexcept;
	void ProcessInput() noexcept;
};