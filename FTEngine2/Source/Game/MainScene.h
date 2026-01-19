#pragma once

#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"
#include "Core/Label.h"
#include "Core/Font.h"

enum class SHIELD_STATE
{
	Growing,
	Waiting,
	CoolTime,
	End
};

enum class ORBIT_STATE
{
	Rotating,
	CoolTime,
	End
};

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

	float getRandom(const float min, const float max);
	uint32_t getRandom(const uint32_t min, const uint32_t max);

	D2D1_POINT_2F getMouseWorldPosition() const;

	void initializeCameraShake(const float amplitude, const float duration, const float frequency);
	D2D1_POINT_2F updateCameraShake(const float deltaTime);

private:
	Texture mRectangleTexture{};
	Texture mRedRectangleTexture{};
	Texture mBlueRectangleTexture{};

	Texture mCircleTexture{};
	Texture mRedCircleTexture{};

	Camera mMainCamera{};
	float mCameraShakeTime = 0.0f;
	float mCameraShakeTimer = 0.0f;
	float mCameraShakeAmplitude = 0.0f;
	float mCameraShakeDuration = 0.0f;
	float mCameraShakeFrequency = 0.0f;

	static constexpr float BOUNDARY_RADIUS = 400.0f;
	static constexpr float IN_BOUNDARY_RADIUS = 30.0f;
	static constexpr float SHELD_MAX_RADIUS = 150.0f;
	static constexpr float SHELD_MIN_RADIUS = 50.0f;
	static constexpr float UI_CENTER_POSITION_Y = 300.0f;
	static constexpr float UI_HP_SCALE_WIDTH = 10.0f;
	static constexpr float RUN_MONSTER_WIDTH = 0.4f;
	static constexpr float UI_DASH_SCALE_WIDTH = 3.0f;
	static constexpr float MIN_ANGLE = 0.0f;
	static constexpr float MAX_ANGLE = 2.0f * Math::PI;
	static constexpr float TEST_RADIUS = 5.0f;
	static constexpr float MONSTER_SCALE = 1.2f;
	static constexpr float RUN_MONSTER_SCALE = 0.5f;

	static constexpr uint32_t MONSTER_COUNT = 10;
	static constexpr uint32_t RUN_MONSTER_COUNT = 10;
	static constexpr uint32_t BULLET_COUNT = 20;
	static constexpr uint32_t DASH_MAX_COUNT = 3;

	static constexpr int32_t mMonsterAttackValue = 10;
	static constexpr int32_t mHeroHpMax = 1500;

	std::array<std::vector<Sprite*>, uint32_t(Layer::Count)> mSpriteLayers{};
	std::vector<Label*> mLabels{};

	Sprite mHero{};
	Sprite mMonsters[MONSTER_COUNT]{};
	Sprite mRunMonsters[RUN_MONSTER_COUNT]{};
	Sprite mRunMonsterBars[RUN_MONSTER_COUNT]{};
	Sprite mZoom{};
	Sprite mBullets[BULLET_COUNT]{};
	Sprite mHpBar{};
	Sprite mHpUiBar{};
	Sprite mDashValue{};
	Sprite mDashUiBar{};
	Sprite mDashShadow{};

	Sprite mTemp{};

	GizmoLine mLine{};

	Sprite* mTargetMonster = nullptr;
	Sprite* mTargetBullet = nullptr;

	int32_t mHeroHpValue = mHeroHpMax;
	int32_t mDashCount = DASH_MAX_COUNT;

	D2D1_POINT_2F mHeroVelocity{};
	D2D1_POINT_2F mPrevBulletPosition[BULLET_COUNT]{};
	D2D1_POINT_2F mBulletDirections[BULLET_COUNT]{};

	D2D1_SIZE_F mShieldScale = { .width = SHELD_MIN_RADIUS, .height = SHELD_MIN_RADIUS };

	SHIELD_STATE mShieldState = SHIELD_STATE::End;
	ORBIT_STATE mOrbitState = ORBIT_STATE::End;

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;
	ID2D1SolidColorBrush* mYellowBrush = nullptr;
	ID2D1SolidColorBrush* mCyanBrush = nullptr;

	bool mIsCursorConfined = false;
	bool mIsColliderKeyDown = false;
	bool mShieldBlinkOn = false;
	bool mOrbitBlinkOn = false;
	bool mIsMonsterToBullets[MONSTER_COUNT]{};
	bool mIsRunMonsterToBullets[RUN_MONSTER_COUNT]{};

	Font mDefaultFont{};
	Font mTimerFont{};
	Font mEndingFont{};

	Label mTimerLabel{};
	Label mHpValueLabel{};
	Label mEndingLabel{};
	Label mShieldLabel{};

	float mMonsterSpawnTimer{};
	float mRunMonsterSpawnTimer{};
	float mGameTimer{};
	float mMonsterDamageTimer{};
	float mRunMonsterDamageTimer{};
	float mShieldTotalElapsedTimer{};
	float mOrbitAngle{};
	float mMonsterDieTimer;
	float mRunMonsterDieTimer;

	D2D1_ELLIPSE mOrbitEllipse{};
};