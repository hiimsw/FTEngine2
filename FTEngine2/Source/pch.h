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

constexpr char PROJECT_PATH[] = "C:\\Users\\peach\\github\\FTEngine\\FTEngine\\Source";

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
	sprintf_s(log, sizeof(log), "%s(%d): " format "\n", __FILE__ + sizeof(PROJECT_PATH), __LINE__, __VA_ARGS__); \
	printf(log); \
	OutputDebugStringA(log); \
} \
void(0)

#define LOG_W(format, ...) \
{ \
	wchar_t log[256]{}; \
	swprintf_s(log, sizeof(log), "%s(%d): " format "\n", __FILE__ + sizeof(PROJECT_PATH), __LINE__, __VA_ARGS__); \
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