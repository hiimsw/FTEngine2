#include "pch.h"

#include "Constant.h"
#include "Input.h"

Input& Input::Get()
{
	static Input input;
	return input;
}

bool Input::GetKey(const uint32_t virtualKey) const
{
	MASSERT(virtualKey < VIRTUAL_KEY_COUNT, "지원하지 않는 키입니다.");
	return mbKeysPressed[virtualKey];
}

bool Input::GetKeyDown(const uint32_t virtualKey) const
{
	MASSERT(virtualKey < VIRTUAL_KEY_COUNT, "지원하지 않는 키입니다.");
	return mbKeysStateChanged[virtualKey] and mbKeysPressed[virtualKey];
}

bool Input::GetKeyUp(const uint32_t virtualKey) const
{
	MASSERT(virtualKey < VIRTUAL_KEY_COUNT, "지원하지 않는 키입니다.");
	return mbKeysStateChanged[virtualKey] and not mbKeysPressed[virtualKey];
}

bool Input::GetMouseButton(const eMouseButton button) const
{
	return mbMouseButtonPressed[uint32_t(button)];
}

bool Input::GetMouseButtonDown(const eMouseButton button) const
{
	uint32_t index = uint32_t(button);
	return mbMouseButtonStateChanged[index] and mbMouseButtonPressed[index];
}

bool Input::GetMouseButtonUp(const eMouseButton button) const
{
	uint32_t index = uint32_t(button);
	return mbMouseButtonStateChanged[index] and not mbMouseButtonPressed[index];
}

D2D1_POINT_2F Input::GetMousePosition() const
{
	return mMousePosition;
}

int Input::GetMouseScrollWheel() const
{
	return mMouseScrollWheel;
}

bool Input::IsCursorVisible() const
{
	return mbCursorVisible;
}

void Input::SetCursorVisible(const bool bVisible)
{
	if (mbCursorVisible != bVisible)
	{
		mbCursorVisible = bVisible;
		ShowCursor(mbCursorVisible);
	}
}

Input::eCursorLockState Input::GetCursorLockState() const
{
	return mCursorLockState;
}

void Input::SetCursorLockState(const eCursorLockState cursorLockState)
{
	eCursorLockState previousCursorLockState = mCursorLockState;
	mCursorLockState = cursorLockState;

	if (previousCursorLockState == eCursorLockState::None)
	{
		if (mCursorLockState != eCursorLockState::None)
		{
			_ConfineCursor();

			if (mCursorLockState == eCursorLockState::Locked)
			{
				D2D1_POINT_2F position(Constant::Get().GetWidth() * 0.5f, Constant::Get().GetHeight() * 0.5f);
				_SetMousePosition(position);
			}
		}
	}
	else if (mCursorLockState == eCursorLockState::None)
	{
		ClipCursor(nullptr);
	}
}

void Input::_Initialize(HWND hWnd)
{
	ASSERT(hWnd != nullptr);

	mHWnd = hWnd;
	_RenewScreenCenterPosition();
}

void Input::_Clear()
{
	mbKeysStateChanged.reset();

	memset(mbMouseButtonStateChanged, 0, sizeof(mbMouseButtonStateChanged));
	mMouseScrollWheel = 0;
}

void Input::_SetKeyState(const uint32_t virtualKey, const bool bPressed)
{
	mbKeysStateChanged[virtualKey] = (mbKeysPressed[virtualKey] != bPressed);
	mbKeysPressed[virtualKey] = bPressed;
}

void Input::_SetMouseButtonState(const eMouseButton button, const bool bPressed)
{
	uint32_t index = uint32_t(button);
	mbMouseButtonStateChanged[index] = (mbMouseButtonPressed[index] != bPressed);
	mbMouseButtonPressed[index] = bPressed;
}

void Input::_SetMousePosition(const D2D1_POINT_2F mousePosition)
{
	mMousePosition = mousePosition;
	mMousePosition.y = Constant::Get().GetHeight() - mousePosition.y - 1.0f;

	if (mCursorLockState == eCursorLockState::Locked)
	{
		SetCursorPos(mScreenCenterPosition.x, mScreenCenterPosition.y);
	}
}

void Input::_SetMouseScrollWheel(const int32_t scrollWheel)
{
	mMouseScrollWheel += scrollWheel;
}

void Input::_ConfineCursor() const
{
	RECT clientRect = {};
	GetClientRect(mHWnd, &clientRect);
	MapWindowPoints(mHWnd, nullptr, reinterpret_cast<LPPOINT>(&clientRect), 2);
	ClipCursor(&clientRect);
}

void Input::_RenewScreenCenterPosition()
{
	RECT clientRect = {};
	GetClientRect(mHWnd, &clientRect);
	MapWindowPoints(mHWnd, nullptr, reinterpret_cast<LPPOINT>(&clientRect), 2);

	mScreenCenterPosition.x = clientRect.left + Constant::Get().GetWidth() / 2;
	mScreenCenterPosition.y = clientRect.top + Constant::Get().GetHeight() / 2;
}