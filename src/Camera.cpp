#include "Camera.h"

using namespace linalg;

void Camera::MoveTo(const vec3f& p) noexcept
{
	m_position = p;
}

void Camera::Move(const vec3f& v) noexcept
{
	m_position += v;
}

mat4f Camera::WorldToViewMatrix() const noexcept
{
	// Assuming a camera's position and rotation is defined by matrices T(p) and R,
	// the View-to-World transform is T(p)*R (for a first-person style camera).
	//
	// World-to-View then is the inverse of T(p)*R;
	//		inverse(T(p)*R) = inverse(R)*inverse(T(p)) = transpose(R)*T(-p)
	// Since now there is no rotation, this matrix is simply T(-p)

	return mat4f::translation(-m_position);
}

mat4f Camera::ProjectionMatrix() const noexcept
{
	return mat4f::projection(m_vfov, m_aspect, m_zNear, m_zFar);
}