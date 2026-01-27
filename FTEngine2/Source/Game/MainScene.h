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
	Texture mPinkRectangleTexture{};

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

	std::array<std::vector<Sprite*>, uint32_t(Layer::Count)> mSpriteLayers{};
	std::vector<Label*> mLabels{};

	// 공용
	static constexpr float TEST_RADIUS = 5.0f;

	static constexpr float IN_BOUNDARY_RADIUS = 30.0f;
	static constexpr float BOUNDARY_RADIUS = 400.0f;

	static constexpr float MIN_ANGLE = 0.0f;
	static constexpr float MAX_ANGLE = 2.0f * Math::PI;

	static constexpr float UI_CENTER_POSITION_Y = 300.0f;

	static constexpr float START_LERP_TIME = 0.2f;
	static constexpr float DURING_TIME = 0.3f;

	static constexpr uint32_t SHADOW_COUNT = 40;

	static constexpr uint32_t MONSTER_ATTACK_VALUE = 10;
	static constexpr uint32_t BULLET_ATTACK_VALUE = 10;
	static constexpr uint32_t BOUNDRY_ATTACK_VALUE = 10;

	Font mDefaultFont{};

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;
	ID2D1SolidColorBrush* mYellowBrush = nullptr;
	ID2D1SolidColorBrush* mCyanBrush = nullptr;

	// 키 관련
	bool mIsCursorConfined = false;
	bool mIsColliderKeyDown = false;

	// 플레이어
	static constexpr uint32_t HERO_MAX_HP = 1500;
	static constexpr uint32_t PLAYER_ATTACK_VALUE = 10;
	static constexpr float UI_HP_SCALE_WIDTH = 1.5f;
	Sprite mHero{};
	D2D1_POINT_2F mHeroVelocity{};
	int32_t mHeroHpValue = HERO_MAX_HP;

	// 플레이어 줌
	Sprite mZoom{};

	// 플레이어 대쉬
	static constexpr uint32_t DASH_MAX_COUNT = 10;
	Sprite mDashShadow[SHADOW_COUNT]{};
	int32_t mDashCount = DASH_MAX_COUNT;

	// 플레이어 총알
	static constexpr uint32_t BULLET_COUNT = 80;
	Sprite mBullets[BULLET_COUNT]{};
	D2D1_POINT_2F mPrevBulletPosition[BULLET_COUNT]{};
	D2D1_POINT_2F mBulletDirections[BULLET_COUNT]{};
	int32_t mBulletValue = BULLET_COUNT;

	// 플레이어 탄피
	static constexpr uint32_t CASING_COUNT = BULLET_COUNT;
	Sprite mCasings[CASING_COUNT]{};
	D2D1_POINT_2F mCasingDirections[CASING_COUNT]{};
	float mCasingTimers[CASING_COUNT]{};
	bool misKeyDownReload = false;

	// 플레이어 쉴드 스킬
	static constexpr float SHELD_MIN_RADIUS = 50.0f;
	static constexpr float SHELD_MAX_RADIUS = 150.0f;

	eShield_State mShieldState = eShield_State::End;
	D2D1_SIZE_F mShieldScale = { .width = SHELD_MIN_RADIUS, .height = SHELD_MIN_RADIUS };
	float mShieldTotalElapsedTimer{};
	bool mShieldBlinkOn = false;

	// 플레이어 공전 스킬
	D2D1_ELLIPSE mOrbitEllipse{};
	eOrbit_State mOrbitState = eOrbit_State::End;
	float mOrbitAngle{};
	bool mOrbitBlinkOn = false;

	// UI
	static constexpr float UI_DASH_SCALE_WIDTH = 1.5f;
	Sprite mUiDashBar{};
	Sprite mUiBackgroundDashBar{};

	Sprite mUiHpBar{};
	Sprite mUiBackgroundHpBar{};

	Label mHpValueLabel{};
	Label mDashValueLabel{};

	Font mBulletFont{};
	Label mBulletLabel{};

	Label mShieldLabel{};

	Font mTimerFont{};
	Label mTimerLabel{};
	float mGameTimer{};

	Font mEndingFont{};
	Label mEndingLabel{};

	// 몬스터
	static constexpr uint32_t MONSTER_COUNT = 10;
	static constexpr uint32_t MONSTER_MAX_HP = 20;
	static constexpr float MONSTER_SCALE = 1.2f;
	static constexpr float MONSTER_HP_BAR_WIDTH = 0.1f;

	Sprite mMonsters[MONSTER_COUNT]{};

	float mMonsterSpawnTimer{};
	bool mIsMonsterSpawns[MONSTER_COUNT]{};
	float mMonsterGrowingTimers[MONSTER_COUNT]{};

	// 충돌 관련
	bool mIsMonsterToBulletCollidings[MONSTER_COUNT]{};
	float mMonsterToInBoundryEnterCollidingTimers[MONSTER_COUNT]{};
	float mMonsterToPlayerEnterCollidingTimers[MONSTER_COUNT]{};
	float mMonsterDieTimers[MONSTER_COUNT]{};

	D2D1_POINT_2F mMonsterToBulletThicks[MONSTER_COUNT]{};
	D2D1_SIZE_F mMonsterToBulletEffectScales[MONSTER_COUNT]{};
	float mMonsterToBulletEffectTimers[MONSTER_COUNT]{};

	Sprite mMonsterBackgroundHpBars[MONSTER_COUNT]{};
	Sprite mMonsterHpBars[MONSTER_COUNT]{};
	int32_t mMonsterHpValues[MONSTER_COUNT]{};
	bool mMonsterDeads[MONSTER_COUNT]{};

	// 돌진 몬스터
	static constexpr uint32_t RUN_MONSTER_COUNT = 5;
	static constexpr float RUN_MONSTER_SCALE = 0.5f;
	static constexpr float RUN_MONSTER_START_BAR_WIDTH = 0.4f;

	static constexpr uint32_t RUN_MONSTER_MAX_HP = 10;
	static constexpr float RUN_MONSTER_HP_BAR_WIDTH = 0.05f;

	Sprite mRunMonsters[RUN_MONSTER_COUNT]{};

	Sprite mRunMonsterStartBars[RUN_MONSTER_COUNT]{};

	bool mIsRunMonsterSpawns[RUN_MONSTER_COUNT]{};
	float mRunMonsterSpawnTimer{};

	bool mIsRunMonsterToBullets[RUN_MONSTER_COUNT]{};

	Sprite mRunMonsterBackgroundHpBars[RUN_MONSTER_COUNT]{};
	Sprite mRunMonsterHpBars[RUN_MONSTER_COUNT]{};
	int32_t mRunMonsterHpValue[RUN_MONSTER_COUNT]{};

	D2D1_POINT_2F mRunMonsterThicks[RUN_MONSTER_COUNT]{};
	D2D1_SIZE_F mRunMonsterToBulletEffectScales[RUN_MONSTER_COUNT]{};

	float mRunMonsterDamageTimer[RUN_MONSTER_COUNT]{};
	float mRunMonsterDieTimer[RUN_MONSTER_COUNT]{};
	float mInBoundaryToRunMonsterTimer[RUN_MONSTER_COUNT]{};
	float mRunMonsterEffectTimer[RUN_MONSTER_COUNT]{};
	float mRunMonsterGrowingTimer[RUN_MONSTER_COUNT]{};

	// 느린 몬스터
	static constexpr uint32_t SLOW_MONSTER_COUNT = 5;
	static constexpr float SLOW_MONSTER_SCALE = 0.7f;

	Sprite mSlowMonsters[SLOW_MONSTER_COUNT]{};

	eSlow_Monster_State mSlowMonsterState[SLOW_MONSTER_COUNT] = { eSlow_Monster_State::End };

	// 스폰
	float mSlowMonsterSpawnTimers{};
	bool mIsSlowMonsterSpawns[SLOW_MONSTER_COUNT]{};
	float mSlowMonsterGrowingTimers[SLOW_MONSTER_COUNT]{};

	// 충돌 관련
	bool mIsSlowMonsterToBulletCollidings[SLOW_MONSTER_COUNT]{};
	float mSlowMonsterToBulletEnterCollidingTimers[SLOW_MONSTER_COUNT]{};
	float mSlowMonsterToBulletEffectTimers[SLOW_MONSTER_COUNT]{};

	float mSlowMonsterToInBoundaryTimers[SLOW_MONSTER_COUNT]{};
	float mSlowMonsterToPlayerEnterCollidingTimers[SLOW_MONSTER_COUNT]{};

	Sprite mSlowMonsterShadows[SLOW_MONSTER_COUNT][SHADOW_COUNT]{};

	D2D1_POINT_2F mSlowMonsterThicks[SLOW_MONSTER_COUNT]{};
	D2D1_SIZE_F mSlowMonsterBulletEffectScales[SLOW_MONSTER_COUNT]{};

	GizmoLine mLine{};

	// 충돌 관련
	Sprite* mTargetMonster = nullptr;
	Sprite* mTargetBullet = nullptr;
};