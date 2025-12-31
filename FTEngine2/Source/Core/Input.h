#pragma once

class Input final
{
public:
	enum class eMouseButton
	{
		Left,
		Right,
		Middle,
		_ForAlign,
		Count
	};

	enum class eCursorLockState
	{
		None,
		Locked,
		Confined,
		Count
	};

public:
	[[nodiscard]] static Input& Get();

	[[nodiscard]] bool GetKey(const uint32_t virtualKey) const;
	[[nodiscard]] bool GetKeyDown(const uint32_t virtualKey) const;
	[[nodiscard]] bool GetKeyUp(const uint32_t virtualKey) const;

	[[nodiscard]] bool GetMouseButton(const eMouseButton button) const;
	[[nodiscard]] bool GetMouseButtonDown(const eMouseButton button) const;
	[[nodiscard]] bool GetMouseButtonUp(const eMouseButton button) const;

	[[nodiscard]] D2D1_POINT_2F GetMousePosition() const;
	[[nodiscard]] int GetMouseScrollWheel() const;

	[[nodiscard]] bool IsCursorVisible() const;
	void SetCursorVisible(const bool bVisible);

	[[nodiscard]] eCursorLockState GetCursorLockState() const;
	void SetCursorLockState(const eCursorLockState cursorLockState);

public:
	void _Initialize(const HWND hWnd);
	void _Clear();

	void _SetKeyState(const uint32_t virtualKey, const bool bPressed);
	void _SetMouseButtonState(const eMouseButton button, const bool bPressed);
	void _SetMousePosition(const D2D1_POINT_2F mousePosition);
	void _SetMouseScrollWheel(const int32_t scrollWheel);

	void _ConfineCursor() const;
	void _RenewScreenCenterPosition();

private:
	Input() = default;
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
	~Input() = default;

private:
	HWND mHWnd = nullptr;
	D2D1_POINT_2L mScreenCenterPosition{};

	static constexpr size_t VIRTUAL_KEY_COUNT = 256;
	std::bitset<VIRTUAL_KEY_COUNT> mbKeysPressed{};
	std::bitset<VIRTUAL_KEY_COUNT> mbKeysStateChanged{};

	bool mbMouseButtonPressed[size_t(eMouseButton::Count)]{};
	bool mbMouseButtonStateChanged[size_t(eMouseButton::Count)]{};
	D2D1_POINT_2F mMousePosition{};
	int32_t mMouseScrollWheel = 0;

	bool mbCursorVisible = true;
	eCursorLockState mCursorLockState = eCursorLockState::None;
};