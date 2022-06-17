/**
 * @file Camera.h
 * @brief Basic camera class
*/

#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "vec\vec.h"
#include "vec\mat.h"

/**
 * @brief Manages camera data, also handles generation of view and projection matrices.
*/
class Camera
{
public:
	/**
	 * @brief Creates a camera.
	 * @param[in] vfov Vertical field of view.
	 * @param[in] aspect Aspect ratio, calculate from screen width / sceen height.
	 * @param[in] zNear Near plane distance.
	 * @param[in] zFar Far plane distance, must be larger than the near plane.
	*/
	inline constexpr Camera(float vfov, float aspect, float zNear, float zFar) noexcept 
		: m_vfov(vfov), m_aspect(aspect), m_zNear(zNear), m_zFar(zFar), m_position(0.0f) {}

	/**
	 * @brief Move the camera to a new position
	 * @param[in] p New position for the camera
	*/
	void MoveTo(const linalg::vec3f& p) noexcept;

	/**
	 * @brief Move the camera along a vector
	 * @param[in] v vector to move along
	*/
	void Move(const linalg::vec3f& v) noexcept;

	/**
	 * @brief Changes the camera aspect ratio.
	 * @param[in] aspect New aspect ratio, calculate with width / height
	*/
	inline void SetAspect(float aspect) noexcept { m_aspect = aspect; }

	/**
	 * @brief Get the World-to-View matrix of the camera.
	 * @return World-to-View matrix.
	*/
	linalg::mat4f WorldToViewMatrix() const noexcept;

	/**
	 * @brief get the Matrix transforming from View space to Clip space
	 * @return Projection matrix.
	 * 
	 * @note In a performance sensitive situation this matrix should be precomputed if possible
	*/
	linalg::mat4f ProjectionMatrix() const noexcept;

private:
	// Aperture attributes
	float m_vfov;
	float m_aspect;

	// Clip planes in view space coordinates
	// Evrything outside of [zNear, zFar] is clipped away on the GPU side
	// zNear should be > 0
	// zFar should depend on the size of the scene
	// This range should be kept as tight as possibly to improve
	// numerical precision in the z-buffer
	float m_zNear;
	float m_zFar;

	linalg::vec3f m_position;
};

#endif