
//
//  Camera.h
//
//	Basic camera class
//

#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "vec\Vec.h"
#include "vec\Mat.h"

using namespace linalg;

class Camera
{
public:

	// Aperture attributes
	float vfov, aspect;	
	
	// Clip planes in view space coordinates
	// Evrything outside of [zNear, zFar] is clipped away on the GPU side
	// zNear should be > 0
	// zFar should depend on the size of the scene
	// This range should be kept as tight as possibly to improve
	// numerical precision in the z-buffer

	float zNear, zFar;	
						
	Vec3f position;
	float speed; // Movement speed in units/s

	Camera(float vfov, float aspect, float zNear, float zFar, float speed):
		vfov(vfov), aspect(aspect), zNear(zNear), zFar(zFar), speed(speed)
	{
		position = { 0.0f, 0.0f, 0.0f };
	}

	// Move to an absolute position
	void MoveTo(const Vec3f& position)
	{
		this->position = position;
	}

	// Move relatively
	void MoveBy(const Vec3f& amount)
	{
		position += amount;
	}

	void Update(InputHandler* inputHandler, float deltaTime)
	{
		// Basic camera control

		if (inputHandler->IsKeyPressed(Keys::Up) || inputHandler->IsKeyPressed(Keys::W))
			position.z -= speed * deltaTime;

		if (inputHandler->IsKeyPressed(Keys::Down) || inputHandler->IsKeyPressed(Keys::S))
			position.z += speed * deltaTime;

		if (inputHandler->IsKeyPressed(Keys::Left) || inputHandler->IsKeyPressed(Keys::A))
			position.x -= speed * deltaTime;

		if (inputHandler->IsKeyPressed(Keys::Right) || inputHandler->IsKeyPressed(Keys::D))
			position.x += speed * deltaTime;
	}

	// Return World-to-View matrix for this camera
	Mat4f Get_WorldToViewMatrix()
	{
		// Assuming a camera's position and rotation is defined by matrices T(p) and R,
		// the View-to-World transform is T(p)*R (for a first-person style camera).
		//
		// World-to-View then is the inverse of T(p)*R;
		//		inverse(T(p)*R) = inverse(R)*inverse(T(p)) = transpose(R)*T(-p)
		// Since now there is no rotation, this matrix is simply T(-p)

		return Mat4f::Translation(-position);
	}

	// Matrix transforming from View space to Clip space
	// In a performance sensitive situation this matrix should be precomputed
	// if possible
	Mat4f Get_ProjectionMatrix()
	{
		return Mat4f::Projection(vfov, aspect, zNear, zFar);
	}
};

#endif