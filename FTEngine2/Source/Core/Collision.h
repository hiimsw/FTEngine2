#pragma once

namespace Collision
{
	inline bool IsCollidedSqureWithPoint(const RECT rect, const D2D1_POINT_2F point);
	inline bool IsCollidedCircleWithPoint(const D2D1_POINT_2F center, const float radius, const D2D1_POINT_2F point);

	bool IsCollidedSqureWithPoint(const RECT rect, const D2D1_POINT_2F point)
	{
		bool result = rect.left <= point.x and point.x <= rect.right
			and rect.bottom <= point.y and point.y <= rect.top;

		return result;
	}

	bool IsCollidedCircleWithPoint(const D2D1_POINT_2F center, const float radius, const D2D1_POINT_2F point)
	{
		D2D1_POINT_2F centerToPoint = Math::SubtractVector(center, point);
		float centerToPointLength = Math::GetVectorLength(centerToPoint);
		bool result = centerToPointLength <= radius;

		return result;
	}
}