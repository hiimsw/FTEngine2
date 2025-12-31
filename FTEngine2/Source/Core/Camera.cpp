#include "pch.h"
#include "Camera.h"

D2D1_POINT_2F Camera::GetPosition() const
{
	return mPosition;
}

void Camera::SetPosition(const D2D1_POINT_2F position)
{
	mPosition = position;
}

float Camera::GetAngle() const
{
	return mAngle;
}

void Camera::SetAngle(const float angle)
{
	mAngle = angle;
}

float Camera::GetFieldOfView() const
{
	return mFieldOfView;
}

void Camera::SetFieldOfView(const float fieldOfView)
{
	mFieldOfView = fieldOfView;
}
