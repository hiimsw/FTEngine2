#pragma once
#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"
#include "Core/Label.h"
#include "Core/Font.h"

enum class eShield_State
{
	Growing,
	Waiting,
	CoolTime,
	End
};

enum class eOrbit_State
{
	Rotating,
	CoolTime,
	End
};

enum class eSlow_Monster_State
{
	Moving,
	Stop,
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

	Texture mWhiteBarTexture{};
	Texture mRedBarTexture{};
	Texture mYellowBarTexture{};

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
	static constexpr float UI_HP_SCALE_WIDTH = 1.5f;
	static constexpr float RUN_MONSTER_WIDTH = 0.4f;
	static constexpr float UI_DASH_SCALE_WIDTH = 1.5f;
	static constexpr float MIN_ANGLE = 0.0f;
	static constexpr float MAX_ANGLE = 2.0f * Math::PI;
	static constexpr float TEST_RADIUS = 5.0f;
	static constexpr float MONSTER_SCALE = 1.2f;
	static constexpr float RUN_MONSTER_SCALE = 0.5f;
	static constexpr float SLOW_MONSTER_SCALE = 0.7f;
	static constexpr float START_LERP_TIME = 0.2f;
	static constexpr float DURING_TIME = 0.3f;

	static constexpr uint32_t MONSTER_COUNT = 10;
	static constexpr uint32_t RUN_MONSTER_COUNT = 5;
	static constexpr uint32_t BULLET_COUNT = 40;
	static constexpr uint32_t CASING_COUNT = BULLET_COUNT;
	static constexpr uint32_t MONSTER_ATTACK_VALUE = 10;
	static constexpr uint32_t HERO_MAX_HP = 1500;
	static constexpr uint32_t SHADOW_COUNT = 40;
	static constexpr uint32_t SLOW_MONSTER_COUNT = 5;
	static constexpr uint32_t MONSTER_MAX_HP = 20;
	static constexpr uint32_t DASH_MAX_COUNT = 10;

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
	Sprite mDashShadow[SHADOW_COUNT]{};
	Sprite mSlowMonsters[SLOW_MONSTER_COUNT]{};
	Sprite mSlowMonsterShadows[SLOW_MONSTER_COUNT][SHADOW_COUNT]{};
	Sprite mCasings[BULLET_COUNT]{};

	GizmoLine mLine{};

	Sprite* mTargetMonster = nullptr;
	Sprite* mTargetBullet = nullptr;

	int32_t mHeroHpValue = HERO_MAX_HP;
	int32_t mBulletValue = BULLET_COUNT;
	int32_t mCasingIndex = 0;
	int32_t mMonsterHpValue = MONSTER_MAX_HP;
	int32_t mDashCount = DASH_MAX_COUNT;

	D2D1_POINT_2F mHeroVelocity{};
	D2D1_POINT_2F mPrevBulletPosition[BULLET_COUNT]{};
	D2D1_POINT_2F mBulletDirections[BULLET_COUNT]{};
	D2D1_POINT_2F mCasingDirections[BULLET_COUNT]{};
	D2D1_POINT_2F mMonsterThicks[MONSTER_COUNT]{};
	D2D1_POINT_2F mRunMonsterThicks[RUN_MONSTER_COUNT]{};
	D2D1_POINT_2F mSlowMonsterThicks[SLOW_MONSTER_COUNT]{};

	D2D1_SIZE_F mShieldScale = { .width = SHELD_MIN_RADIUS, .height = SHELD_MIN_RADIUS };
	D2D1_SIZE_F mMonsterBulletEffectScales[MONSTER_COUNT]{};
	D2D1_SIZE_F mRunMonsterBulletEffectScales[RUN_MONSTER_COUNT]{};
	D2D1_SIZE_F mSlowMonsterBulletEffectScales[SLOW_MONSTER_COUNT]{};

	eShield_State mShieldState = eShield_State::End;
	eOrbit_State mOrbitState = eOrbit_State::End;
	eSlow_Monster_State mSlowMonsterState[SLOW_MONSTER_COUNT] = { eSlow_Monster_State::End };

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;
	ID2D1SolidColorBrush* mYellowBrush = nullptr;
	ID2D1SolidColorBrush* mCyanBrush = nullptr;

	bool mIsCursorConfined = false;
	bool mIsColliderKeyDown = false;
	bool mShieldBlinkOn = false;
	bool mOrbitBlinkOn = false;
	bool mIsMonsterToBullets[MONSTER_COUNT]{};
	bool mIsRunMonsterToBullets[RUN_MONSTER_COUNT]{};
	bool mIsSlowMonsterToBullets[SLOW_MONSTER_COUNT]{};
	bool mIsMonsterSpawns[MONSTER_COUNT]{};

	Font mDefaultFont{};
	Font mTimerFont{};
	Font mEndingFont{};
	Font mBulletFont{};

	Label mTimerLabel{};
	Label mHpValueLabel{};
	Label mEndingLabel{};
	Label mShieldLabel{};
	Label mBulletLabel{};

	float mMonsterSpawnTimer{};
	float mRunMonsterSpawnTimer{};
	float mSlowMonsterSpawnTimer{};
	float mGameTimer{};
	float mMonsterDamageTimer[MONSTER_COUNT]{};
	float mRunMonsterDamageTimer[RUN_MONSTER_COUNT]{};
	float mSlowMonsterDamageTimer[SLOW_MONSTER_COUNT]{};
	float mShieldTotalElapsedTimer{};
	float mOrbitAngle{};
	float mMonsterDieTimer[MONSTER_COUNT]{};
	float mRunMonsterDieTimer[RUN_MONSTER_COUNT]{};
	float mSlowMonsterDieTimer[SLOW_MONSTER_COUNT]{};
	float mInBoundaryToMonsterTimer[MONSTER_COUNT]{};
	float mInBoundaryToRunMonsterTimer[RUN_MONSTER_COUNT]{};
	float mInBoundaryToSlowMonsterTimer[SLOW_MONSTER_COUNT]{};
	float mCasingTimer[CASING_COUNT]{};
	float mMonsterDieEffectTimer[MONSTER_COUNT]{};
	float mRunMonsterDieEffectTimer[RUN_MONSTER_COUNT]{};
	float mSlowMonsterDieEffectTimer[SLOW_MONSTER_COUNT]{};

	D2D1_ELLIPSE mOrbitEllipse{};
};