#pragma once
#include "Core/Camera.h"
#include "Core/Font.h"
#include "Core/Label.h"
#include "Core/Scene.h"
#include "Core/Sound.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"

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

struct Monster
{
	Sprite BackgroundHpBar;
	Sprite HpBar;

	Sprite Sprite;

	bool IsSpawn;
	float GrowingTimer;

	// 충돌 관련
	D2D1_POINT_2F PrevPosition;
	float BoundryDistance;
	float InBoundryDistance;
	float PlayerDistance;

	bool IsBulletColliding;
	float BulletEffectTimer;

	bool IsDead;
	float DieTimer;

	D2D1_POINT_2F BulletThick;
	D2D1_SIZE_F BulletEffectScale;

	int32_t HpValue;
};

struct Bullet
{
	Sprite Sprite;
	D2D1_POINT_2F PrevPosition;
	D2D1_POINT_2F Direction;
};

struct Casing
{
	Sprite Sprite;
	D2D1_POINT_2F CasingDirection;
	D2D1_POINT_2F StartPosition;
	D2D1_POINT_2F EndPosition;
	float CasingTimer;
};

class MainScene final : public Scene
{
	enum class Layer
	{
		Background,
		Monster,
		Player,
		UI,
		End
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

	std::array<std::vector<Sprite*>, uint32_t(Layer::End)> mSpriteLayers{};
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

	static constexpr uint32_t PLAYER_ATTACK_VALUE = 10;
	static constexpr uint32_t MONSTER_ATTACK_VALUE = 10;
	static constexpr uint32_t BULLET_ATTACK_VALUE = 10;
	static constexpr uint32_t BOUNDRY_ATTACK_VALUE = 10;

	Font mDefaultFont{};

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;
	ID2D1SolidColorBrush* mYellowBrush = nullptr;
	ID2D1SolidColorBrush* mCyanBrush = nullptr;

	Sound mBackgroundSound{};

	// 키 관련
	bool mIsCursorConfined = false;
	bool mIsColliderKeyDown = false;

	// 플레이어
	static constexpr uint32_t HERO_MAX_HP = 1500;
	static constexpr float UI_HP_SCALE_WIDTH = 1.5f;
	Sprite mHero{};
	D2D1_POINT_2F mHeroVelocity{};
	int32_t mHeroHpValue = HERO_MAX_HP;
	Sound mHeroSound{};

	// 플레이어 줌
	Sprite mZoom{};

	// 플레이어 대쉬
	static constexpr uint32_t DASH_MAX_COUNT = 10;
	Sprite mDashShadows[SHADOW_COUNT]{};
	int32_t mDashCount = DASH_MAX_COUNT;
	float mDashCountTimer{};
	float mDashShadowCoolTimer{};
	float mDashSpeed = 0.0f;
	bool mIsDashing = false;

	// 플레이어 총알
	static constexpr uint32_t BULLET_COUNT = 80;
	Bullet mBullets[BULLET_COUNT]{};
	float mBulletShootingCoolTimer{};
	int32_t mBulletValue = BULLET_COUNT;
	Sound mBulletSound{};

	// 플레이어 탄피
	static constexpr uint32_t CASING_COUNT = BULLET_COUNT;
	Casing mCasings[CASING_COUNT]{};
	bool misKeyDownReload = false;

	// 플레이어 쉴드 스킬
	static constexpr float SHELD_MIN_RADIUS = 50.0f;
	static constexpr float SHELD_MAX_RADIUS = 150.0f;

	eShield_State mShieldState = eShield_State::End;
	D2D1_SIZE_F mShieldScale = { .width = SHELD_MIN_RADIUS, .height = SHELD_MIN_RADIUS };
	float mShieldTotalElapsedTimer{};
	bool mShieldBlinkOn = false;

	Sound mShieldSound{};

	// 플레이어 공전 스킬
	D2D1_ELLIPSE mOrbitEllipse{};
	eOrbit_State mOrbitState = eOrbit_State::End;
	float mOrbitAngle{};
	bool mOrbitBlinkOn = false;

	Sound mOrbitSound{};

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
	Sound mEndingSound{};

	// 몬스터
	static constexpr uint32_t MONSTER_COUNT = 1;
	static constexpr uint32_t MONSTER_MAX_HP = 20;
	static constexpr float MONSTER_SCALE = 1.2f;
	static constexpr float MONSTER_HP_BAR_WIDTH = 0.1f;

	Monster mMonsters[MONSTER_COUNT]{};
	float mMonsterSpawnTimer{};
	
	Sound mMonsterDeadSound{};

	// 돌진 몬스터
	static constexpr uint32_t RUN_MONSTER_COUNT = 1;
	static constexpr float RUN_MONSTER_SCALE = 0.5f;
	static constexpr float RUN_MONSTER_START_BAR_WIDTH = 0.4f;

	Monster mRunMonsters[RUN_MONSTER_COUNT]{};
	float mRunMonsterSpawnTimer{};

	// 바가 다 차면, 이동한다.
	static constexpr uint32_t RUN_MONSTER_MAX_HP = 10;
	static constexpr float RUN_MONSTER_HP_BAR_WIDTH = 0.05f;

	Sprite mRunMonsterStartBars[RUN_MONSTER_COUNT]{};
	bool mRunMonsterisMoveables[RUN_MONSTER_COUNT]{};
	D2D1_POINT_2F mRunMonsterMoveDirections[RUN_MONSTER_COUNT]{};
	float mRunMonsterMoveSpeeds[RUN_MONSTER_COUNT]{};

	Sound mRunMonsterDeadSound{};

	// 느린 몬스터
	static constexpr uint32_t SLOW_MONSTER_COUNT = 10;
	static constexpr float SLOW_MONSTER_SCALE = 0.7f;

	static constexpr float SLOW_MONSTER_HP_BAR_WIDTH = 0.06f;
	static constexpr uint32_t SLOW_MONSTER_MAX_HP = 10;

	Monster mSlowMonsters[SLOW_MONSTER_COUNT]{};
	float mSlowMonsterSpawnTimer{};

	// 이동 관련
	eSlow_Monster_State mSlowMonsterState[SLOW_MONSTER_COUNT] = { eSlow_Monster_State::End };

	float mSlowMonsterMovingTimers[SLOW_MONSTER_COUNT]{};
	float mSlowMonsterStopTimers[SLOW_MONSTER_COUNT]{};

	D2D1_POINT_2F mSlowMonsterStartPositions[SLOW_MONSTER_COUNT]{};
	D2D1_POINT_2F mSlowMonsterEndPositions[SLOW_MONSTER_COUNT]{};

	Sound mSlowMonsterDeadSound{};

	// 그림자 관련
	Sprite mSlowMonsterShadows[SLOW_MONSTER_COUNT][SHADOW_COUNT]{};
	float mSlowMonsterShadowCoolTimer{};

	// 충돌 관련
	Sprite* mTargetMonster = nullptr;
	Sprite* mTargetBullet = nullptr;

	// 기타
	GizmoLine mLine{};
};