
#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "stdafx.h"
#include "InputHandler.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

// New files
// Material
// Texture <- stb

// TEMP

class Scene
{
protected:

	ID3D11Device*			dxDevice;
	ID3D11DeviceContext*	dxDeviceContext;
	int						windowWidth;
	int						windowHeight;

public:

	Scene(
		ID3D11Device* dxDevice,
		ID3D11DeviceContext* dxDeviceContext,
		int windowWidth,
		int windowHeight);

	virtual void Init() = 0;

	virtual void Update(float deltaTime, InputHandler* inputHandler) = 0;
	
	virtual void Render() = 0;
	
	virtual void Release() = 0;

	virtual void ResizeWindow(int width, int height);

};

class OurTestScene : public Scene
{
	//
	// Constant buffers (CBuffers) for data that is sent to shaders
	//

	// CBuffer for transformation matrices
	ID3D11Buffer* transformationBuffer = nullptr;
	// + other CBuffers

	//
	// Scene content
	//

	Camera* camera;
	
	Mat4f worldToViewMatrix;
	Mat4f projectionMatrix;

	// Game objects and their model-to-world transformation matrices

	QuadModel* quad;
	Mat4f mtwQuad;

	OBJModel* sponza;
	Mat4f mtwSponza;

	//
	// Misc
	//

	float angle = 0;			     // A per-frame updated rotation angle (radians)...
	float angularVelocity = fPI / 2; // ...and its velocity (radians/sec)
	float fpsCooldown = 0;

	void InitTransformationBuffer();

	void UpdateTransformationBuffer(
		Mat4f modelToWorldMatrix,
		Mat4f worldToViewMatrix,
		Mat4f projectionMatrix);

public:

	OurTestScene(
		ID3D11Device* dxDevice,
		ID3D11DeviceContext* dxDeviceContext,
		int windowWidth,
		int windowHeight);

	void Init() override;

	void Update(float dt, InputHandler* inputHandler) override;

	void Render() override;

	void Release() override;

	void ResizeWindow(int width, int height) override;

};

#endif