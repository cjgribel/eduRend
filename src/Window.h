/**
 * @file Window.h
 * @author Oliver Öhrström
 * @brief Contains the class Window that handles windowing functionality for the WIN32 platform.
 * @copyright MIT License.
 */

#pragma once
#include "stdafx.h"
#include "vec/vec.h"

/**
 * @brief Hold the upper and lower corner coordinates of a rectangle.
*/
struct Rect
{
	union
	{
		struct
		{
			int XMin;
			int YMin;
			int XMax;
			int YMax;
		};
		struct
		{
			linalg::int2 LowerBounds;
			linalg::int2 UpperBounds;
		};
	};
};

/**
 * @brief Handles creation, callbacks, and resizing of a WIN32 window.
*/
class Window
{
public:
	/**
	 * @brief Creates a windows window.
	 * @param instance Process instance that the window should be created with.
	 * @param nCmdShow The show command to pass to the window.
	 * @param width Initial width of the window.
	 * @param height Initial height of the window.
	 * 
	 * @exception Throws "Window already created" exception if a window already exists.
	 * @exception Throws "Class creation failed" exception if the window class registration fails.
	 * @exception Throws "Window creation failed" exception if the win32 window creation fails.
	*/
	Window(HINSTANCE instance, int nCmdShow, int width = 728, int height = 728);

	/**
	* @brief Destroys the window.
	*/
	~Window() noexcept;

	/**
	 * @brief Processes all window events since last call to this function.
	 * @return False if the events included a close event, otherwise returns True.
	*/
	bool Update() noexcept;

	/**
	 * @brief Returns the win32 handle of the window.
	 * @return Win32 Handle of the window.
	*/
	HWND GetHandle() noexcept;

	/**
	 * @brief Gets the currect size of the window.
	 * @return linalg::vec2i containing the Width and Height of the window.
	*/
	linalg::vec2i GetSize() const noexcept;

	/**
	 * @brief Gets the current window bounds.
	 * @return Rect containing the upper and lower bounds of the window.
	*/
	Rect GetBounds() const noexcept;

	/**
	 * @brief Check if the size of the window has changed during the latest Update
	 * @return True if the size was changed.
	*/
	bool SizeChanged() const noexcept;

private:
	LRESULT CALLBACK WindowCallback(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK CallbackWrapper(HWND, UINT, WPARAM, LPARAM);

private:
	static Window* s_instance;
	HWND m_windowHandle;
	size_t m_width;
	size_t m_height;
	bool m_sizeChanged;
};