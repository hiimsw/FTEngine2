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

enum class eMonster_State
{
	Spawn,
	Life,
	Dead
};

enum class eCollision_State
{
	None,
	Enter,
	Stay,
	Exit
};

enum class eSpawnEffect_State
{
	Bigger,
	Smaller,
	End
};

struct GizmoLine
{
	D2D1_POINT_2F point0;
	D2D1_POINT_2F point1;
};

struct Player
{
	Sprite sprite;
	D2D1_POINT_2F velocity;
	float hitEffectTimer;
	bool isHitEffect;
	int32_t hp;
	int32_t prevHp;
};

struct Shield
{
	eShield_State state;
	D2D1_SIZE_F scale;
	float speed;

	float labelCoolTimer;

	float waitingTimer;
	bool isBlinkOn;
	float blinkTimer;
	float coolTimer;
};

struct Orbit
{
	D2D1_ELLIPSE ellipse;
	eOrbit_State state;
	float angle;

	float labelCoolTimer;

	float rotatingTimer;
	bool isBlinkOn;
	float blinkTimer;
	float coolTimer;
};

struct Monster
{
	Sprite sprite;
	eMonster_State state;

	// 충돌 관련
	bool isBulletColliding;

	eSpawnEffect_State spawnState;
	float spawnStartEffectTimer;
	float spawnEndEffectTimer;
	float deadEffectTimer;
	float moveSpeed;

	Sprite backgroundHpBar;
	Sprite hpBar;
	int32_t hp;
};

struct Bullet
{
	Sprite sprite;
	D2D1_POINT_2F prevPosition;
	D2D1_POINT_2F direction;
};

struct Casing
{
	Sprite sprite;
	D2D1_POINT_2F casingDirection;
	D2D1_POINT_2F startPosition;
	D2D1_POINT_2F endPosition;
	float casingTimer;
};

struct BulletEffect
{
	float timer;
	D2D1_POINT_2F thick;
	D2D1_SIZE_F scale;
};

struct Particle
{
	Sprite sprite;
	D2D1_POINT_2F direction;
	float speed;
};

struct MonsterSpawnDesc
{
	Monster* monster;
	const D2D1_SIZE_F scale;
	const D2D1_SIZE_F hitEffect;
	const int32_t maxHp;
};

struct MonsterSpawnEffectDesc
{
	Monster* monster;
	const D2D1_SIZE_F originalScale;
	const D2D1_SIZE_F effectScale;
	const float time;
	const float deltaTime;
};

struct MonsterDeadSoundDesc
{
	Monster* monster;
	const D2D1_SIZE_F originalScale;
	const D2D1_SIZE_F effectScale;
	const float time;
	const float deltaTime;
};

class MainScene final : public Scene
{
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

	void spawnMonster(const MonsterSpawnDesc& desc);
	void updateMonsterHp(Monster* monster, const float maxWidthBar, const uint32_t maxHp, const float deltaTime);
	void spawnMonsterEffect(const MonsterSpawnEffectDesc& desc);
	void deadMonsterEffect(const MonsterDeadSoundDesc& desc);

private:
	Texture mRectangleTexture{};
	Texture mRedRectangleTexture{};
	Texture mBlueRectangleTexture{};
	Texture mPinkRectangleTexture{};
	Texture mSkyBlueRectangleTexture{};

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

	std::array<std::vector<Sprite*>, uint32_t(Scene::Layer::End)> mSpriteLayers{};
	std::vector<Label*> mLabels{};

	// 공용
	static constexpr float IN_BOUNDARY_RADIUS = 60.0f;
	static constexpr float BOUNDARY_RADIUS = 500.0f;

	static constexpr float UI_CENTER_POSITION_Y = 300.0f;

	static constexpr uint32_t SHADOW_COUNT = 40;

	static constexpr uint32_t PLAYER_ATTACK_VALUE = 10;
	static constexpr uint32_t MONSTER_ATTACK_VALUE = 10;
	static constexpr uint32_t BULLET_ATTACK_VALUE = 10;
	static constexpr uint32_t BOUNDRY_ATTACK_VALUE = 10;

	Font mDefaultFont{};

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;
	ID2D1SolidColorBrush* mYellowBrush = nullptr;
	ID2D1SolidColorBrush* mOrange = nullptr;
	ID2D1SolidColorBrush* mCyanBrush = nullptr;
	ID2D1SolidColorBrush* mDarkGreen = nullptr;

	Sound mBackgroundSound{};

	bool mIsUpdate = true;

	// 키 관련
	bool mIsCursorConfined = false;
	bool mIsColliderKeyDown = false;

	// 플레이어
	static constexpr uint32_t HERO_MAX_HP = 1500;
	static constexpr float UI_HP_SCALE_WIDTH = 1.5f;

	Player mHero{};
	Sound mHeroHitSound{};

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
	static constexpr uint32_t BULLET_COUNT = 100;
	Bullet mBullets[BULLET_COUNT]{};
	float mBulletShootingCoolTimer{};
	int32_t mBulletValue = BULLET_COUNT;
	Sound mBulletSound{};

	bool misKeyDownReload = false;
	float mreloadKeyDownCoolTimer = 0.0f;
	float mreloadCoolTimer = 0.0f;
	Sound mReloadSound{};

	// 플레이어 탄피
	static constexpr uint32_t CASING_COUNT = BULLET_COUNT;
	Casing mCasings[CASING_COUNT]{};

	// 플레이어 쉴드 스킬
	static constexpr float SHELD_MIN_RADIUS = 50.0f;
	static constexpr float SHELD_MAX_RADIUS = 150.0f;

	Shield mShield{};
	Sound mShieldSound{};

	// 플레이어 공전 스킬
	Orbit mOrbit{};
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
	Label mShieldKeyLabel{};

	Label mOrbitLabel{};
	Label mOrbitKeyLabel{};

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

	static constexpr float MONSTER_DIE_EFFECT_TIME = 0.2f;

	Monster mMonsters[MONSTER_COUNT]{};
	float mMonsterSpawnTimer{};
	bool mIsMonsterHp[MONSTER_COUNT]{};

	static constexpr D2D1_SIZE_F MONSTER_TO_BULLET_EFFECT_SCALE = { 1.2f, 50.0f };
	Sprite mMonsterToBulletEffects[MONSTER_COUNT]{};
	float mMonsterToBulletEffectsTimers[MONSTER_COUNT]{};

	Sound mMonsterDeadSound{};

	// 돌진 몬스터
	static constexpr uint32_t RUN_MONSTER_COUNT = 1;
	static constexpr float RUN_MONSTER_SCALE = 0.5f;
	static constexpr float RUN_MONSTER_START_BAR_WIDTH = 0.4f;

	static constexpr float RUN_MONSTER_DIE_EFFECT_TIME = 0.5f;

	Monster mRunMonsters[RUN_MONSTER_COUNT]{};
	float mRunMonsterSpawnTimer{};

	// 바가 다 차면, 이동한다.
	static constexpr uint32_t RUN_MONSTER_MAX_HP = 1;
	static constexpr float RUN_MONSTER_HP_BAR_WIDTH = 0.05f;

	Sprite mRunMonsterStartBars[RUN_MONSTER_COUNT]{};
	bool mRunMonsterisMoveables[RUN_MONSTER_COUNT]{};
	D2D1_POINT_2F mRunMonsterMoveDirections[RUN_MONSTER_COUNT]{};
	float mRunMonsterMoveSpeeds[RUN_MONSTER_COUNT]{};

	BulletEffect mRunMonsterToBulletEffects[RUN_MONSTER_COUNT]{};

	Sound mRunMonsterDeadSound{};

	// 느린 몬스터
	static constexpr uint32_t SLOW_MONSTER_COUNT = 1;
	static constexpr float SLOW_MONSTER_SCALE = 0.7f;

	static constexpr float SLOW_MONSTER_HP_BAR_WIDTH = 0.06f;
	static constexpr uint32_t SLOW_MONSTER_MAX_HP = 10;

	static constexpr float SLOW_MONSTER_DIE_EFFECT_TIME = 0.5f;

	Monster mSlowMonsters[SLOW_MONSTER_COUNT]{};
	float mSlowMonsterSpawnTimer{};

	// 이동 관련
	eSlow_Monster_State mSlowMonsterState[SLOW_MONSTER_COUNT] = { eSlow_Monster_State::End };

	float mSlowMonsterMovingTimers[SLOW_MONSTER_COUNT]{};
	float mSlowMonsterStopTimers[SLOW_MONSTER_COUNT]{};

	D2D1_POINT_2F mSlowMonsterStartPositions[SLOW_MONSTER_COUNT]{};
	D2D1_POINT_2F mSlowMonsterEndPositions[SLOW_MONSTER_COUNT]{};

	// 이펙트 관련
	BulletEffect mSlowMonsterToBulletEffects[SLOW_MONSTER_COUNT]{};

	Sound mSlowMonsterDeadSound{};

	// 그림자 관련
	Sprite mSlowMonsterShadows[SLOW_MONSTER_COUNT][SHADOW_COUNT]{};
	float mSlowMonsterShadowCoolTimer{};

	// 충돌 관련
	Sprite* mTargetMonster = nullptr;
	Sprite* mTargetBullet = nullptr;
};