#pragma once

class Camera final
{
public:
	Camera() = default;
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	[[nodiscard]] D2D1_POINT_2F GetPosition() const;
	void SetPosition(const D2D1_POINT_2F position);

	[[nodiscard]] float GetAngle() const;
	void SetAngle(const float angle);

	[[nodiscard]] float GetFieldOfView() const;
	void SetFieldOfView(const float fieldOfView);

private:
	D2D1_POINT_2F mPosition{};
	float mAngle = 0.0f;
	float mFieldOfView = 1.0f;
};