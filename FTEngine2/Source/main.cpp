#include "pch.h"

#include "Core/Constant.h"
#include "Core/Core.h"
#include "Core/Input.h"

#include "Game/MainScene.h"

using namespace std::chrono;

enum class eGameScene
{
	StartScene,
	VillageScene,
	Stage01Scene,
	MainScene,
	CameraGuideScene,
};

static LRESULT HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static Core gCore;
static eGameScene gGameScene;

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	constexpr const _TCHAR* MENU_NAME = TEXT("FTEngine");
	WNDCLASSEX windowClass
	{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = HandleWindowMessage,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = hInstance,
		.hIcon = nullptr,
		.hCursor = LoadCursor(nullptr, IDC_ARROW),
		.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)),
		.lpszMenuName = MENU_NAME,
		.lpszClassName = MENU_NAME,
		.hIconSm = nullptr
	};

	if (not RegisterClassEx(&windowClass))
	{
		MASSERT(false, "Failed to call RegisterClassEx()");
	}

	RECT windowRect{ .left = 0, .top = 0, .right = Constant::Get().GetWidth(), .bottom = Constant::Get().GetHeight() };
	if (AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false) == 0)
	{
		MASSERT(false, "Failed to call AdjustWindowRect()");
	}

	HWND hWnd = CreateWindow(windowClass.lpszClassName, windowClass.lpszClassName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hWnd, nShowCmd);

	Input::Get()._Initialize(hWnd);

	gCore.Initialize(hWnd, new MainScene);
	gGameScene = eGameScene::VillageScene;

	MSG msg{};
	float deltaTime = 0.0f;

	while (true)
	{
		auto startTime = high_resolution_clock::now();

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				goto EXIT_WINDOW;
			}
		}

		if (not gCore.Update(deltaTime))
		{
		}

		Input::Get()._Clear();

		deltaTime = duration_cast<milliseconds>(high_resolution_clock::now() - startTime).count() * 0.001f;
	}

EXIT_WINDOW:

	gCore.Finalize();

	return 0;
}

LRESULT HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (wParam == VK_MENU)
		{
			Input::Get()._SetKeyState(uint32_t(wParam), bool(WM_SYSKEYUP - message));
			return 0;
		}
		return 0;

	case WM_KEYDOWN:
	case WM_KEYUP:
		Input::Get()._SetKeyState(uint32_t(wParam), bool(WM_KEYUP - message));
		return 0;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		Input::Get()._SetMouseButtonState(Input::eMouseButton::Left, bool(WM_LBUTTONUP - message));
		return 0;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		Input::Get()._SetMouseButtonState(Input::eMouseButton::Right, bool(WM_RBUTTONUP - message));
		return 0;

	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		Input::Get()._SetMouseButtonState(Input::eMouseButton::Middle, bool(WM_MBUTTONUP - message));
		return 0;

	case WM_MOUSEMOVE:
		Input::Get()._SetMousePosition(D2D1_POINT_2F{ .x = float(LOWORD(lParam)), .y = float(HIWORD(lParam)) });
		return 0;

	case WM_MOUSEWHEEL:
		Input::Get()._SetMouseScrollWheel(int16_t(HIWORD(wParam)));
		return 0;

	case WM_SETFOCUS:
		if (Input::Get().GetCursorLockState() != Input::eCursorLockState::None)
		{
			Input::Get()._ConfineCursor();
		}
		return 0;

	case WM_MOVING:
		Input::Get()._RenewScreenCenterPosition();
		return 0;

	case WM_PAINT:
	{
		// 무효 영역을 없애 더 이상 WM_PAINT가 호출되지 않도록 합니다.
		// ref: https://learn.microsoft.com/en-us/windows/win32/gdi/the-wm-paint-message
		PAINTSTRUCT ps{};
		void(BeginPaint(hWnd, &ps));
		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}