#pragma once

namespace Collision
{
	inline bool IsCollidedSqureWithPoint(const RECT rect, const D2D1_POINT_2F point);
	inline bool IsCollidedCircleWithPoint(const D2D1_POINT_2F center, const float radius, const D2D1_POINT_2F point);
	inline bool DoLinesIntersect(Line line0, Line line1);

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

	bool DoLinesIntersect(Line line0, Line line1)
	{
		float ccwProductFromLine0 = Math::CrossProduct2D(line0.Point0, line0.Point1, line1.Point0) * Math::CrossProduct2D(line0.Point0, line0.Point1, line1.Point1);
		float ccwProductFromLine1 = Math::CrossProduct2D(line1.Point0, line1.Point1, line0.Point0) * Math::CrossProduct2D(line1.Point0, line1.Point1, line0.Point1);

		bool isCollision = false;

		if (std::abs(ccwProductFromLine0) < FLT_EPSILON and std::abs(ccwProductFromLine1) < FLT_EPSILON)
		{
			if (std::make_pair(line0.Point0.x, line0.Point0.y) > std::make_pair(line0.Point1.x, line0.Point1.y))
			{
				std::swap(line0.Point0, line0.Point1);
			}

			if (std::make_pair(line1.Point0.x, line1.Point0.y) > std::make_pair(line1.Point1.x, line1.Point1.y))
			{
				std::swap(line1.Point0, line1.Point1);
			}

			isCollision = std::make_pair(line1.Point0.x, line1.Point0.y) <= std::make_pair(line0.Point1.x, line0.Point1.y)
				and std::make_pair(line0.Point0.x, line0.Point0.y) <= std::make_pair(line1.Point1.x, line1.Point1.y);
		}
		else
		{
			isCollision = ccwProductFromLine0 <= FLT_EPSILON && ccwProductFromLine1 <= FLT_EPSILON;
		}
	}
}