#pragma once

class Texture;

class Sprite final
{
public:
	Sprite() = default;
	Sprite(const Sprite&) = delete;
	Sprite& operator=(const Sprite&) = delete;

	[[nodiscard]] const Texture* GetTextureOrNull() const;
	void SetTexture(Texture* texture);

	[[nodiscard]] bool IsActive() const;
	void SetActive(bool bActive);

	[[nodiscard]] D2D1_SIZE_F GetScale() const;
	void SetScale(const D2D1_SIZE_F& scale);

	[[nodiscard]] D2D1_POINT_2F GetPosition() const;
	void SetPosition(const D2D1_POINT_2F& position);

	[[nodiscard]] D2D1_POINT_2F GetCenter() const;
	void SetCenter(const D2D1_POINT_2F& center);

	[[nodiscard]] float GetAngle() const;
	void SetAngle(const float angle);

	[[nodiscard]] float GetOpacity() const;
	void SetOpacity(const float opacity);

	[[nodiscard]] bool IsUI() const;
	void SetUI(const bool bUI);

public:
	[[nodiscard]] Texture* _GetTextureOrNull() const;

private:
	Texture* mTexture = nullptr;

	bool mbActive = true;
	D2D1_SIZE_F mScale{ .width = 1.0f, .height = 1.0f };
	D2D1_POINT_2F mPosition{};
	D2D1_POINT_2F mCenter{};
	float mAngle = 0.0f;
	float mOpacity = 1.0f;
	bool mbUI = false;
};