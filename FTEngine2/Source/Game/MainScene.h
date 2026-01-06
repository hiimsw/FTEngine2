#pragma once

#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"

struct GizmoLine
{
	D2D1_POINT_2F Point0;
	D2D1_POINT_2F Point1;
};

class MainScene final : public Scene
{
	enum class Layer
	{
		Background,
		Monster,
		Player,
		UI,
		Count
	};

public:
	MainScene() = default;
	MainScene(const MainScene&) = delete;
	MainScene& operator=(const MainScene&) = delete;

	void Initialize() override;
	void PreDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI) override;
	bool Update(const float deltaTime) override;
	void PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI) override;
	void Finalize() override;

private:
	D2D1_RECT_F GetRectangleFromSprite(const Sprite& sprite);
	D2D1_ELLIPSE GetCircleFromSprite(const Sprite& sprite);

private:
	Texture mRectangleTexture{};
	Texture mRedRectangleTexture{};

	Texture mCircleTexture{};
	Texture mRedCircleTexture{};

	Camera mMainCamera{};

	std::array<std::vector<Sprite*>, uint32_t(Layer::Count)> mSpriteLayers{};
	Sprite mHero{};
	constexpr static uint32_t MONSTER_COUNT = 10;
	Sprite mMonsters[MONSTER_COUNT]{};
	Sprite mZoom{};
	GizmoLine mLine{};
	Sprite mBullet{};

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;

	bool mIsCursorConfined = false;
	bool mIsBulletActive = false;

	static constexpr float BOUNDARY_RADIUS = 400.0f;
	static constexpr float IN_BOUNDARY_RADIUS = 100.0f;
	static constexpr float OUTLINE_OFFSET = 380.0f;
};