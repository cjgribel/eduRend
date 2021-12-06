
#pragma once
#ifndef PROGRAM_H
#define PROGRAM_H

#include "stdafx.h"
#include "InputHandler.h"
#include "Camera.h"
#include "Geometry.h"

// New files
// Material
// Texture

class Scene
{
private:
	ID3D11Device*			dxdevice;
	ID3D11DeviceContext*	dxdevice_context;
	int						window_width;
	int						window_height;

public:

	Scene(
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context,
		int window_width,
		int window_height);

	void Init();

	void Update(
		float dt,
		InputHandler* input_handler);
	
	void Render();
	
	void Release();
	
	void WindowResize(
		int window_width,
		int window_height);
};






//
// Called at initialization
//
void initObjects(
	unsigned window_width,
	unsigned window_height,
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context);

//
// Called every frame
//
void updateObjects(
	float dt, 
	InputHandler* input_handler);

//
// Called every frame, after update
//
void renderObjects(
	ID3D11Buffer* matrix_buffer);

//
// Called when window is resized
//
void WindowResize(int width, int height);

//
// Called at termination
//
void releaseObjects();

#endif