#include "pch.h"
#include "MainScene.h"

#include "Core/Collision.h"
#include "Core/Constant.h"
#include "Core/Helper.h"
#include "Core/Input.h"
#include "Core/Transformation.h"

using namespace D2D1;

void MainScene::Initialize()
{
	// 기본 데이터를 초기화한다.
	{
		for (std::vector<Sprite*>& layer : mSpriteLayers)
		{
			layer.reserve(128);
		}

		SetSpriteLayers(mSpriteLayers.data(), uint32_t(mSpriteLayers.size()));

		SetCamera(&mMainCamera);

		mLabels.reserve(8);
		SetLabels(&mLabels);

		mTimerFont.Initialize(GetHelper(), L"Arial", 40.0f);
		mDefaultFont.Initialize(GetHelper(), L"Arial", 20.0f);
		mEndingFont.Initialize(GetHelper(), L"Arial", 50.0f);
		mBulletFont.Initialize(GetHelper(), L"Arial", 30.0f);

		Input::Get().SetCursorVisible(false);
		Input::Get().SetCursorLockState(Input::eCursorLockState::Confined);

		ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();
		HR(renderTarget->CreateSolidColorBrush(ColorF(1.0f, 1.0f, 1.0f), &mDefaultBrush));
		HR(renderTarget->CreateSolidColorBrush(ColorF(ColorF::Yellow), &mYellowBrush));
		HR(renderTarget->CreateSolidColorBrush(ColorF(ColorF::Cyan), &mCyanBrush));

		mIsCursorConfined = (Input::Get().GetCursorLockState() == Input::eCursorLockState::Confined);

		srand(unsigned int(time(nullptr)));
	}

	// 사용되는 이미지를 초기화한다.
	{
		mRectangleTexture.Initialize(GetHelper(), L"Resource/Rectangle.png");
		mRedRectangleTexture.Initialize(GetHelper(), L"Resource/RedRectangle.png");
		mBlueRectangleTexture.Initialize(GetHelper(), L"Resource/BlueRectangle.png");
		mPinkRectangleTexture.Initialize(GetHelper(), L"Resource/PinkRectangle.png");
		mSkyBlueRectangleTexture.Initialize(GetHelper(), L"Resource/SkyBlueRectangle.png");

		mCircleTexture.Initialize(GetHelper(), L"Resource/Circle.png");
		mRedCircleTexture.Initialize(GetHelper(), L"Resource/RedCircle.png");

		mWhiteBarTexture.Initialize(GetHelper(), L"Resource/WhiteBar.png");
		mRedBarTexture.Initialize(GetHelper(), L"Resource/RedBar.png");
		mYellowBarTexture.Initialize(GetHelper(), L"Resource/YellowBar.png");
	}

	// 사운드를 초기화한다.
	{
		mBackgroundSound.Initialize(GetHelper(), "Resource/Sound/DST-TowerDefenseTheme.mp3", true);
		mBackgroundSound.SetVolume(0.3f);

		mBulletSound.Initialize(GetHelper(), "Resource/Sound/shoot_sound.wav", false);

		mHeroSound.Initialize(GetHelper(), "Resource/Sound/player_hit.wav", false);
		mHeroSound.SetVolume(0.2f);

		mShieldSound.Initialize(GetHelper(), "Resource/Sound/E_Skill.mp3", false);
		mShieldSound.SetVolume(0.2f);

		mOrbitSound.Initialize(GetHelper(), "Resource/Sound/Q_Skill.mp3", false);
		mOrbitSound.SetVolume(0.2f);

		mMonsterDeadSound.Initialize(GetHelper(), "Resource/Sound/bone_break.mp3", false);
		mMonsterDeadSound.SetVolume(0.5f);

		mRunMonsterDeadSound.Initialize(GetHelper(), "Resource/Sound/bone_break2.mp3", false);
		mRunMonsterDeadSound.SetVolume(0.5f);

		mSlowMonsterDeadSound.Initialize(GetHelper(), "Resource/Sound/bone_break3.mp3", false);
		mSlowMonsterDeadSound.SetVolume(0.5f);

		mEndingSound.Initialize(GetHelper(), "Resource/Sound/game_over.mp3", false);
		mEndingSound.SetVolume(0.3f);
	}

	// 플레이어를 초기화한다.
	{
		mHero.hp = HERO_MAX_HP;
		mHero.velocity = {};

		Sprite& hero = mHero.sprite;
		hero.SetPosition({ .x = -200.0f, .y = 0.0f });
		hero.SetScale({ .width = 0.75f, .height = 0.75f });
		hero.SetTexture(&mPinkRectangleTexture);
		mSpriteLayers[uint32_t(Layer::Player)].push_back(&hero);

		for (Sprite& shadow : mDashShadows)
		{
			shadow.SetScale(hero.GetScale());
			shadow.SetTexture(&mPinkRectangleTexture);
			shadow.SetActive(false);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&shadow);
		}
	}

	// 총알을 초기화한다.
	{
		for (Bullet& bullet : mBullets)
		{
			Sprite& sprite = bullet.sprite;
			sprite.SetPosition(mHero.sprite.GetPosition());
			sprite.SetCenter({ .x = -0.5f, .y = 0.0f });
			sprite.SetScale({ 2.5f, 0.1f });
			sprite.SetActive(false);
			sprite.SetTexture(&mRedRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&sprite);
		}
	}

	// 탄피를 초기화한다.
	{
		for (Casing& casing : mCasings)
		{
			Sprite& sprite = casing.sprite;
			sprite.SetScale({ .width = 0.2f, .height = 0.2f });
			sprite.SetCenter({ -0.5f, 0.0f });
			sprite.SetOpacity(0.3f);
			sprite.SetActive(false);
			sprite.SetTexture(&mRedRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&sprite);
		}
	}

	// 공전 스킬을 초기화한다.
	{
		mOrbitEllipse =
		{
			.point = {.x = 0.0f, .y = -80.0f },
			.radiusX = 20.0f,
			.radiusY = 20.0f,
		};
	}

	// 몬스터를 초기화한다.
	{
		for (Monster& monster : mMonsters)
		{
			// 기본 정보를 초기화한다.
			monster.state = eMonster_State::Spawn;
			monster.hp = MONSTER_MAX_HP;

			Sprite& sprite = monster.sprite;
			sprite.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });
			sprite.SetActive(false);
			sprite.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&sprite);

			Sprite& hpBackground = monster.backgroundHpBar;
			hpBackground.SetScale({ .width = MONSTER_HP_BAR_WIDTH, .height = 0.7f });
			hpBackground.SetCenter({ .x = -0.5f, .y = 0.0f });
			hpBackground.SetActive(false);
			hpBackground.SetTexture(&mWhiteBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBackground);

			Sprite& hpBar = monster.hpBar;
			hpBar.SetScale({ .width = MONSTER_HP_BAR_WIDTH, .height = 0.7f });
			hpBar.SetCenter({ .x = -0.5f, .y = 0.0f });
			hpBar.SetActive(false);
			hpBar.SetTexture(&mRedBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBar);
		}

		for (Sprite& effect : mMonsterToBulletEffects)
		{
			effect.SetScale({ .width = MONSTER_TO_BULLET_EFFECT_SCALE.width, .height = MONSTER_TO_BULLET_EFFECT_SCALE.height });
			effect.SetActive(false);
			effect.SetTexture(&mSkyBlueRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&effect);
		}
	}

	// 돌진 몬스터를 초기화한다.
	{
		for (Monster& monster : mRunMonsters)
		{
			// 기본 정보를 초기화한다.
			monster.state = eMonster_State::Dead;
			monster.hp = RUN_MONSTER_MAX_HP;

			Sprite& sprite = monster.sprite;
			sprite.SetScale({ .width = RUN_MONSTER_SCALE, .height = RUN_MONSTER_SCALE });
			sprite.SetActive(false);
			sprite.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&sprite);

			// 체력바 배경 정보를 초기화한다.
			Sprite& background = monster.backgroundHpBar;
			background.SetScale({ .width = RUN_MONSTER_HP_BAR_WIDTH, .height = 0.5f });
			background.SetCenter({ -0.5f, 0.0f });
			background.SetActive(false);
			background.SetTexture(&mWhiteBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&background);

			// 체력바 정보를 초기화한다.
			Sprite& hpBar = monster.hpBar;
			hpBar.SetScale({ .width = RUN_MONSTER_HP_BAR_WIDTH, .height = 0.5f });
			hpBar.SetCenter({ -0.5f, 0.0f });
			hpBar.SetActive(false);
			hpBar.SetTexture(&mRedBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBar);
		}

		// 시작바를 생성한다.
		for (Sprite& bar : mRunMonsterStartBars)
		{
			bar.SetScale({ .width = 0.0f, .height = 0.1f });
			bar.SetCenter({ .x = -0.5f, .y = 0.0f });
			bar.SetActive(false);
			bar.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&bar);
		}
	}

	// 느린 몬스터를 초기화한다.
	{
		for (Monster& monster : mSlowMonsters)
		{
			// 기본 정보를 초기화한다.
			monster.state = eMonster_State::Dead;
			monster.hp = SLOW_MONSTER_MAX_HP;

			Sprite& sprite = monster.sprite;
			sprite.SetScale({ .width = SLOW_MONSTER_SCALE, .height = SLOW_MONSTER_SCALE });
			sprite.SetActive(false);
			sprite.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&sprite);

			// 체력바 배경 정보를 초기화한다.
			Sprite& background = monster.backgroundHpBar;
			background.SetScale({ .width = SLOW_MONSTER_HP_BAR_WIDTH, .height = 0.5f });
			background.SetCenter({ -0.5f, 0.0f });
			background.SetActive(false);
			background.SetTexture(&mWhiteBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&background);

			// 체력바 정보를 초기화한다.
			Sprite& hpBar = monster.hpBar;
			hpBar.SetScale({ .width = SLOW_MONSTER_HP_BAR_WIDTH, .height = 0.5f });
			hpBar.SetCenter({ -0.5f, 0.0f });
			hpBar.SetActive(false);
			hpBar.SetTexture(&mRedBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBar);
		}

		// 그림자를 초기화한다.
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			for (uint32_t j = 0; j < SHADOW_COUNT; ++j)
			{
				Sprite& shadow = mSlowMonsterShadows[i][j];

				shadow.SetScale({ .width = SLOW_MONSTER_SCALE, .height = SLOW_MONSTER_SCALE });

				float opacity = 0.8f - (float(j) / SHADOW_COUNT);
				shadow.SetOpacity(opacity);

				shadow.SetActive(false);
				shadow.SetTexture(&mRectangleTexture);

				mSpriteLayers[uint32_t(Layer::Monster)].push_back(&shadow);
			}
		}
	}

	// 줌을 초기화한다.
	{
		mZoom.SetScale({ .width = 0.7f, .height = 0.7f });
		mZoom.SetAngle(45.0f);
		mZoom.SetUI(true);
		mZoom.SetTexture(&mRedRectangleTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mZoom);
	}

	// 카메라를 초기화한다.
	{
		const D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
		const D2D1_POINT_2F cameraPosition{ .x = heroPosition.x * 0.5f, .y = heroPosition.y * 0.5f };

		mMainCamera.SetPosition(cameraPosition);
	}

	// Dash 게이지를 초기화한다.
	{
		mUiBackgroundDashBar.SetPosition({ .x = -300.0f, .y = -UI_CENTER_POSITION_Y });
		mUiBackgroundDashBar.SetScale({ .width = UI_DASH_SCALE_WIDTH, .height = 1.0f });
		mUiBackgroundDashBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiBackgroundDashBar.SetUI(true);
		mUiBackgroundDashBar.SetTexture(&mWhiteBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiBackgroundDashBar);

		D2D1_POINT_2F offset = { .x = mUiBackgroundDashBar.GetPosition().x + 2.5f, .y = mUiBackgroundDashBar.GetPosition().y };
		mUiDashBar.SetPosition(offset);

		mUiDashBar.SetScale(mUiBackgroundDashBar.GetScale());
		mUiDashBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiDashBar.SetUI(true);
		mUiDashBar.SetTexture(&mYellowBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiDashBar);
	}

	// HP바를 초기화한다.
	{
		constexpr float OFFSET_Y = 50.0f;
		mUiBackgroundHpBar.SetPosition({ .x = mUiBackgroundDashBar.GetPosition().x, .y = mUiBackgroundDashBar.GetPosition().y + OFFSET_Y });

		mUiBackgroundHpBar.SetScale({ .width = UI_HP_SCALE_WIDTH, .height = 1.0f });
		mUiBackgroundHpBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiBackgroundHpBar.SetUI(true);
		mUiBackgroundHpBar.SetTexture(&mWhiteBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiBackgroundHpBar);

		D2D1_POINT_2F offset = { .x = mUiBackgroundHpBar.GetPosition().x + 2.5f, .y = mUiBackgroundHpBar.GetPosition().y };
		mUiHpBar.SetPosition(offset);

		mUiHpBar.SetScale(mUiBackgroundHpBar.GetScale());
		mUiHpBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiHpBar.SetUI(true);
		mUiHpBar.SetTexture(&mRedBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiHpBar);
	}

	// 라벨을 초기화한다.
	{
		// 타이머
		{
			mTimerLabel.SetFont(&mTimerFont);
			mTimerLabel.SetUI(true);
			mTimerLabel.SetPosition({ .x = 0.0f, .y = UI_CENTER_POSITION_Y });
			mLabels.push_back(&mTimerLabel);
		}

		// 현재 대쉬 개수
		{
			mDashValueLabel.SetFont(&mDefaultFont);
			mDashValueLabel.SetUI(true);

			const D2D1_POINT_2F dashBarPosition = mUiBackgroundDashBar.GetPosition();
			constexpr float OFFSET_X = 10.0f;
			D2D1_POINT_2F offset = { .x = dashBarPosition.x - OFFSET_X, .y = dashBarPosition.y };
			mDashValueLabel.SetPosition(offset);

			mDashValueLabel.SetCenter({ .x = 0.5f, .y = 0.0f });
			mDashValueLabel.SetText(L"Dash: " + std::to_wstring(mDashCount) + L" / " + std::to_wstring(DASH_MAX_COUNT));
			mLabels.push_back(&mDashValueLabel);
		}

		// 현재 체력
		{
			mHpValueLabel.SetFont(&mDefaultFont);
			mHpValueLabel.SetUI(true);

			const D2D1_POINT_2F hpBarPosition = mUiHpBar.GetPosition();
			constexpr float OFFSET_X = 10.0f;
			D2D1_POINT_2F offset = { .x = hpBarPosition.x - OFFSET_X, .y = hpBarPosition.y };
			mHpValueLabel.SetPosition(offset);

			mHpValueLabel.SetCenter({ .x = 0.5f, .y = 0.0f });
			mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHero.hp) + L" / " + std::to_wstring(HERO_MAX_HP));
			mLabels.push_back(&mHpValueLabel);
		}

		// 쉴드 쿨 타이머
		{
			mShieldLabel.SetFont(&mDefaultFont);
			mShieldLabel.SetUI(true);

			const D2D1_POINT_2F position = mHpValueLabel.GetPosition();
			const D2D1_POINT_2F offset = { .x = position.x + 280.0f, .y = position.y };
			mShieldLabel.SetPosition(offset);

			mShieldLabel.SetCenter({ .x = -0.5f, .y = 0.0f });
			mLabels.push_back(&mShieldLabel);
		}

		// 총알 개수
		{
			mBulletLabel.SetFont(&mBulletFont);
			mBulletLabel.SetUI(true);

			const D2D1_POINT_2F position = mTimerLabel.GetPosition();
			const D2D1_POINT_2F offset = { .x = position.x - 360.0f, .y = position.y };
			mBulletLabel.SetPosition(offset);

			mBulletLabel.SetText(std::to_wstring(mBulletValue) + L"/" + std::to_wstring(BULLET_COUNT));
			mBulletLabel.SetCenter({ .x = -0.5f, .y = 0.0f });
			mLabels.push_back(&mBulletLabel);
		}

		// 엔딩
		{
			mEndingLabel.SetFont(&mEndingFont);
			mEndingLabel.SetUI(true);
			mEndingLabel.SetPosition({});
			mEndingLabel.SetActive(false);
			mEndingLabel.SetText(L"GameOver");
			mLabels.push_back(&mEndingLabel);
		}
	}

	mLine.point0 = { .x = -200.0f, .y = 200.0f };
	mLine.point1 = { .x = 150.0f, .y = 100.0f };

	// TODO(이수원): 디버깅 용도로 사용되며, 추후 삭제 예정이다.
	{
		//for (Sprite* sprite : mSpriteLayer)
		//{
		//	D2D1_SIZE_F scale = sprite->GetScale();
		//	scale.width *= SCALE;
		//	scale.height *= SCALE;
		//	sprite->SetScale(scale);
		//}
	}
}

void MainScene::PreDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
	ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();

	// 바운더리를 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix() * view;
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse{ .radiusX = BOUNDARY_RADIUS, .radiusY = BOUNDARY_RADIUS };
		renderTarget->DrawEllipse(ellipse, mDefaultBrush, 2.0f);
	}

	// 내부 바운더리를 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix() * view;
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse{ .radiusX = IN_BOUNDARY_RADIUS, .radiusY = IN_BOUNDARY_RADIUS };
		renderTarget->DrawEllipse(ellipse, mDefaultBrush, 2.0f);
	}
}

bool MainScene::Update(const float deltaTime)
{
	mBackgroundSound.Play();

	// 키를 업데이트한다.
	{
		// 게임을 종료한다.
		if (Input::Get().GetKeyDown(VK_ESCAPE))
		{
			return false;
		}

		// 마우스 커서를 설정한다.
		if (Input::Get().GetKeyDown(VK_CONTROL))
		{
			mIsCursorConfined = !mIsCursorConfined;
			Input::Get().SetCursorLockState(mIsCursorConfined ? Input::eCursorLockState::Confined : Input::eCursorLockState::None);
			Input::Get().SetCursorVisible(not mIsCursorConfined);
		}

		if (Input::Get().GetKeyDown('T'))
		{
			mIsColliderKeyDown = !mIsColliderKeyDown;
		}
	}

	// 줌을 업데이트한다.
	{
		// 세팅되어 있는 좌표(0, 0)으로 맞춰준다.
		const D2D1_POINT_2F centerOffset =
		{
			.x = (Constant::Get().GetWidth() - 1.0f) * 0.5f,
			.y = (Constant::Get().GetHeight() - 1.0f) * 0.5f
		};

		const D2D1_POINT_2F mousePosition = Input::Get().GetMousePosition();

		// 마우스 좌표를 좌표계에 맞춘다.
		const D2D1_POINT_2F screenPosition = Math::SubtractVector(mousePosition, centerOffset);
		mZoom.SetPosition(screenPosition);

		const D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
		const D2D1_POINT_2F zoomPosition = getMouseWorldPosition();

		D2D1_POINT_2F toTarget = Math::SubtractVector(zoomPosition, heroPosition);
		const float length = Math::GetVectorLength(toTarget);

		constexpr float MAX_SCALE = 0.7f;
		constexpr float MIN_SCALE = 0.5f;
		constexpr float MAX_DISTANCE = 300.0f;
		float zoomScale = std::clamp(length / MAX_DISTANCE, MIN_SCALE, MAX_SCALE);

		mZoom.SetScale({ .width = zoomScale, .height = zoomScale });
	}

	// 플레이어를 업데이트한다.
	{
		// 이동을 업데이트한다.
		{
			constexpr float MAX_SPEED = 300.0f;
			constexpr float ACC = 32.0f; // 가속도

			const int32_t moveX = Input::Get().GetKey('D') - Input::Get().GetKey('A');
			const int32_t moveY = Input::Get().GetKey('W') - Input::Get().GetKey('S');

			static int32_t previousMoveX;
			static int32_t previousMoveY;
			D2D1_POINT_2F& velocity = mHero.velocity;

			if (moveX != 0)
			{
				velocity.x = std::clamp(velocity.x + ACC * moveX, -MAX_SPEED, MAX_SPEED);
				previousMoveX = moveX;
			}
			else
			{
				if (previousMoveX > 0)
				{
					velocity.x = max(velocity.x - ACC, 0.0f);
				}
				else
				{
					velocity.x = min(velocity.x + ACC, 0.0f);
				}
			}

			if (moveY != 0)
			{
				velocity.y = std::clamp(velocity.y + ACC * moveY, -MAX_SPEED, MAX_SPEED);
				previousMoveY = moveY;
			}
			else
			{
				if (previousMoveY > 0)
				{
					velocity.y = max(velocity.y - ACC, 0.0f);
				}
				else
				{
					velocity.y = min(velocity.y + ACC, 0.0f);
				}
			}

			constexpr float MAX_DASH_SPEED = 600.0f;
			constexpr float DASH_ACC = 30.0f;
			static D2D1_POINT_2F dashDirection;

			if (Math::GetVectorLength(velocity) != 0.0f)
			{
				float speed = min(Math::GetVectorLength(velocity), MAX_SPEED);
				const D2D1_POINT_2F direction = Math::NormalizeVector(velocity);
				D2D1_POINT_2F adjustVelocity = Math::ScaleVector(direction, speed);
				adjustVelocity = Math::ScaleVector(adjustVelocity, deltaTime);

				if (not mIsDashing and Input::Get().GetKeyDown(VK_SPACE))
				{
					dashDirection = direction;

					if (mDashCount != 0)
					{
						mDashCount--;
						mIsDashing = true;
						mDashShadowCoolTimer = 0.0f;
					}
				}

				const D2D1_POINT_2F position = Math::AddVector(mHero.sprite.GetPosition(), adjustVelocity);
				mHero.sprite.SetPosition(position);
			}

			if (mIsDashing)
			{
				mDashSpeed = min(mDashSpeed + DASH_ACC, MAX_DASH_SPEED);
				const D2D1_POINT_2F velocity = Math::ScaleVector(dashDirection, mDashSpeed * deltaTime);

				if (mDashSpeed >= MAX_DASH_SPEED)
				{
					mDashSpeed = 0.0f;
					mIsDashing = false;
				}

				const D2D1_POINT_2F position = Math::AddVector(mHero.sprite.GetPosition(), velocity);
				mHero.sprite.SetPosition(position);

				mDashShadowCoolTimer -= deltaTime;
				if (mDashShadowCoolTimer <= 0.0f)
				{
					for (Sprite& shadow : mDashShadows)
					{
						if (shadow.IsActive())
						{
							continue;
						}

						shadow.SetOpacity(1.0f);
						shadow.SetPosition(position);
						shadow.SetActive(true);

						break;
					}

					mDashShadowCoolTimer = 0.05f;
				}
			}

			for (Sprite& shadow : mDashShadows)
			{
				if (not shadow.IsActive())
				{
					continue;
				}

				float opacity = shadow.GetOpacity();
				opacity -= 5.0f * deltaTime;
				shadow.SetOpacity(opacity);

				if (opacity <= 0.0f)
				{
					shadow.SetActive(false);
				}
			}

			if (mDashCount < DASH_MAX_COUNT)
			{
				mDashCountTimer += deltaTime;

				if (mDashCountTimer >= 2.0f)
				{
					mDashCount++;
					mDashCountTimer = 0.0f;
				}
			}
		}

		// 총알을 업데이트한다.
		{
			constexpr float LENGTH = 100.0f;

			mBulletShootingCoolTimer = max(mBulletShootingCoolTimer - deltaTime, 0.0f);

			// 총알을 스폰한다.
			if (Input::Get().GetMouseButton(Input::eMouseButton::Left)
				and mBulletShootingCoolTimer <= 0.001f
				and mBulletValue != 0
				and not misKeyDownReload)
			{
				mBulletSound.Replay();

				for (uint32_t i = 0; i < BULLET_COUNT; ++i)
				{
					Bullet& bullet = mBullets[i];
					Sprite& bulletSprite = bullet.sprite;

					if (bulletSprite.IsActive())
					{
						continue;
					}

					const D2D1_POINT_2F spawnPosition = mHero.sprite.GetPosition();
					bulletSprite.SetPosition(spawnPosition);
					bullet.prevPosition = spawnPosition;

					// 이동 방향을 구한다.
					bullet.direction = Math::SubtractVector(getMouseWorldPosition(), spawnPosition);

					// 거리에 따라 반동효과가 다르다.
					const float length = Math::GetVectorLength(bullet.direction);
					bullet.direction = (length >= 200.0f) ?
						Math::RotateVector(bullet.direction, getRandom(-10.0f, 10.0f))
						: Math::RotateVector(bullet.direction, getRandom(-5.0f, 5.0f));

					bullet.direction = Math::NormalizeVector(bullet.direction);

					float angle = Math::ConvertRadianToDegree(atan2f(bullet.direction.y, bullet.direction.x));
					bulletSprite.SetAngle(-angle);

					bulletSprite.SetActive(true);

					bulletSprite.SetOpacity(0.0f);

					mBulletValue--;

					// 탄피를 생성한다.
					{
						for (uint32_t j = 0; j < CASING_COUNT; ++j)
						{
							Casing& casing = mCasings[j];
							Sprite& casingSprite = casing.sprite;

							if (casingSprite.IsActive())
							{
								continue;
							}

							casingSprite.SetOpacity(1.0f);
							casingSprite.SetActive(true);

							D2D1_POINT_2F& casingDirection = casing.casingDirection;
							casingDirection = Math::NormalizeVector(bullet.direction);
							casingDirection = Math::RotateVector(casingDirection, getRandom(-30.0f, 30.0f));
							casingDirection = Math::ScaleVector(casingDirection, -1.0f);	// 뒤로 가도록 조정한다.

							D2D1_POINT_2F spawnPosition = mHero.sprite.GetPosition();
							constexpr float OFFSET = 50.0f;
							spawnPosition = Math::AddVector(spawnPosition, Math::ScaleVector(casingDirection, OFFSET));
							casingSprite.SetPosition((spawnPosition));

							// 탄피의 이동 좌표를 생성한다.
							casing.startPosition = casingSprite.GetPosition();
							casing.endPosition = Math::AddVector(casing.startPosition, Math::ScaleVector(casingDirection, LENGTH));

							break;
						}
					}

					// 카메라 흔들기를 시작합니다.
					{
						const float amplitude = Constant::Get().GetHeight() * getRandom(0.008f, 0.012f);
						const float duration = getRandom(0.05f, 0.08f);
						const float frequency = getRandom(50.0f, 60.0f);
						initializeCameraShake(amplitude, duration, frequency);
					}

					break;
				}

				mBulletShootingCoolTimer = 0.12f;
			}

			// 총알을 이동시킨다.
			{
				constexpr float MOVE_SPEED = 1500.0f;

				for (uint32_t i = 0; i < BULLET_COUNT; ++i)
				{
					Bullet& bullet = mBullets[i];
					Sprite& bulletSprite = bullet.sprite;

					if (not bulletSprite.IsActive())
					{
						continue;
					}

					const D2D1_POINT_2F velocity = Math::ScaleVector(bullet.direction, MOVE_SPEED * deltaTime);
					const D2D1_POINT_2F position = Math::AddVector(bulletSprite.GetPosition(), velocity);

					bullet.prevPosition = bulletSprite.GetPosition();
					bulletSprite.SetPosition(position);

					float opacity = bulletSprite.GetOpacity();

					D2D1_POINT_2F lerp = { opacity, opacity };
					lerp = Math::LerpVector(lerp, { 1.0f, 1.0f }, 10.0f * deltaTime);
					bulletSprite.SetOpacity(lerp.x);
				}
			}

			// 탄피를 이동시킨다.
			{
				constexpr float SPEED = 400.0f;
				constexpr float MOVE_TIME = 1.0f;

				for (uint32_t i = 0; i < CASING_COUNT; ++i)
				{
					Casing& casing = mCasings[i];
					Sprite& casingSprite = casing.sprite;

					if (not casingSprite.IsActive())
					{
						continue;
					}

					casing.casingTimer += deltaTime;

					float t = casing.casingTimer / MOVE_TIME;
					t = std::clamp(t, 0.0f, 1.0f);

					D2D1_POINT_2F position = Math::LerpVector(casing.startPosition, casing.endPosition, t);
					casingSprite.SetPosition(position);

					if (t >= 1.0f)
					{
						casingSprite.SetActive(false);
						casing.casingTimer = 0.0f;
					}

					float opacity = casingSprite.GetOpacity();
					opacity -= 0.8f * deltaTime;
					casingSprite.SetOpacity(opacity);
				}
			}

			// 재장전을 한다.
			{
				static float reloadCoolTimer = 0.0f;
				constexpr float RELOAD_TIME = 1.5f;

				if (mBulletValue <= 0)
				{
					reloadCoolTimer += deltaTime;

					if (reloadCoolTimer >= RELOAD_TIME)
					{
						mBulletValue = BULLET_COUNT;
						reloadCoolTimer = 0.0f;
					}
				}

				if (Input::Get().GetKeyDown('R'))
				{
					if (mBulletValue != BULLET_COUNT)
					{
						misKeyDownReload = true;
					}
				}

				static float reloadKeyDownCoolTimer = 0.0f;

				if (misKeyDownReload)
				{
					reloadKeyDownCoolTimer += deltaTime;

					if (reloadKeyDownCoolTimer >= RELOAD_TIME)
					{
						mBulletValue = BULLET_COUNT;
						reloadKeyDownCoolTimer = 0.0f;
						misKeyDownReload = false;
					}
				}
			}
		}

		// 쉴드 키를 업데이트한다.
		{
			static float speed;
			static float shieldTimer;
			static float shieldCoolTimer;
			constexpr float SHIELD_SKILL_DURATION = 3.0f;
			static float blinkTimer;

			if (mShieldState == eShield_State::End
				and Input::Get().GetKeyDown('E'))
			{
				mShieldSound.Replay();

				mShieldLabel.SetActive(true);
				mShieldState = eShield_State::Growing;
			}

			mShieldTotalElapsedTimer += deltaTime;
			const uint32_t seconds = uint32_t(mShieldTotalElapsedTimer) % 60;

			if (mShieldState != eShield_State::End)
			{
				mShieldLabel.SetText(std::to_wstring(8 - seconds));
			}
			else
			{
				mShieldLabel.SetActive(false);
			}

			switch (mShieldState)
			{
			case eShield_State::Growing:
			{
				speed = 50.0f;

				mShieldScale.width += speed * deltaTime;
				mShieldScale.height += speed * deltaTime;

				if (mShieldScale.width >= SHELD_MAX_RADIUS)
				{
					mShieldState = eShield_State::Waiting;
				}

				break;
			}

			case eShield_State::Waiting:
			{
				speed = 0.0f;

				shieldTimer += deltaTime;

				// 1초 남았을 때 깜빡거린다.
				if ((SHIELD_SKILL_DURATION - shieldTimer) <= 1.0f)
				{
					blinkTimer += deltaTime;

					if (blinkTimer >= 0.1f)
					{
						mShieldBlinkOn = !mShieldBlinkOn;
						blinkTimer = 0.0f;
					}
				}
				else
				{
					mShieldBlinkOn = true;
				}

				if (shieldTimer >= SHIELD_SKILL_DURATION)
				{
					mShieldScale.width = SHELD_MIN_RADIUS;
					mShieldScale.height = SHELD_MIN_RADIUS;

					shieldTimer = 0.0f;

					blinkTimer = 0.0f;
					mShieldBlinkOn = true;

					mShieldState = eShield_State::CoolTime;
				}

				break;
			}

			case eShield_State::CoolTime:
			{
				mShieldSound.Pause();

				shieldCoolTimer += deltaTime;

				if (shieldCoolTimer >= 2.0f)
				{
					shieldCoolTimer = 0.0f;
					mShieldState = eShield_State::End;
				}

				break;
			}

			case eShield_State::End:
				mShieldTotalElapsedTimer = 0.0f;
				break;

			default:
				break;
			}
		}

		// 플레이어 주변을 공전하는 스킬을 업데이트한다.
		{
			if (Input::Get().GetKeyDown('Q')
				and mOrbitState == eOrbit_State::End)
			{
				mOrbitSound.Replay();

				mOrbitState = eOrbit_State::Rotating;
			}

			static float orbitOnTimer;
			static float orbitCoolTimer;
			static float tempTimer;
			constexpr float ORBIT_ON_TIMER = 4.0f;
			constexpr float ORBIT_COOL_TIMER = 3.0f;
			constexpr float SPEED = 400.0f;

			switch (mOrbitState)
			{

			case eOrbit_State::Rotating:
			{
				orbitOnTimer += deltaTime;
				mOrbitAngle += SPEED * deltaTime;

				constexpr float OFFSET = 120.0f;
				mOrbitEllipse.point = { .x = 0.0f, .y = -OFFSET };
				mOrbitEllipse.point = Math::RotateVector(mOrbitEllipse.point, -mOrbitAngle);

				// 1초 남았을 때 깜빡거린다.
				if ((ORBIT_ON_TIMER - orbitOnTimer) <= 1.0f)
				{
					tempTimer += deltaTime;

					if (tempTimer >= 0.1f)
					{
						mOrbitBlinkOn = !mOrbitBlinkOn;
						tempTimer = 0.0f;
					}
				}
				else
				{
					mOrbitBlinkOn = true;
				}

				if (orbitOnTimer >= ORBIT_ON_TIMER)
				{
					orbitOnTimer = 0.0f;
					mOrbitState = eOrbit_State::CoolTime;
				}

				break;
			}
			case eOrbit_State::CoolTime:
			{
				mOrbitSound.Pause();

				orbitCoolTimer += deltaTime;

				if (orbitCoolTimer >= ORBIT_COOL_TIMER)
				{
					orbitCoolTimer = 0.0f;
					mOrbitState = eOrbit_State::End;
				}
				break;
			}
			case eOrbit_State::End:
				break;
			default:
				break;
			}
		}
	}

	// 몬스터를 업데이트한다.
	{
		// 일반 몬스터를 일정 시간마다 스폰한다.
		mMonsterSpawnTimer += deltaTime;
		if (mMonsterSpawnTimer >= 0.5f)
		{
			for (Monster& monster : mMonsters)
			{
				if (monster.sprite.IsActive())
				{
					monster.backgroundHpBar.SetActive(true);
					monster.hpBar.SetActive(true);
					continue;
				}

				SpawnMonster(
					{
						.monster = &monster,
						.scale {.width = MONSTER_SCALE , .height = MONSTER_SCALE },
						.maxHp = MONSTER_MAX_HP
					}
				);

				monster.moveSpeed = getRandom(10.0f, 80.0f);
				mMonsterSpawnTimer = 0.0f;
				break;
			}
		}

		// 몬스터를 스폰 시 이펙트가 발생한다.
		for (Monster& monster : mMonsters)
		{
			if (monster.state != eMonster_State::Spawn)
			{
				continue;
			}

			UpdateMonsterSpawnEffect(
				{
					.monster = &monster,
					.originalScale { MONSTER_SCALE, MONSTER_SCALE },
					.effectScale{ 3.0f, 3.0f },
					.time = 0.3f,
					.deltaTime = deltaTime
				}
			);

		}

		// 몬스터를 이동시킨다.
		constexpr D2D1_POINT_2F HP_OFFSET = { .x = 3.5f, .y = 10.0f };
		for (Monster& monster : mMonsters)
		{
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			Sprite& sprite = monster.sprite;
			D2D1_POINT_2F position = sprite.GetPosition();

			D2D1_POINT_2F direction = Math::SubtractVector({}, position);
			direction = Math::NormalizeVector(direction);
			const D2D1_POINT_2F velocity = Math::ScaleVector(direction, monster.moveSpeed * deltaTime);

			position = Math::AddVector(position, velocity);
			sprite.SetPosition(position);

			// hp를 좌표를 업데이트한다.
			{
				const D2D1_SIZE_F scaleOffset =
				{
					.width = sprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f,
					.height = sprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f
				};

				const D2D1_POINT_2F offset =
				{
					.x = position.x - scaleOffset.width + HP_OFFSET.x,
					.y = position.y - scaleOffset.height - HP_OFFSET.y
				};

				monster.backgroundHpBar.SetPosition(offset);
				monster.hpBar.SetPosition(offset);
			}
		}

		// 몬스터와 총알이 충돌하면, 총알 이펙트가 생성된다.		
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Monster& monster = mMonsters[i];
			if (monster.state == eMonster_State::Dead)
			{
				continue;
			}

			if (not monster.isBulletColliding)
			{
				continue;
			}

			D2D1_POINT_2F position = monster.sprite.GetPosition();
			Sprite& effect = mMonsterToBulletEffects[i];
			effect.SetPosition(position);
			effect.SetActive(true);

			const D2D1_POINT_2F direction = Math::NormalizeVector(monster.sprite.GetPosition());
			float angle = Math::ConvertRadianToDegree(direction.y);
			effect.SetAngle(-angle);

			// 이펙트가 생성된다.
			mMonsterToBulletEffectsTimers[i] += deltaTime;
			float t = mMonsterToBulletEffectsTimers[i] / MONSTER_DIE_EFFECT_TIME;
			D2D1_POINT_2F scale = Math::LerpVector({ .x = MONSTER_TO_BULLET_EFFECT_SCALE.width, .y = MONSTER_TO_BULLET_EFFECT_SCALE.height },
				{ .x = 0.1f, .y = MONSTER_TO_BULLET_EFFECT_SCALE.height }, t);
			effect.SetScale({ .width = scale.x, .height = scale.y });

			if (t >= 1.0f)
			{
				monster.isBulletColliding = false;
				effect.SetActive(false);
				mMonsterToBulletEffectsTimers[i] = 0.0f;
			}

			break;
		}

		for (Monster& monster : mMonsters)
		{
			// 체력바를 업데이트한다.
			UpdateMonsterHp(&monster, MONSTER_HP_BAR_WIDTH, MONSTER_MAX_HP, deltaTime);

			// 몬스터가 죽으면 이펙트가 생성된다.
			if (monster.hp <= 0
				and monster.state == eMonster_State::Life)
			{
				monster.state == eMonster_State::Dead;
			}

			MonsterDeadEffect(
				{
					.monster = &monster,
					.sound = &mMonsterDeadSound,
					.startScale = {.width = 3.0f, .height = 3.0f },
					.endScale = {.width = 0.1f, .height = 0.1f },
					.time = MONSTER_DIE_EFFECT_TIME,
					.deltaTime = deltaTime
				}
			);
		}
	}

	// 돌진 몬스터를 업데이트한다.
	//{
		// 몬스터를 일정 시간마다 스폰한다.
	mRunMonsterSpawnTimer += deltaTime;
	if (mRunMonsterSpawnTimer >= 0.5f)
	{
		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Monster& monster = mRunMonsters[i];
			Sprite& sprite = monster.sprite;

			if (sprite.IsActive())
			{
				continue;
			}

			SpawnMonster(
				{
					.monster = &monster,
					.scale {.width = RUN_MONSTER_SCALE , .height = RUN_MONSTER_SCALE },
					/*.hpOffset { 0.0f, -10.0f },*/
					.maxHp = RUN_MONSTER_MAX_HP
				}
			);

			// 출발바를 생성한다.
			mRunMonsterisMoveables[i] = false;
			mRunMonsterStartBars[i].SetPosition({ .x = sprite.GetPosition().x - 10.0f, .y = sprite.GetPosition().y - 20.0f });
			mRunMonsterStartBars[i].SetScale({ .width = 0.0f, .height = 0.1f });
			mRunMonsterStartBars[i].SetActive(true);

			mRunMonsterSpawnTimer = 0.0f;
			break;
		}
	}

	// 돌진 몬스터가 스폰되면, 커졌다가 작아진다.
	for (Monster& monster : mRunMonsters)
	{
		if (monster.state != eMonster_State::Spawn)
		{
			continue;
		}

		UpdateMonsterSpawnEffect(
			{
				.monster = &monster,
				.originalScale { RUN_MONSTER_SCALE, RUN_MONSTER_SCALE },
				.effectScale{ 3.0f, 3.0f },
				.time = 0.5f,
				.deltaTime = deltaTime
			}
		);
	}

	// 돌진 몬스터가 이동한다.
	for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
	{
		Monster& monster = mRunMonsters[i];
		Sprite& sprite = monster.sprite;

		if (monster.state != eMonster_State::Life)
		{
			continue;
		}

		if (not mRunMonsterisMoveables[i])
		{
			// 출발바가 꽉 차면 돌진 몬스터는 이동한다.
			constexpr float START_COOL_TIME = 2.0f;
			float barSpeed = RUN_MONSTER_START_BAR_WIDTH / START_COOL_TIME;

			D2D1_SIZE_F scale = mRunMonsterStartBars[i].GetScale();
			if (scale.width < RUN_MONSTER_START_BAR_WIDTH)
			{
				scale.width += barSpeed * deltaTime;
				mRunMonsterStartBars[i].SetScale(scale);

				continue;
			}
			else
			{
				scale.width = RUN_MONSTER_START_BAR_WIDTH;
				mRunMonsterStartBars[i].SetScale(scale);

				const D2D1_POINT_2F monsterPosition = sprite.GetPosition();
				const D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();

				mRunMonsterMoveSpeeds[i] = 0.0f;

				mRunMonsterMoveDirections[i] = Math::SubtractVector(heroPosition, monsterPosition);
				mRunMonsterMoveDirections[i] = Math::NormalizeVector(mRunMonsterMoveDirections[i]);

				mRunMonsterStartBars[i].SetActive(false);
				mRunMonsterisMoveables[i] = true;

				// hp바를 생성한다.
				monster.backgroundHpBar.SetActive(true);
				monster.hpBar.SetActive(true);
			}
		}

		constexpr float MOVE_ACC = 5.0f;
		mRunMonsterMoveSpeeds[i] = min(mRunMonsterMoveSpeeds[i] + MOVE_ACC, 400.0f);
		D2D1_POINT_2F velocity = Math::ScaleVector(mRunMonsterMoveDirections[i], mRunMonsterMoveSpeeds[i] * deltaTime);

		D2D1_POINT_2F position = sprite.GetPosition();
		position = Math::AddVector(position, velocity);
		sprite.SetPosition(position);

		// hp를 좌표를 업데이트한다.
		const D2D1_SIZE_F scaleOffset =
		{
			.width = sprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f,
			.height = sprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f
		};

		const D2D1_POINT_2F offset =
		{
			.x = position.x - scaleOffset.width,
			.y = position.y - scaleOffset.height - 10.0f
		};

		monster.backgroundHpBar.SetPosition(offset);
		monster.hpBar.SetPosition(offset);
	}

	// 돌진 몬스터와 총알이 충돌하면, 총알 이펙트가 생성된다.
	for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
	{
		Monster& monster = mRunMonsters[i];
		if (monster.state == eMonster_State::Dead)
		{
			continue;
		}

		if (not monster.isBulletColliding)
		{
			continue;
		}

		BulletEffect& effect = mRunMonsterToBulletEffects[i];

		effect.timer += deltaTime;

		// 크기를 보간한다.
		D2D1_POINT_2F scale = { effect.scale.width, effect.scale.height };
		scale = Math::LerpVector(scale, { 80.0f , 80.0f }, 5.0f * deltaTime);
		effect.scale = { scale.x, scale.y };

		// 두께를 보간한다.
		effect.thick = { .x = 50.0f, .y = 50.0f };
		float t = effect.timer / (RUN_MONSTER_DIE_EFFECT_TIME - 0.1f);
		t = std::clamp(t, 0.0f, 1.0f);
		effect.thick = Math::LerpVector(effect.thick, { 0.1f , 0.1f }, t);

		if (t >= 1.0f)
		{
			monster.isBulletColliding = false;
			effect.timer = 0.0f;
		}

		break;
	}

	for (Monster& monster : mRunMonsters)
	{
		// 체력바를 업데이트한다.
		UpdateMonsterHp(&monster, RUN_MONSTER_HP_BAR_WIDTH, RUN_MONSTER_MAX_HP, deltaTime);


		// 몬스터가 죽으면 이펙트가 생성된다.
		if (monster.hp <= 0
			and monster.state == eMonster_State::Life)
		{
			monster.state == eMonster_State::Dead;
		}

		MonsterDeadEffect(
			{
				.monster = &monster,
				.sound = &mRunMonsterDeadSound,
				.startScale = {.width = RUN_MONSTER_SCALE, .height = RUN_MONSTER_SCALE },
				.endScale = {.width = 0.1f, .height = 0.1f },
				.time = RUN_MONSTER_DIE_EFFECT_TIME,
				.deltaTime = deltaTime
			}
		);
	}

	// 느린 몬스터를 업데이트한다.
	{
		// 몬스터를 일정 시간마다 스폰한다.
		mSlowMonsterSpawnTimer += deltaTime;
		if (mSlowMonsterSpawnTimer >= 0.5f)
		{
			for (Monster& monster : mSlowMonsters)
			{
				Sprite& sprite = monster.sprite;
				if (sprite.IsActive())
				{
					continue;
				}

				SpawnMonster(
					{
						.monster = &monster,
						.scale {.width = SLOW_MONSTER_SCALE , .height = SLOW_MONSTER_SCALE },
						.maxHp = SLOW_MONSTER_MAX_HP
					}
				);

				mSlowMonsterSpawnTimer = 0.0f;
				break;
			}
		}

		// 그림자를 업데이트한다.
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			for (Sprite& shadow : mSlowMonsterShadows[i])
			{
				if (not shadow.IsActive())
				{
					continue;
				}

				float opacity = shadow.GetOpacity();
				opacity -= 5.0f * deltaTime;
				shadow.SetOpacity(opacity);

				if (opacity <= 0.0f)
				{
					shadow.SetActive(false);
				}
			}
		}

		// 느린 몬스터가 스폰되면, 커졌다가 작아진다.
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Monster& monster = mSlowMonsters[i];
			if (monster.state != eMonster_State::Spawn)
			{
				continue;
			}

			UpdateMonsterSpawnEffect(
				{
					.monster = &monster,
					.originalScale { SLOW_MONSTER_SCALE, SLOW_MONSTER_SCALE },
					.effectScale{ 2.0f, 2.0f },
					.time = 0.7f,
					.deltaTime = deltaTime
				}
			);

			mSlowMonsterState[i] = eSlow_Monster_State::Stop;
		}

		// 느린 몬스터와 그림자가 이동한다.
		constexpr float LENGTH = 100.0f;
		constexpr float MOVE_TIME = 1.5f;
		constexpr float STOP_TIME = 1.0f;

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Monster& monster = mSlowMonsters[i];
			Sprite& sprite = monster.sprite;

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			// 느린 몬스터가 이동한다.
			switch (mSlowMonsterState[i])
			{
			case eSlow_Monster_State::Moving:
			{
				mSlowMonsterMovingTimers[i] += deltaTime;

				float t = mSlowMonsterMovingTimers[i] / MOVE_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				// 갈수록 느려지는 효과이다.
				float easeOutT = 1.0f - (1.0f - t) * (1.0f - t);

				D2D1_POINT_2F position = Math::LerpVector(mSlowMonsterStartPositions[i], mSlowMonsterEndPositions[i], easeOutT);
				sprite.SetPosition(position);

				if (easeOutT >= 1.0f)
				{
					mSlowMonsterState[i] = eSlow_Monster_State::Stop;
					mSlowMonsterStopTimers[i] = 0.0f;
				}

				break;
			}

			case eSlow_Monster_State::Stop:
			{
				monster.backgroundHpBar.SetActive(true);
				monster.hpBar.SetActive(true);

				mSlowMonsterStopTimers[i] += deltaTime;

				if (mSlowMonsterStopTimers[i] >= STOP_TIME)
				{
					mSlowMonsterShadowCoolTimer = 0.0f;
					mSlowMonsterMovingTimers[i] = 0.0f;

					mSlowMonsterStartPositions[i] = sprite.GetPosition();

					D2D1_POINT_2F direction = Math::SubtractVector({}, mSlowMonsterStartPositions[i]);
					direction = Math::NormalizeVector(direction);

					mSlowMonsterEndPositions[i] = Math::AddVector(mSlowMonsterStartPositions[i], Math::ScaleVector(direction, LENGTH));

					mSlowMonsterState[i] = eSlow_Monster_State::Moving;
				}

				break;
			}
			}

			// 그림자가 이동한다.
			mSlowMonsterShadowCoolTimer -= deltaTime;

			if (mSlowMonsterShadowCoolTimer <= 0.0f)
			{
				for (uint32_t j = 0; j < SHADOW_COUNT; ++j)
				{
					Sprite& shadow = mSlowMonsterShadows[i][j];

					if (mSlowMonsterState[i] == eSlow_Monster_State::Stop)
					{
						continue;
					}

					shadow.SetOpacity(1.0f);
					shadow.SetPosition(sprite.GetPosition());
					shadow.SetActive(true);
					break;
				}

				mSlowMonsterShadowCoolTimer = 0.05f;
			}

			// hp를 좌표를 업데이트한다.
			const D2D1_SIZE_F scaleOffset =
			{
				.width = sprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f,
				.height = sprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f
			};

			const D2D1_POINT_2F offset =
			{
				.x = sprite.GetPosition().x - scaleOffset.width + 2.0f,
				.y = sprite.GetPosition().y - scaleOffset.height - 10.0f
			};

			monster.backgroundHpBar.SetPosition(offset);
			monster.hpBar.SetPosition(offset);
		}

		// 총알 이펙트가 생성된다.
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Monster& slowMonster = mSlowMonsters[i];

			if (slowMonster.state == eMonster_State::Dead)
			{
				continue;
			}

			if (not slowMonster.isBulletColliding)
			{
				continue;
			}

			BulletEffect& effect = mSlowMonsterToBulletEffects[i];

			effect.timer += deltaTime;

			// 크기를 보간한다.
			D2D1_POINT_2F scale = { effect.scale.width, effect.scale.height };
			scale = Math::LerpVector(scale, { 70.0f , 70.0f }, 5.0f * deltaTime);
			effect.scale = { scale.x, scale.y };

			// 두께를 보간한다.
			effect.thick = { .x = 25.0f, .y = 25.0f };
			float t = effect.timer / (SLOW_MONSTER_DIE_EFFECT_TIME - 0.1f);
			t = std::clamp(t, 0.0f, 1.0f);

			effect.thick = Math::LerpVector(effect.thick, { 0.1f , 0.1f }, t);

			if (t >= 1.0f)
			{
				slowMonster.isBulletColliding = false;
				effect.timer = 0.0f;
			}

			break;
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Monster& monster = mSlowMonsters[i];

			// 체력바를 업데이트한다.
			UpdateMonsterHp(&monster, SLOW_MONSTER_HP_BAR_WIDTH, SLOW_MONSTER_MAX_HP, deltaTime);

			// 몬스터가 죽으면 이펙트가 생성된다.
			if (monster.hp <= 0
				and monster.state == eMonster_State::Life)
			{
				monster.state == eMonster_State::Dead;
			}

			MonsterDeadEffect(
				{
					.monster = &monster,
					.sound = &mSlowMonsterDeadSound,
					.startScale = {.width = 3.0f, .height = 3.0f },
					.endScale = {.width = 0.1f, .height = 0.1f },
					.time = SLOW_MONSTER_DIE_EFFECT_TIME,
					.deltaTime = deltaTime
				}
			);

			for (uint32_t j = 0; j < SHADOW_COUNT; ++j)
			{
				mSlowMonsterShadows[i][j].SetActive(false);
			}
		}
	}

	// 플레이어 체력에 관련된 부분을 업데이트한다.
	{
		// 플레이어가 죽었을 때 종료된다.
		if (mHero.hp <= 0)
		{
			mBackgroundSound.Pause();
			mEndingSound.Play();

			mEndingLabel.SetActive(true);

			mHero.hp = 0;
			mHero.velocity = {};
			mHero.sprite.SetActive(false);
		}

		// 플레이어 체력 라벨을 업데이트 한다.
		static int32_t prevHp = mHero.hp;

		if (prevHp != mHero.hp)
		{
			mHeroSound.Replay();

			mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHero.hp) + L" / " + std::to_wstring(HERO_MAX_HP));

			prevHp = mHero.hp;
		}

		// 플레이어 체력바를 업데이트한다.
		D2D1_POINT_2F scale = { mUiHpBar.GetScale().width, mUiHpBar.GetScale().height };
		scale = Math::LerpVector(scale,
			{ UI_HP_SCALE_WIDTH * (float(mHero.hp) / float(HERO_MAX_HP)), scale.y },
			10.0f * deltaTime);
		mUiHpBar.SetScale({ scale.x, scale.y });
	}

	// 플레이어 대쉬바를 업데이트한다.
	{
		static int32_t prevDashCount = mDashCount;

		if (prevDashCount != mDashCount)
		{
			mDashValueLabel.SetText(L"Dash: " + std::to_wstring(mDashCount) + L" / " + std::to_wstring(DASH_MAX_COUNT));

			prevDashCount = mDashCount;
		}

		D2D1_POINT_2F scale = { mUiDashBar.GetScale().width, mUiDashBar.GetScale().height };
		scale = Math::LerpVector(scale, { UI_DASH_SCALE_WIDTH * (float(mDashCount) / float(DASH_MAX_COUNT)),  scale.y }, 8.0f * deltaTime);
		mUiDashBar.SetScale({ scale.x, scale.y });
	}

	// 총알 라벨을 업데이트한다.
	{
		static int32_t prevBulletCount = mBulletValue;

		if (prevBulletCount != mBulletValue)
		{
			mBulletLabel.SetText(std::to_wstring(mBulletValue) + L"/" + std::to_wstring(BULLET_COUNT));

			prevBulletCount = mBulletValue;
		}
	}

	// 타이머 라벨을 업데이트한다.
	{
		if (mHero.hp > 0)
		{
			mGameTimer += deltaTime;
		}

		const uint32_t seconds = uint32_t(mGameTimer) % 60;
		const uint32_t minutes = uint32_t(mGameTimer) / 60;

		std::wstring name = L"Timer: " + std::to_wstring(minutes) + L":" + std::to_wstring(seconds);
		mTimerLabel.SetText(name);
	}

	// 충돌 처리를 업데이트한다.
	{
		// 플레이어 - 외부 원
		{
			if (not Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mHero.sprite), {}, BOUNDARY_RADIUS))
			{
				mHero.velocity = {};

				D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
				const D2D1_POINT_2F direction = Math::NormalizeVector(heroPosition);

				heroPosition = Math::AddVector(heroPosition, Math::ScaleVector(direction, -2.0f));
				mHero.sprite.SetPosition(heroPosition);
			}
		}

		// 플레이어 - 내부 원
		{
			const float offset = IN_BOUNDARY_RADIUS + mHero.sprite.GetScale().height * mPinkRectangleTexture.GetHeight();
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mHero.sprite), {}, offset))
			{
				mHero.velocity = {};

				D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
				const D2D1_POINT_2F direction = Math::NormalizeVector(heroPosition);

				heroPosition = Math::AddVector(heroPosition, Math::ScaleVector(direction, 2.0f));
				mHero.sprite.SetPosition(heroPosition);
			}
		}

		// 총알과 원이 충돌한다.
		for (Bullet& bullet : mBullets)
		{
			Sprite& sprite = bullet.sprite;

			const float halfLength = sprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f;
			const D2D1_POINT_2F endPosition =
			{
				.x = sprite.GetPosition().x + bullet.direction.x * halfLength,
				.y = sprite.GetPosition().y + bullet.direction.y * halfLength
			};

			Line line =
			{
				.Point0 = bullet.prevPosition,
				.Point1 = endPosition
			};

			bool isBoundryToBullet = Collision::IsCollidedCircleWithLine({}, BOUNDARY_RADIUS, line);
			if (not isBoundryToBullet)
			{
				sprite.SetActive(false);
			}

			const float offset = IN_BOUNDARY_RADIUS + sprite.GetScale().height * mRedRectangleTexture.GetHeight();
			bool isInBoundryToBullet = Collision::IsCollidedCircleWithLine({}, offset, line);
			if (isInBoundryToBullet)
			{
				sprite.SetActive(false);
			}
		}
		static int frame = 0;
		for (Monster& monster : mMonsters)
		{
			Sprite& sprite = monster.sprite;
			if (not sprite.IsActive())
			{
				continue;
			}

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			// 몬스터 - 내부 바운더리
			{
				const float offset = IN_BOUNDARY_RADIUS + monster.sprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f;
				const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), {}, offset);

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}

			// 몬스터 - 플레이어
			{
				const bool isCollision = Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero.sprite), getRectangleFromSprite(monster.sprite));
				if (isCollision and monster.hp > 0)
				{
					monster.hp -= MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}
		}

		for (Monster& monster : mRunMonsters)
		{
			Sprite& sprite = monster.sprite;
			if (not sprite.IsActive())
			{
				continue;
			}

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			// 돌진 몬스터 - 내부 바운더리
			{
				const float offset = IN_BOUNDARY_RADIUS + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
				const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), {}, offset);

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= RUN_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}

			// 돌진 몬스터 - 외부 바운더리
			{
				const bool isCollision = not (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), {}, BOUNDARY_RADIUS));

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= RUN_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}

			// 몬스터 - 플레이어
			{
				const bool isCollision = Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero.sprite), getRectangleFromSprite(monster.sprite));

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= RUN_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}
		}

		for (Monster& monster : mSlowMonsters)
		{
			Sprite& sprite = monster.sprite;
			if (not sprite.IsActive())
			{
				continue;
			}

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			// 느린 몬스터 - 내부 바운더리
			{
				const float offset = IN_BOUNDARY_RADIUS + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
				const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), {}, offset);

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= SLOW_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}

			// 느린 몬스터 - 플레이어
			{
				const bool isCollision = Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero.sprite), getRectangleFromSprite(monster.sprite));

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= SLOW_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}
		}

		// 총알과 모든 몬스터 충돌을 검사한다.
		for (Bullet& bullet : mBullets)
		{
			Sprite& bulletSprite = bullet.sprite;
			if (not bulletSprite.IsActive())
			{
				continue;
			}

			// 이전 좌표와 현재 좌표의 직선을 그려서 충돌체크를 한다.
			const float halfLength = bulletSprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f;
			const D2D1_POINT_2F endPosition =
			{
				.x = bulletSprite.GetPosition().x + bullet.direction.x * halfLength,
				.y = bulletSprite.GetPosition().y + bullet.direction.y * halfLength
			};

			const Line line =
			{
				.Point0 = bullet.prevPosition,
				.Point1 = endPosition
			};

			// 총알과 기본 몬스터 충돌을 검사한다.
			Sprite* targetMonster = nullptr;
			float targetMonsterDistance = 999.9f;

			for (Monster& monster : mMonsters)
			{
				if (monster.state != eMonster_State::Life)
				{
					continue;
				}

				Sprite& monsterSprite = monster.sprite;
				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(monsterSprite), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(bullet.prevPosition, monsterSprite.GetPosition()));
				if (distance < targetMonsterDistance)
				{
					monster.hp -= BULLET_ATTACK_VALUE;
					monster.isBulletColliding = true;

					targetMonster = &monsterSprite;
					targetMonsterDistance = distance;
				}
			}

			// 돌진 몬스터를 계산한다.
			Sprite* targetRunMonster = nullptr;
			float targetRunMonsterDistance = 999.9f;

			for (uint32_t j = 0; j < RUN_MONSTER_COUNT; ++j)
			{
				Monster& runMonster = mRunMonsters[j];
				Sprite& runMonsterSprite = runMonster.sprite;

				if (not runMonsterSprite.IsActive())
				{
					continue;
				}

				if (runMonster.state != eMonster_State::Life)
				{
					continue;
				}

				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(runMonsterSprite), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(bullet.prevPosition, runMonsterSprite.GetPosition()));
				if (distance < targetRunMonsterDistance)
				{
					runMonster.hp -= BULLET_ATTACK_VALUE;
					runMonster.isBulletColliding = true;

					targetRunMonster = &runMonsterSprite;
					targetRunMonsterDistance = distance;
				}
			}

			// 느린 몬스터를 계산한다
			Sprite* targetSlowMonster = nullptr;
			float targetSlowMonsterDistance = 999.9f;

			for (uint32_t j = 0; j < SLOW_MONSTER_COUNT; ++j)
			{
				Monster& slowMonster = mSlowMonsters[j];
				Sprite& slowMonsterSprite = slowMonster.sprite;

				if (not slowMonsterSprite.IsActive())
				{
					continue;
				}

				if (slowMonster.state != eMonster_State::Life)
				{
					continue;
				}

				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(slowMonsterSprite), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(bullet.prevPosition, slowMonsterSprite.GetPosition()));
				if (distance < targetMonsterDistance)
				{
					slowMonster.hp -= BULLET_ATTACK_VALUE;
					slowMonster.isBulletColliding = true;

					targetSlowMonster = &slowMonsterSprite;
					targetSlowMonsterDistance = distance;
				}
			}

			if (targetMonster != nullptr
				or targetRunMonster != nullptr
				or targetSlowMonster != nullptr)
			{
				bullet.sprite.SetActive(false);
			}
		}

		// 플레이어 쉴드와 몬스터가 충돌하면 몬스터는 삭제된다.
		for (Monster& monster : mMonsters)
		{
			if (mShieldState != eShield_State::Growing
				and mShieldState != eShield_State::Waiting)
			{
				continue;
			}

			const float offset = mShieldScale.width * 0.5f + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), mHero.sprite.GetPosition(), offset))
			{
				monster.hp = 0;
			}
		}

		for (Monster& monster : mRunMonsters)
		{
			if (mShieldState != eShield_State::Growing
				and mShieldState != eShield_State::Waiting)
			{
				continue;
			}

			const float offset = mShieldScale.width * 0.5f + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), mHero.sprite.GetPosition(), offset))
			{
				monster.hp = 0;
			}
		}

		for (Monster& monster : mSlowMonsters)
		{
			if (mShieldState != eShield_State::Growing
				and mShieldState != eShield_State::Waiting)
			{
				continue;
			}

			const float offset = mShieldScale.width * 0.5f + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), mHero.sprite.GetPosition(), offset))
			{
				monster.hp = 0;
			}
		}

		// 플레이어 주변을 공전하는 원과 몬스터가 충돌하면 몬스터는 삭제된다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			if (mOrbitState != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::AddVector(mOrbitEllipse.point, mHero.sprite.GetPosition());
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mMonsters[i].sprite), center, mOrbitEllipse.radiusX))
			{
				mMonsters[i].hp = 0;
			}
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			if (mOrbitState != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::AddVector(mOrbitEllipse.point, mHero.sprite.GetPosition());
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mRunMonsters[i].sprite), center, mOrbitEllipse.radiusX))
			{
				mRunMonsters[i].hp = 0;
			}
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			if (mOrbitState != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::AddVector(mOrbitEllipse.point, mHero.sprite.GetPosition());
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mSlowMonsters[i].sprite), center, mOrbitEllipse.radiusX))
			{
				mSlowMonsters[i].hp = 0;
			}
		}
	}

	// 카메라를 업데이트한다.
	{
		D2D1_POINT_2F position = mMainCamera.GetPosition();
		const D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
		position = Math::LerpVector(position, heroPosition, 8.0f * deltaTime);

		if (mCameraShakeAmplitude > 0.0f)
		{
			const D2D1_POINT_2F offset = updateCameraShake(deltaTime);
			position = Math::AddVector(position, offset);
		}

		mMainCamera.SetPosition(position);
	}

	return true;
}

void MainScene::PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
	ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();

	// Hero 쉴드 스킬을 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix(mHero.sprite.GetPosition()) * view;
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse =
		{
			.radiusX = mShieldScale.width * 0.5f,
			.radiusY = mShieldScale.height * 0.5f
		};

		if (mShieldState == eShield_State::Growing or mShieldState == eShield_State::Waiting
			and mShieldBlinkOn)
		{
			renderTarget->DrawEllipse(ellipse, mYellowBrush, 10.0f);
		}
	}

	// Hero 공전 스킬을 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix(mHero.sprite.GetPosition()) * view;
		renderTarget->SetTransform(worldView);

		if (mOrbitState == eOrbit_State::Rotating
			and mOrbitBlinkOn)
		{
			renderTarget->DrawEllipse(mOrbitEllipse, mYellowBrush, 5.0f);
		}
	}

	// 총알과 돌진 몬스터가 충돌하면, 이펙트를 그린다.
	{
		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Monster& monster = mRunMonsters[i];

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			if (not monster.isBulletColliding)
			{
				continue;
			}

			Sprite& sprite = monster.sprite;

			const Matrix3x2F worldView =
				Transformation::getWorldMatrix(
					{
						.x = sprite.GetPosition().x,
						.y = sprite.GetPosition().y + 50.0f
					}, 45.0f) * view;
			renderTarget->SetTransform(worldView);

			const BulletEffect effect = mRunMonsterToBulletEffects[i];

			const D2D1_RECT_F colliderSize =
			{
				.left = 0.0f,
				.top = 0.0f,
				.right = effect.scale.width,
				.bottom = effect.scale.height
			};

			renderTarget->DrawRectangle(colliderSize, mCyanBrush, effect.thick.x);
		}
	}

	// 총알과 느린 몬스터가 충돌하면, 이펙트를 그린다.
	{
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Monster& monster = mSlowMonsters[i];

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			if (not monster.isBulletColliding)
			{
				continue;
			}

			Sprite& slowMonsterSprite = mSlowMonsters[i].sprite;

			const Matrix3x2F worldView =
				Transformation::getWorldMatrix(
					{
						.x = slowMonsterSprite.GetPosition().x,
						.y = slowMonsterSprite.GetPosition().y + 50.0f
					}, 45.0f) * view;
			renderTarget->SetTransform(worldView);

			const BulletEffect effect = mSlowMonsterToBulletEffects[i];

			const D2D1_RECT_F colliderSize =
			{
				.left = 0.0f,
				.top = 0.0f,
				.right = effect.scale.width,
				.bottom = effect.scale.height
			};

			renderTarget->DrawRectangle(colliderSize, mCyanBrush, effect.thick.x);
		}
	}

	// 라인을 그린다.
	{
		//const D2D1_ELLIPSE CIRCLE{ .radiusX = TEST_RADIUS, .radiusY = TEST_RADIUS };

		//Matrix3x2F point0WorldView = Transformation::getWorldMatrix(mLine.Point0) * view;
		//Matrix3x2F point1WorldView = Transformation::getWorldMatrix(mLine.Point1) * view;

		//// 두 점을 그린다.
		//{
		//	renderTarget->SetTransform(point0WorldView);
		//	renderTarget->DrawEllipse(CIRCLE, mDefaultBrush);

		//	renderTarget->SetTransform(point1WorldView);
		//	renderTarget->DrawEllipse(CIRCLE, mDefaultBrush);
		//}

		//// 라인을 그린다.
		//{
		//	const D2D1_POINT_2F point0 = D2D1_POINT_2F{ .x = 0.0f, .y = 0.0f } *point0WorldView;
		//	const D2D1_POINT_2F point1 = D2D1_POINT_2F{ .x = 0.0f, .y = 0.0f } *point1WorldView;
		//	renderTarget->SetTransform(Matrix3x2F::Identity());
		//	renderTarget->DrawLine(point0, point1, mDefaultBrush);
		//}
	}

	// 몬스터를 그린다.
	//{
		//for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		//{
		//	D2D1_POINT_2F spawnDirection = Math::GetNormalizeVector(mMonsters[i].GetPosition());
		//	D2D1_POINT_2F spawnPositionInCircle = Math::ScaleVector(spawnDirection, 350.0f);

		//	Matrix3x2F worldView = Transformation::getWorldMatrix(spawnPositionInCircle) * view;
		//	renderTarget->SetTransform(worldView);

		//	D2D1_ELLIPSE CIRCLE{ .radiusX = 30.0f, .radiusY = 30.0f };
		//	renderTarget->DrawEllipse(CIRCLE, mDefaultBrush);
		//}
	//}

	// 몬스터 충돌박스를 그린다.
	{
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monsterSprite = mMonsters[i].sprite;

			if (mIsColliderKeyDown and monsterSprite.IsActive())
			{
				const Matrix3x2F worldView = Transformation::getWorldMatrix(
					{
						.x = getRectangleFromSprite(monsterSprite).left,
						.y = getRectangleFromSprite(monsterSprite).top
					}) * view;
				renderTarget->SetTransform(worldView);

				const D2D1_SIZE_F scale = monsterSprite.GetScale();

				const D2D1_RECT_F colliderSize =
				{
					.left = 0.0f,
					.top = 0.0f,
					.right = scale.width * mRectangleTexture.GetWidth(),
					.bottom = scale.width * mRectangleTexture.GetWidth()
				};

				renderTarget->DrawRectangle(colliderSize, mCyanBrush);
			}
		}
	}

	// 총알 충돌박스를 그린다.
	{
		for (uint32_t i = 0; i < BULLET_COUNT; ++i)
		{
			Sprite& bulletSprite = mBullets[i].sprite;

			if (mIsColliderKeyDown and bulletSprite.IsActive())
			{
				const Matrix3x2F worldView = Transformation::getWorldMatrix(getCircleFromSprite(bulletSprite).point) * view;
				renderTarget->SetTransform(worldView);

				const D2D1_SIZE_F scale = bulletSprite.GetScale();

				const D2D1_ELLIPSE circleSize =
				{
					.radiusX = scale.width * mCircleTexture.GetWidth() * 0.5f,
					.radiusY = scale.height * mCircleTexture.GetHeight() * 0.5f
				};

				renderTarget->DrawEllipse(circleSize, mYellowBrush);
			}
		}
	}

	// 쉴드 스킬 UI를 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix
		(
			{
				.x = float(Constant::Get().GetWidth()) * 0.5f,
				.y = 110.0f
			}
		);
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse{ .radiusX = 25.0f, .radiusY = 25.0f };
		renderTarget->DrawEllipse(ellipse, mYellowBrush, 2.0f);
	}
}

void MainScene::Finalize()
{
	RELEASE_D2D1(mCyanBrush);
	RELEASE_D2D1(mYellowBrush);
	RELEASE_D2D1(mDefaultBrush);

	mRectangleTexture.Finalize();
	mRedRectangleTexture.Finalize();
	mBlueRectangleTexture.Finalize();
	mPinkRectangleTexture.Finalize();
	mSkyBlueRectangleTexture.Finalize();

	mCircleTexture.Finalize();
	mRedCircleTexture.Finalize();

	mWhiteBarTexture.Finalize();
	mRedBarTexture.Finalize();
	mYellowBarTexture.Finalize();
}

D2D1_RECT_F MainScene::getRectangleFromSprite(const Sprite& sprite)
{
	const D2D1_SIZE_F scale = sprite.GetScale();

	const D2D1_SIZE_F offset =
	{
		.width = scale.width * mRectangleTexture.GetWidth() * 0.5f,
		.height = scale.height * mRectangleTexture.GetHeight() * 0.5f
	};

	const D2D1_POINT_2F position = sprite.GetPosition();

	const D2D1_RECT_F rect =
	{
		.left = position.x - offset.width,
		.top = position.y + offset.height,
		.right = position.x + offset.width,
		.bottom = position.y - offset.height
	};

	return rect;
}

D2D1_ELLIPSE MainScene::getCircleFromSprite(const Sprite& sprite)
{
	const D2D1_RECT_F rect = getRectangleFromSprite(sprite);
	const D2D1_POINT_2F position = sprite.GetPosition();

	const D2D1_ELLIPSE circle =
	{
		.point = position,
		.radiusX = (rect.right - position.x),
		.radiusY = rect.top - position.y
	};

	return circle;
}

float MainScene::getRandom(const float min, const float max)
{
	const float result = float(rand()) / RAND_MAX * (max - min) + min;
	return result;
}

uint32_t MainScene::getRandom(const uint32_t min, const uint32_t max)
{
	const uint32_t result = rand() % (max - min + 1) + min;
	return result;
}

D2D1_POINT_2F MainScene::getMouseWorldPosition() const
{
	const D2D1_POINT_2F zoomPosition = mZoom.GetPosition();
	const D2D1_POINT_2F cameraPosition = mMainCamera.GetPosition();

	const D2D1_POINT_2F result = Math::AddVector(zoomPosition, cameraPosition);

	return result;
}

void MainScene::initializeCameraShake(const float amplitude, const float duration, const float frequency)
{
	mCameraShakeTime = 0.0f;
	mCameraShakeTimer = 0.0f;

	//if (amplitude > mCameraShakeAmplitude)
	{
		mCameraShakeAmplitude = amplitude;
	}
	//if (duration > mCameraShakeDuration)
	{
		mCameraShakeDuration = duration;
	}
	//if (frequency > mCameraShakeFrequency)
	{
		mCameraShakeFrequency = frequency;
	}
}

D2D1_POINT_2F MainScene::updateCameraShake(const float deltaTime)
{
	mCameraShakeTime += deltaTime;
	if (mCameraShakeTime >= mCameraShakeDuration)
	{
		initializeCameraShake(0.0f, 0.0f, 0.0f);

		return D2D1_POINT_2F{};
	}

	mCameraShakeTimer += deltaTime;

	if (const float period = (1.0f / mCameraShakeFrequency);
		mCameraShakeTimer >= period)
	{
		float progress = mCameraShakeTime / mCameraShakeDuration;
		float strength = std::pow(1.0f - progress, 2.0f) * mCameraShakeAmplitude;

		const float radian = getRandom(0.0f, 2.0f * Math::PI);
		D2D1_POINT_2F direction = { .x = cos(radian) * 1.2f, .y = sin(radian) * 0.8f };

		mCameraShakeTimer = 0.0f;

		D2D1_POINT_2F offset = Math::ScaleVector(direction, strength);
		return offset;
	}

	return D2D1_POINT_2F{};
}

void MainScene::SpawnMonster(const MonsterSpawnDesc& desc)
{
	Monster* monster = desc.monster;
	D2D1_SIZE_F scale = desc.scale;
	int32_t maxHp = desc.maxHp;

	// 초기 정보를 업데이트한다.
	monster->state = eMonster_State::Spawn;
	monster->hp = maxHp;

	monster->backgroundHpBar.SetActive(false);
	monster->hpBar.SetActive(false);

	constexpr float MIN_ANGLE = 0.0f;
	constexpr float MAX_ANGLE = 2.0f * Math::PI;
	const float angle = getRandom(MIN_ANGLE, MAX_ANGLE);

	const D2D1_POINT_2F spawnDirection =
	{
		.x = cos(angle),
		.y = sin(angle)
	};

	const float SPAWN_DISTANCE = BOUNDARY_RADIUS - 30.0f;
	const D2D1_POINT_2F spawnPosition = Math::ScaleVector(spawnDirection, SPAWN_DISTANCE);

	Sprite& sprite = monster->sprite;
	sprite.SetPosition(spawnPosition);
	sprite.SetScale(scale);
	sprite.SetActive(true);
}

void MainScene::UpdateMonsterSpawnEffect(const MonsterSpawnEffectDesc& desc)
{
	Monster* monster = desc.monster;
	const D2D1_SIZE_F originalScale = desc.originalScale;
	const D2D1_SIZE_F effectScale = desc.effectScale;
	const float time = desc.time;
	const float deltaTime = desc.deltaTime;

	monster->spawnStartEffectTimer += desc.deltaTime;
	float startT = monster->spawnStartEffectTimer / time;
	startT = std::clamp(startT, 0.0f, 1.0f);
	D2D1_POINT_2F scale = Math::LerpVector({ monster->sprite.GetScale().width, monster->sprite.GetScale().height }, { effectScale.width, effectScale.height }, startT);

	if (startT >= 1.0f)
	{
		monster->spawnStartEffectTimer = 0.0f;
	}

	monster->spawnEndEffectTimer += desc.deltaTime;
	float endT = monster->spawnEndEffectTimer / time;
	endT = std::clamp(endT, 0.0f, 1.0f);
	scale = Math::LerpVector({ scale.x, scale.y }, { originalScale.width, originalScale.height }, endT);

	if (endT >= 1.0f)
	{
		monster->state = eMonster_State::Life;
		monster->spawnEndEffectTimer = 0.0f;
	}

	monster->sprite.SetScale({ scale.x, scale.y });
}

void MainScene::UpdateMonsterHp(Monster* monster, const float maxWidthBar, const uint32_t maxHp, const float deltaTime)
{
	ASSERT(monster != nullptr);

	Sprite& hpBar = monster->hpBar;
	D2D1_POINT_2F scale = { .x = hpBar.GetScale().width, .y = hpBar.GetScale().height };
	scale = Math::LerpVector(scale, { maxWidthBar * (float(monster->hp) / float(maxHp)), hpBar.GetScale().height }, 10.0f * deltaTime);
	hpBar.SetScale({ scale.x, scale.y });
}

void MainScene::MonsterDeadEffect(const MonsterDeadSoundDesc& desc)
{
	Monster* monster = desc.monster;
	Sound* sound = desc.sound;
	const D2D1_SIZE_F startScale = desc.startScale;
	const D2D1_SIZE_F endScale = desc.endScale;
	const float time = desc.time;
	const float deltaTime = desc.deltaTime;

	if (monster->hp <= 0)
	{
		// 카메라 흔들기를 시작한다.
		const float amplitude = Constant::Get().GetHeight() * getRandom(0.008f, 0.012f);
		const float duration = getRandom(0.5f, 0.8f);
		const float frequency = getRandom(50.0f, 60.0f);
		initializeCameraShake(amplitude, duration, frequency);

		monster->hp = 0;
		sound->Replay();

		monster->backgroundHpBar.SetActive(false);
		monster->hpBar.SetActive(false);

		monster->deadEffectTimer += deltaTime;

		float t = monster->deadEffectTimer / time;
		t = std::clamp(t, 0.0f, 1.0f);

		D2D1_POINT_2F scale = Math::LerpVector({ .x = startScale.width, .y = startScale.height }, { .x = endScale.width, .y = endScale.height }, t);
		monster->sprite.SetScale({ scale.x , scale.y });

		if (t >= 1.0f)
		{
			monster->sprite.SetActive(false);
			monster->deadEffectTimer = 0.0f;
		}
	}
}