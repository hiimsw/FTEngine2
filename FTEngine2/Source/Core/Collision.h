#pragma once

namespace Collision
{
	inline bool IsCollidedSqureWithPoint(const D2D1_RECT_F rect, const D2D1_POINT_2F point);
	inline bool IsCollidedSqureWithSqure(const D2D1_RECT_F lhs, const D2D1_RECT_F rhs);
	inline bool IsCollidedSqureWithLine(const D2D1_RECT_F rect, const Line line);
	inline bool IsCollidedCircleWithPoint(const D2D1_POINT_2F center, const float radius, const D2D1_POINT_2F point);
	inline bool IsCollidedCircleWithLine(const D2D1_POINT_2F center, const float radius, const Line line);
	inline bool IsCollidedCircleWithCircle(const D2D1_ELLIPSE lhs, const D2D1_ELLIPSE rhs);
	inline bool DoLinesIntersect(Line line0, Line line1);

	bool IsCollidedSqureWithPoint(const D2D1_RECT_F rect, const D2D1_POINT_2F point)
	{
		const bool result = rect.left <= point.x and point.x <= rect.right
			and rect.bottom <= point.y and point.y <= rect.top;

		return result;
	}

	bool IsCollidedSqureWithSqure(const D2D1_RECT_F lhs, const D2D1_RECT_F rhs)
	{
		const bool result = IsCollidedSqureWithPoint(lhs, { .x = float(rhs.left), .y = float(rhs.top) })
			or IsCollidedSqureWithPoint(lhs, { .x = float(rhs.left), .y = float(rhs.bottom) })
			or IsCollidedSqureWithPoint(lhs, { .x = float(rhs.right), .y = float(rhs.top) })
			or IsCollidedSqureWithPoint(lhs, { .x = float(rhs.right), .y = float(rhs.bottom) });

		return result;
	}

	bool IsCollidedSqureWithLine(const D2D1_RECT_F rect, const Line line)
	{
		const Line leftLine = { .Point0 = {.x = rect.left, .y = rect.top }, .Point1 = {.x = rect.left, .y = rect.bottom } };
		const Line rightLine = { .Point0 = {.x = rect.right, .y = rect.top }, .Point1 = {.x = rect.right, .y = rect.bottom } };
		const Line topLine = { .Point0 = {.x = rect.left, .y = rect.top }, .Point1 = {.x = rect.right, .y = rect.top } };
		const Line bottomLine = { .Point0 = {.x = rect.left, .y = rect.bottom }, .Point1 = {.x = rect.right, .y = rect.bottom } };

		const bool result = DoLinesIntersect(leftLine, line)
			or DoLinesIntersect(rightLine, line)
			or DoLinesIntersect(topLine, line)
			or DoLinesIntersect(bottomLine, line);

		return result;
	}

	bool IsCollidedCircleWithPoint(const D2D1_POINT_2F center, const float radius, const D2D1_POINT_2F point)
	{
		const D2D1_POINT_2F centerToPoint = Math::SubtractVector(center, point);
		const float centerToPointLength = Math::GetVectorLength(centerToPoint);
		const bool result = centerToPointLength <= radius;

		return result;
	}

	bool IsCollidedCircleWithLine(const D2D1_POINT_2F center, const float radius, const Line line)
	{
		const D2D1_POINT_2F ac = Math::SubtractVector(center, line.Point0);
		const D2D1_POINT_2F ab = Math::SubtractVector(line.Point1, line.Point0);
		
		const float abLength = Math::GetVectorLength(ab);

		if (abLength == 0.0f)	// 선이 아니라 점일 때
		{
			return Math::GetVectorLength(ac) <= radius;
		}

		const D2D1_POINT_2F abDirection = Math::NormalizeVector(ab);

		float t = Math::DotProduct2D(ac, abDirection);
		t = std::clamp(t, 0.0f, abLength);

		const D2D1_POINT_2F cPoint = 
		{
			line.Point0.x + abDirection.x * t,
			line.Point0.y + abDirection.y * t
		};

		const D2D1_POINT_2F diff = Math::SubtractVector(center, cPoint);
		
		bool result = Math::DotProduct2D(diff, diff) <= radius * radius;

		return result;
	}

	bool IsCollidedCircleWithCircle(const D2D1_ELLIPSE lhs, const D2D1_ELLIPSE rhs)
	{
		const float radius = lhs.radiusX + rhs.radiusX;

		const D2D1_POINT_2F toTarget = Math::SubtractVector(lhs.point, rhs.point);
		const float distance = Math::GetVectorLength(toTarget);

		const bool result = radius >= distance;

		return result;
	}

	bool DoLinesIntersect(Line line0, Line line1)
	{
		const float ccwProductFromLine0 = Math::CrossProduct2D(line0.Point0, line0.Point1, line1.Point0) * Math::CrossProduct2D(line0.Point0, line0.Point1, line1.Point1);
		const float ccwProductFromLine1 = Math::CrossProduct2D(line1.Point0, line1.Point1, line0.Point0) * Math::CrossProduct2D(line1.Point0, line1.Point1, line0.Point1);
		bool bIntersecting = false;

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

			bIntersecting = std::make_pair(line1.Point0.x, line1.Point0.y) <= std::make_pair(line0.Point1.x, line0.Point1.y)
				and std::make_pair(line0.Point0.x, line0.Point0.y) <= std::make_pair(line1.Point1.x, line1.Point1.y);
		}
		else
		{
			bIntersecting = ccwProductFromLine0 <= FLT_EPSILON && ccwProductFromLine1 <= FLT_EPSILON;
		}

		return bIntersecting;
	}
}