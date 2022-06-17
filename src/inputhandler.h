/**
 * @file inputhandler.h
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
		: m_direct_input(nullptr), m_keyboard(nullptr), m_mouse(nullptr), m_keyboard_state(), m_mouse_state(), m_previous_mouse_state(), m_screen_width(0), m_screen_height(0), m_mouse_x(0), m_mouse_y(0) {}

	/**
	 * @brief Destructor, does nothing, see Shutdown()
	*/
	~InputHandler() noexcept {};

	/**
	 * @brief Move constructor
	*/
	InputHandler(InputHandler&& other) noexcept;

	/**
	 * @brief Deleted, DirectInput handles can't be copied.
	*/
	InputHandler(const InputHandler&) = delete;

	/**
	 * @brief Move assignment
	*/
	InputHandler& operator=(InputHandler&& other) noexcept;

	/**
	 * @brief Deleted, DirectInput handles can't be copied.
	*/
	InputHandler& operator=(const InputHandler&) = delete;

	/**
	 * @brief Initialized the input handler.
	 * @param[in] instance Processor Instance to use.
	 * @param[in] window Window handle to recieve inputs from.
	 * @param[in] screen_width Width of the window.
	 * @param[in] screen_height Height of the window.
	 * @return True if the initialization was successful.
	*/
	bool Initialize(HINSTANCE instance, HWND window, int screen_width, int screen_height) noexcept;

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
	 * @param[out] mouse_x Will be set to the X coordinate of the mouse. 
	 * @param[out] mouse_y Will be set to the Y coordinate of the mouse. 
	*/
	void GetMouseLocation(int& mouse_x, int& mouse_y) const noexcept;

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
	IDirectInput8* m_direct_input;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;
	unsigned char m_keyboard_state[256];
	DIMOUSESTATE m_mouse_state, m_previous_mouse_state;
	int m_screen_width, m_screen_height;
	int m_mouse_x, m_mouse_y;

	bool ReadKeyboard() noexcept;
	bool ReadMouse() noexcept;
	void ProcessInput() noexcept;
};