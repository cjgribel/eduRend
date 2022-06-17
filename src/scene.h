/**
 * @file Scene.h
 * @brief Contains scene related classes
*/

#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "stdafx.h"
#include "InputHandler.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

/**
 * @brief Abstract class defining scene rendering and updating.
*/
class Scene
{
public:
	/**
	 * @brief Setup for member variables, no initialization is done here.
	 * @note These params are saved in the scene so they must be valid for as long as the scene is.
	 * @param[in] dxdevice ID3D11Device that will be used in the scene.
	 * @param[in] dxdevice_context ID3D11DeviceContext that will be used in the scene.
	 * @param[in] window_width Window hight for the scene.
	 * @param[in] window_height Window width for the scene.
	*/
	Scene(
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context,
		int window_width,
		int window_height);

	/**
	 * @brief Initialize all scene data.
	*/
	virtual void Init() = 0;

	/**
	 * @brief Relese all scene data created in Init()
	*/
	virtual void Release() = 0;

	/**
	 * @brief Update any relevant scene data.
	 * @param[in] dt Seconds since the last call.
	 * @param[in] input_handler Reference to the current InputHandler.
	*/
	virtual void Update(float dt, const InputHandler& input_handler) = 0;
	
	/**
	 * @brief Render the scene.
	*/
	virtual void Render() = 0;
	

	/**
	 * @brief Method called whenever the Window has changed size.
	 * @param[in] window_width New window width.
	 * @param[in] window_height New window height.
	*/
	virtual void WindowResize(int window_width,	int window_height);

protected:
	ID3D11Device* dxdevice;
	ID3D11DeviceContext* dxdevice_context;
	int						window_width;
	int						window_height;

};

/**
 * @brief Test scene used in the project.
*/
class OurTestScene : public Scene
{
	//
	// Constant buffers (CBuffers) for data that is sent to shaders
	//

	// CBuffer for transformation matrices
	ID3D11Buffer* transformation_buffer = nullptr;
	// + other CBuffers

	// 
	// CBuffer client-side definitions
	// These must match the corresponding shader definitions 
	//

	struct TransformationBuffer
	{
		mat4f ModelToWorldMatrix;
		mat4f WorldToViewMatrix;
		mat4f ProjectionMatrix;
	};

	//
	// Scene content
	//
	Camera* camera;

	Model* quad;
	Model* sponza;

	// Model-to-world transformation matrices
	mat4f Msponza;
	mat4f Mquad;

	// World-to-view matrix
	mat4f Mview;
	// Projection matrix
	mat4f Mproj;

	// Misc
	float angle = 0;			// A per-frame updated rotation angle (radians)...
	float angle_vel = fPI / 2;	// ...and its velocity (radians/sec)
	float camera_vel = 5.0f;	// Camera movement velocity in units/s
	float fps_cooldown = 0;

	void InitTransformationBuffer();

	void UpdateTransformationBuffer(
		mat4f ModelToWorldMatrix,
		mat4f WorldToViewMatrix,
		mat4f ProjectionMatrix);

public:
	OurTestScene(
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context,
		int window_width,
		int window_height);

	/**
	 * @brief Initializes all resources held by the scene.
	*/
	void Init() override;

	/**
	 * @brief Updates all ojects in the scene
	 * @param dt Time in seconds since last iteration
	 * @param input_handler Current InputHandler
	*/
	void Update(float dt, const InputHandler& input_handler) override;

	/**
	 * @brief Renders all objects in the scene
	*/
	void Render() override;

	/**
	 * @brief Releases all resources created by the scene.
	*/
	void Release() override;

	/**
	 * @brief Updates all scene data that relates to Window size
	 * @param window_width New width
	 * @param window_height New height
	*/
	void WindowResize(int window_width,	int window_height) override;
};

#endif