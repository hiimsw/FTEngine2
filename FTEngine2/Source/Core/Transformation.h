#pragma once

#include "Constant.h"

namespace Transformation
{
	using namespace D2D1;

	[[nodiscard]] inline Matrix3x2F getWorldMatrix(const D2D1_POINT_2F position = { .x = 0.0f, .y = 0.0f }, const float angle = 0.0f, const D2D1_SIZE_F scale = { .width = 1.0f, .height = 1.0f });

	Matrix3x2F Transformation::getWorldMatrix(const D2D1_POINT_2F position, const float angle, const D2D1_SIZE_F scale)
	{
		Matrix3x2F world = Matrix3x2F::Scale(scale)
			* Matrix3x2F::Rotation(angle)
			* Matrix3x2F::Translation(position.x, Constant::Get().GetHeight() - position.y - 1.0f);

		return world;
	}
}