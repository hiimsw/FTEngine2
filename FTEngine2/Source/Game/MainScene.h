#pragma once

#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"
#include "Core/Label.h"
#include "Core/Font.h"

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
	D2D1_RECT_F getRectangleFromSprite(const Sprite& sprite);
	D2D1_ELLIPSE getCircleFromSprite(const Sprite& sprite);

	float GetRandom(const float min, const float max);
	uint32_t GetRandom(const uint32_t min, const uint32_t max);

private:
	Texture mRectangleTexture{};
	Texture mRedRectangleTexture{};

	Texture mCircleTexture{};
	Texture mRedCircleTexture{};

	Camera mMainCamera{};

	std::array<std::vector<Sprite*>, uint32_t(Layer::Count)> mSpriteLayers{};
	Sprite mHero{};

	static constexpr uint32_t MONSTER_COUNT = 10;
	Sprite mMonsters[MONSTER_COUNT]{};

	Sprite mZoom{};
	GizmoLine mLine{};

	static constexpr uint32_t BULLET_COUNT = 6;
	Sprite mBullets[BULLET_COUNT]{};

	Sprite mHpBar{};

	int32_t mHeroHpMax = 1500;
	int32_t mHeroHpValue = mHeroHpMax;
	static constexpr int32_t mMonsterAttackValue = 10;
	
	D2D1_POINT_2F mPrevBulletPosition[BULLET_COUNT]{};

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;

	bool mIsCursorConfined = false;
	bool mIsBulletKeyDown = false;
	bool mIsColliderKeyDown = false;

	bool mIsMonsterSpwan[MONSTER_COUNT]{};

	D2D1_POINT_2F mHeroVelocity{};
	D2D1_POINT_2F mPrevHeroPosition{};

	std::vector<Label*> mLabels{};

	Font mTimerFont{};
	Label mTimerLabel{};

	Font mHpFont{};
	Label mHpValueLabel{};

	Font mEndingFont{};
	Label mEndingLabel{};

	static constexpr float BOUNDARY_RADIUS = 400.0f;
	static constexpr float IN_BOUNDARY_RADIUS = 100.0f;

	static constexpr float UI_CENTER_POSITION_Y = 250.0f;
	static constexpr float UI_HP_SCALE_WIDTH = 10.0f;

	float mSpawnTimer{};
	float mGameTimer{};
};