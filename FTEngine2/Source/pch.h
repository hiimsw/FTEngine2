#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <d2d1.h>
#include <dwrite.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <tchar.h>
#include <unordered_map>
#include <wincodec.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define RELEASE(x) \
if ((x) != nullptr) \
{ \
	delete (x); \
	(x) = nullptr; \
} \
void(0)

#define RELEASE_ARRAY(x) \
if ((x) != nullptr) \
{ \
	delete[] (x); \
	(x) = nullptr; \
} \
void(0)

#define RELEASE_D2D1(x) \
if ((x) != nullptr) \
{ \
	(x)->Release(); \
	(x) = nullptr; \
} \
void(0)

#define LOG(format, ...) \
{ \
	char log[256]{}; \
	sprintf_s(log, sizeof(log), "%s(%d): " format "\n", strstr(__FILE__, "Source"), __LINE__, __VA_ARGS__); \
	printf(log); \
	OutputDebugStringA(log); \
} \
void(0)

#define LOG_W(format, ...) \
{ \
	wchar_t log[256]{}; \
	swprintf_s(log, sizeof(log), "%s(%d): " format "\n", strstr(__FILE__, "Source"), __LINE__, __VA_ARGS__); \
	wprintf(log); \
	OutputDebugStringW(log); \
} \
void(0)

#if defined(_DEBUG)

#define DEBUG_LOG(format, ...) LOG(format, __VA_ARGS__)

#define DEBUG_BREAK() __debugbreak()

#define ASSERT(expr) \
if (not (expr)) \
{ \
	DEBUG_LOG("assert(" #expr ")"); \
	DEBUG_BREAK(); \
} \
void(0)

#define MASSERT(expr, format, ...) \
if (not (expr)) \
{ \
	DEBUG_LOG("assert(" #expr ", " format ")", __VA_ARGS__); \
	DEBUG_BREAK(); \
} \
void(0)

#define WARN(expr) \
if (not (expr)) \
{ \
	DEBUG_LOG("warning(" #expr ")"); \
} \
void(0)

#define MWARN(expr, format, ...) \
if (not (expr)) \
{ \
	DEBUG_LOG("warning(" #expr ", " format ")", __VA_ARGS__); \
} \
void(0)

#if !defined(HR)
#define HR(x) \
	{ \
		HRESULT hr = (x); \
		if (FAILED(hr)) \
		{ \
			LPVOID errorLog = nullptr; \
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, \
				nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), \
				reinterpret_cast<LPSTR>(&errorLog), 0, nullptr); \
			LOG("%s", static_cast<char*>(errorLog)); \
			LocalFree(errorLog); \
			DEBUG_BREAK(); \
		} \
	} \
	((void)0)
#endif

#else

#define DEBUG_LOG(...) ((void)0)
#define DEBUG_LOG_W(...) ((void)0)
#define DEBUG_BREAK() ((void)0)

#define ASSERT(expr) ((void)0)
#define MASSERT(expr, msg) ((void)0)

#define WARN(expr) ((void)0)
#define MWARN(expr, msg) ((void)0)

#if !defined(HR)
#define HR(x) (x)
#endif

#endif

struct Line
{
	D2D1_POINT_2F Point0;
	D2D1_POINT_2F Point1;
};

namespace Math
{
	inline D2D1_POINT_2F AddVector(const D2D1_POINT_2F lhs, const D2D1_POINT_2F rhs);
	inline D2D1_POINT_2F SubtractVector(const D2D1_POINT_2F lhs, const D2D1_POINT_2F rhs);
	inline D2D1_POINT_2F ScaleVector(const D2D1_POINT_2F vector, const float scalar);
	inline float GetVectorLength(const D2D1_POINT_2F vector);
	inline D2D1_POINT_2F LerpVector(const D2D1_POINT_2F from, const D2D1_POINT_2F to, const float t);
	inline D2D1_POINT_2F GetNormalizeVector(const D2D1_POINT_2F vector);
	inline float CrossProduct2D(const D2D1_POINT_2F point0, const D2D1_POINT_2F point1, const D2D1_POINT_2F point2);

	D2D1_POINT_2F AddVector(const D2D1_POINT_2F lhs, const D2D1_POINT_2F rhs)
	{
		D2D1_POINT_2F result{ .x = lhs.x + rhs.x, .y = lhs.y + rhs.y };
		return result;
	}

	D2D1_POINT_2F SubtractVector(const D2D1_POINT_2F lhs, const D2D1_POINT_2F rhs)
	{
		D2D1_POINT_2F result{ .x = lhs.x - rhs.x, .y = lhs.y - rhs.y };
		return result;
	}

	D2D1_POINT_2F ScaleVector(const D2D1_POINT_2F vector, const float scalar)
	{
		D2D1_POINT_2F result{ .x = vector.x * scalar, .y = vector.y * scalar };
		return result;
	}

	float GetVectorLength(const D2D1_POINT_2F vector)
	{
		float lenght = sqrt(vector.x * vector.x + vector.y * vector.y);
		return lenght;
	}

	D2D1_POINT_2F LerpVector(const D2D1_POINT_2F a, const D2D1_POINT_2F b, const float t)
	{
		D2D1_POINT_2F ab = SubtractVector(b, a);
		D2D1_POINT_2F result = AddVector(a, ScaleVector(ab, t));
		return result;
	}

	D2D1_POINT_2F GetNormalizeVector(const D2D1_POINT_2F vector)
	{
		float lenght = GetVectorLength(vector);
		D2D1_POINT_2F result{ .x = vector.x / lenght, .y = vector.y / lenght };
		return result;
	}

	float CrossProduct2D(const D2D1_POINT_2F point0, const D2D1_POINT_2F point1, const D2D1_POINT_2F point2)
	{
		float ccw = (point1.x - point0.x) * (point2.y - point0.y) - (point2.x - point0.x) * (point1.y - point0.y);
		return ccw;
	}
}
