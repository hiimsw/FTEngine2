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
		mHero.SetPosition({ .x = -200.0f, .y = 0.0f });
		mHero.SetScale({ .width = 0.75f, .height = 0.75f });
		mHero.SetTexture(&mPinkRectangleTexture);
		mSpriteLayers[uint32_t(Layer::Player)].push_back(&mHero);

		for (Sprite& shadow : mDashShadows)
		{
			shadow.SetScale(mHero.GetScale());
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

			sprite.SetPosition(mHero.GetPosition());
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
			monster.state = eMonster_State::Dead;

			Sprite& sprite = monster.sprite;
			sprite.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });
			sprite.SetActive(false);
			sprite.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&sprite);

			// 체력바 배경 정보를 초기화한다.
			Sprite& background = monster.backgroundHpBar;
			background.SetScale({ .width = MONSTER_HP_BAR_WIDTH, .height = 0.7f });
			background.SetCenter({ -0.5f, 0.0f });
			background.SetActive(false);
			background.SetTexture(&mWhiteBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&background);

			// 체력바 정보를 초기화한다.
			Sprite& hpBar = monster.hpBar;
			hpBar.SetScale({ .width = MONSTER_HP_BAR_WIDTH, .height = 0.7f });
			hpBar.SetCenter({ -0.5f, 0.0f });
			hpBar.SetActive(false);
			hpBar.SetTexture(&mRedBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBar);
		}
	}

	// 돌진 몬스터를 초기화한다.
	{
		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Sprite& monster = mRunMonsters[i].sprite;

			mRunMonsters[i].hpValue = MONSTER_MAX_HP;
			mRunMonsters[i].state = eMonster_State::Dead;

			monster.SetScale({ .width = RUN_MONSTER_SCALE, .height = RUN_MONSTER_SCALE });
			monster.SetActive(false);
			monster.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&monster);
		}

		constexpr D2D1_POINT_2F BAR_SCALE = { .x = RUN_MONSTER_START_BAR_WIDTH, .y = 0.1f };

		// 시작바를 생성한다.
		for (Sprite& bar : mRunMonsterStartBars)
		{
			bar.SetScale({ .width = 0.0f, .height = 0.1f });
			bar.SetCenter({ .x = -0.5f, .y = 0.0f });
			bar.SetActive(false);
			bar.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&bar);
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Sprite& background = mRunMonsters[i].backgroundHpBar;

			background.SetScale({ .width = RUN_MONSTER_HP_BAR_WIDTH, .height = 0.5f });
			background.SetCenter({ -0.5f, 0.0f });
			background.SetActive(false);
			background.SetTexture(&mWhiteBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&background);
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Sprite& hpBar = mRunMonsters[i].hpBar;

			hpBar.SetScale({ .width = RUN_MONSTER_HP_BAR_WIDTH, .height = 0.5f });
			hpBar.SetCenter({ -0.5f, 0.0f });
			hpBar.SetActive(false);
			hpBar.SetTexture(&mRedBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBar);
		}
	}

	// 느린 몬스터를 초기화한다.
	{
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Sprite& monster = mSlowMonsters[i].sprite;

			monster.SetScale({ .width = SLOW_MONSTER_SCALE, .height = SLOW_MONSTER_SCALE });
			monster.SetActive(false);
			monster.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&monster);
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

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Sprite& background = mSlowMonsters[i].backgroundHpBar;

			background.SetScale({ .width = SLOW_MONSTER_HP_BAR_WIDTH, .height = 0.5f });
			background.SetCenter({ -0.5f, 0.0f });
			background.SetActive(false);
			background.SetTexture(&mWhiteBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&background);
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Sprite& hpBar = mSlowMonsters[i].hpBar;

			hpBar.SetScale({ .width = SLOW_MONSTER_HP_BAR_WIDTH, .height = 0.5f });
			hpBar.SetCenter({ -0.5f, 0.0f });
			hpBar.SetActive(false);
			hpBar.SetTexture(&mRedBarTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBar);
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
		const D2D1_POINT_2F heroPosition = mHero.GetPosition();
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
			mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHeroHpValue) + L" / " + std::to_wstring(HERO_MAX_HP));
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

		const D2D1_POINT_2F heroPosition = mHero.GetPosition();
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

			if (moveX != 0)
			{
				mHeroVelocity.x = std::clamp(mHeroVelocity.x + ACC * moveX, -MAX_SPEED, MAX_SPEED);
				previousMoveX = moveX;
			}
			else
			{
				if (previousMoveX > 0)
				{
					mHeroVelocity.x = max(mHeroVelocity.x - ACC, 0.0f);
				}
				else
				{
					mHeroVelocity.x = min(mHeroVelocity.x + ACC, 0.0f);
				}
			}

			if (moveY != 0)
			{
				mHeroVelocity.y = std::clamp(mHeroVelocity.y + ACC * moveY, -MAX_SPEED, MAX_SPEED);
				previousMoveY = moveY;
			}
			else
			{
				if (previousMoveY > 0)
				{
					mHeroVelocity.y = max(mHeroVelocity.y - ACC, 0.0f);
				}
				else
				{
					mHeroVelocity.y = min(mHeroVelocity.y + ACC, 0.0f);
				}
			}

			constexpr float MAX_DASH_SPEED = 600.0f;
			constexpr float DASH_ACC = 30.0f;
			static D2D1_POINT_2F dashDirection;

			if (Math::GetVectorLength(mHeroVelocity) != 0.0f)
			{
				float speed = min(Math::GetVectorLength(mHeroVelocity), MAX_SPEED);
				const D2D1_POINT_2F direction = Math::NormalizeVector(mHeroVelocity);
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

				const D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), adjustVelocity);
				mHero.SetPosition(position);
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

				const D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), velocity);
				mHero.SetPosition(position);

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

					const D2D1_POINT_2F spawnPosition = mHero.GetPosition();
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

							D2D1_POINT_2F spawnPosition = mHero.GetPosition();
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
		static float speed[MONSTER_COUNT];

		// 몬스터를 일정 시간마다 스폰한다.
		mMonsterSpawnTimer += deltaTime;
		if (mMonsterSpawnTimer >= 0.5f)
		{
			for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
			{
				Monster& monster = mMonsters[i];

				if (monster.sprite.IsActive())
				{
					continue;
				}

				if (monster.state == eMonster_State::Dead)
				{
					UpdateSpawnMonster(&mMonsters[i], BOUNDARY_RADIUS, { .width = MONSTER_SCALE , .height = MONSTER_SCALE }, { .width = 20.0f, .height = 720.0f },
						{ 3.0f, 10.0f }, MONSTER_MAX_HP, deltaTime);
					speed[i] = getRandom(10.0f, 80.0f);
					mMonsterSpawnTimer = 0.0f;

					break;
				}
			}
		}

		// 몬스터를 스폰 시 이펙트가 발생한다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Monster& monster = mMonsters[i];

			if (not monster.sprite.IsActive())
			{
				continue;
			}

			if (monster.state == eMonster_State::Spawn)
			{
				UpdateSpawnEffectMonster(&monster, { 3.0f, 3.0f }, { MONSTER_SCALE, MONSTER_SCALE }, 0.5f, deltaTime);
			}
		}

		// 몬스터를 이동시킨다.
		constexpr D2D1_POINT_2F HP_OFFSET = { .x = 3.5f, .y = 10.0f };
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Monster& monster = mMonsters[i];
			Sprite& monsterSprite = monster.sprite;

			if (not monsterSprite.IsActive())
			{
				continue;
			}

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			D2D1_POINT_2F position = monsterSprite.GetPosition();
			D2D1_POINT_2F direction = Math::SubtractVector({}, position);
			direction = Math::NormalizeVector(direction);
			const D2D1_POINT_2F velocity = Math::ScaleVector(direction, speed[i] * deltaTime);

			position = Math::AddVector(position, velocity);
			monster.prevPosition = position;
			monsterSprite.SetPosition(position);

			// hp를 좌표를 업데이트한다.
			const D2D1_SIZE_F scaleOffset =
			{
				.width = monsterSprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f,
				.height = monsterSprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f
			};

			const D2D1_POINT_2F monsterPosition = monsterSprite.GetPosition();

			const D2D1_POINT_2F offset =
			{
				.x = monsterPosition.x - scaleOffset.width + HP_OFFSET.x,
				.y = monsterPosition.y - scaleOffset.height - HP_OFFSET.y
			};

			monster.backgroundHpBar.SetPosition(offset);
			monster.hpBar.SetPosition(offset);
		}

		// 몬스터의 체력을 업데이트를 한다.
		static float prevHp[MONSTER_COUNT];

		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Monster& monster = mMonsters[i];

			if (monster.state == eMonster_State::Dead)
			{
				continue;
			}

			if (monster.state == eMonster_State::Life
				and monster.hpValue <= 0)
			{
				mMonsterDeadSound.Replay();

				//monster.HpValue = 0;
			}

			if (prevHp[i] > monster.hpValue)
			{		
				// 카메라 흔들기를 시작한다.
				const float amplitude = Constant::Get().GetHeight() * getRandom(0.008f, 0.012f);
				const float duration = getRandom(0.5f, 0.8f);
				const float frequency = getRandom(50.0f, 60.0f);
				initializeCameraShake(amplitude, duration, frequency);
			}

			prevHp[i] = monster.hpValue;

			Sprite& hpBar = monster.hpBar;
			D2D1_POINT_2F scale = { .x = hpBar.GetScale().width, .y = hpBar.GetScale().height };
			scale = Math::LerpVector(scale, { MONSTER_HP_BAR_WIDTH * (float(monster.hpValue) / float(MONSTER_MAX_HP)), hpBar.GetScale().height }, 10.0f * deltaTime);
			hpBar.SetScale({ scale.x, scale.y });
		}

		// 몬스터와 총알이 충돌하면, 총알 이펙트가 생성된다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Monster& monster = mMonsters[i];

			if (not monster.isBulletColliding)
			{
				continue;
			}

			monster.bulletEffectTimer += deltaTime;

			D2D1_POINT_2F startScale = { monster.bulletEffectScale.width, monster.bulletEffectScale.height };
			
			float tt = (monster.bulletEffectTimer - START_LERP_TIME) / DURING_TIME;
			tt = std::clamp(tt, 0.0f, 1.0f);
			startScale = Math::LerpVector(startScale, { 0.1f , 0.1f }, 5.0f * deltaTime);

			if (tt >= 1.0f)
			{
				monster.bulletEffectTimer = 0.0f;
			}

			monster.bulletEffectScale = { startScale.x , startScale.y };

			// 크기를 보간한다.

			// 두께를 보간한다.
			//monster.BulletThick = { .x = 20.0f, .y = 20.0f };
			//float t = (monster.BulletEffectTimer - START_LERP_TIME) / DURING_TIME;
			//t = std::clamp(t, 0.0f, 1.0f);
			//monster.BulletThick = Math::LerpVector(monster.BulletThick, { 0.1f , 0.1f }, t);

			//if (t >= 1.0f)
			//{
			//	monster.IsBulletColliding = false;
			//	monster.BulletEffectTimer = 0.0f;
			//}

			break;
		}

		// 몬스터가 죽으면, 사라지는 이펙트가 생성된다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Monster& monster = mMonsters[i];
			Sprite& monsterSprite = monster.sprite;

			if (monster.state != eMonster_State::Dead)
			{
				continue;
			}

			monster.dieTimer += deltaTime;

			D2D1_POINT_2F startScale = { monsterSprite.GetScale().width, monsterSprite.GetScale().height };
			startScale = Math::LerpVector(startScale, { 3.0f , 3.0f }, 8.0f * deltaTime);

			float t = (monster.dieTimer - START_LERP_TIME) / DURING_TIME;
			t = std::clamp(t, 0.0f, 1.0f);

			startScale = Math::LerpVector(startScale, { 0.1f , 0.1f }, t);

			if (t >= 1.0f)
			{
				monsterSprite.SetActive(false);

				monster.backgroundHpBar.SetActive(false);
				monster.hpBar.SetActive(false);

				monster.dieTimer = 0.0f;
			}

			monsterSprite.SetScale({ startScale.x , startScale.y });
		}
	}

	// 돌진 몬스터를 업데이트한다.
	//{
	//	// 몬스터를 일정 시간마다 스폰한다.
	//	mRunMonsterSpawnTimer += deltaTime;
	//	if (mRunMonsterSpawnTimer >= 0.5f)
	//	{
	//		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
	//		{
	//			Monster& runMonster = mRunMonsters[i];
	//			Sprite& runMonsterSprite = runMonster.Sprite;

	//			if (runMonsterSprite.IsActive())
	//			{
	//				continue;
	//			}

	//			if (runMonster.IsDead)
	//			{
	//				continue;
	//			}

	//			const float angle = getRandom(MIN_ANGLE, MAX_ANGLE);
	//			const D2D1_POINT_2F spawnDirection =
	//			{
	//				.x = cos(angle),
	//				.y = sin(angle)
	//			};

	//			const float offset = BOUNDARY_RADIUS - 30.0f;
	//			const D2D1_POINT_2F spawnPositionCircle = Math::ScaleVector(spawnDirection, offset);


	//			runMonster.PrevPosition = spawnPositionCircle;
	//			runMonsterSprite.SetPosition(spawnPositionCircle);
	//			runMonsterSprite.SetScale({ .width = RUN_MONSTER_SCALE, .height = RUN_MONSTER_SCALE });
	//			runMonsterSprite.SetActive(true);

	//			// 스폰할 때 필요한 데이터를 업데이트한다.
	//			runMonster.IsSpawn = true;

	//			mRunMonsterisMoveables[i] = false;
	//			mRunMonsterStartBars[i].SetPosition({ .x = spawnPositionCircle.x - 10.0f, .y = spawnPositionCircle.y - 20.0f });
	//			mRunMonsterStartBars[i].SetScale({ .width = 0.0f, .height = 0.1f });
	//			mRunMonsterStartBars[i].SetActive(true);

	//			runMonster.BulletEffectScale = { .width = 10.0f, .height = 10.0f };
	//			runMonster.HpValue = RUN_MONSTER_MAX_HP;

	//			mRunMonsterSpawnTimer = 0.0f;
	//			break;
	//		}
	//	}

	//	// 돌진 몬스터가 스폰되면, 커졌다가 작아진다.
	//	for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
	//	{
	//		Monster& runMonster = mRunMonsters[i];
	//		Sprite& runMonsterSprite = runMonster.Sprite;

	//		if (not runMonsterSprite.IsActive())
	//		{
	//			continue;
	//		}

	//		if (runMonster.IsDead)
	//		{
	//			continue;
	//		}

	//		if (not runMonster.IsSpawn)
	//		{
	//			continue;
	//		}

	//		runMonster.GrowingTimer += deltaTime;

	//		D2D1_POINT_2F startScale = { runMonsterSprite.GetScale().width, runMonsterSprite.GetScale().height };
	//		startScale = Math::LerpVector(startScale, { 3.0f , 3.0f }, 8.0f * deltaTime);

	//		float t = (runMonster.GrowingTimer - START_LERP_TIME) / DURING_TIME;
	//		t = std::clamp(t, 0.0f, 1.0f);

	//		startScale = Math::LerpVector(startScale, { RUN_MONSTER_SCALE , RUN_MONSTER_SCALE }, t);

	//		if (t >= 1.0f)
	//		{
	//			runMonster.GrowingTimer = 0.0f;
	//			runMonster.IsSpawn = false;
	//		}

	//		runMonsterSprite.SetScale({ startScale.x , startScale.y });
	//		break;
	//	}

	//	// 돌진 몬스터가 이동한다.
	//	for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
	//	{
	//		Monster& runMonster = mRunMonsters[i];
	//		Sprite& runMonsterSprite = runMonster.Sprite;

	//		if (not runMonsterSprite.IsActive())
	//		{
	//			mRunMonsterStartBars[i].SetActive(false);
	//			continue;
	//		}

	//		if (runMonster.IsDead)
	//		{
	//			continue;
	//		}

	//		if (not mRunMonsterisMoveables[i])
	//		{
	//			// 출발바가 꽉 차면 돌진 몬스터는 이동한다.
	//			constexpr float START_COOL_TIME = 2.0f;
	//			float barSpeed = RUN_MONSTER_START_BAR_WIDTH / START_COOL_TIME;

	//			D2D1_SIZE_F scale = mRunMonsterStartBars[i].GetScale();
	//			if (scale.width < RUN_MONSTER_START_BAR_WIDTH)
	//			{
	//				scale.width += barSpeed * deltaTime;
	//				mRunMonsterStartBars[i].SetScale(scale);

	//				continue;
	//			}
	//			else
	//			{
	//				scale.width = RUN_MONSTER_START_BAR_WIDTH;
	//				mRunMonsterStartBars[i].SetScale(scale);

	//				const D2D1_POINT_2F monsterPosition = runMonsterSprite.GetPosition();
	//				const D2D1_POINT_2F heroPosition = mHero.GetPosition();

	//				mRunMonsterMoveSpeeds[i] = 0.0f;

	//				mRunMonsterMoveDirections[i] = Math::SubtractVector(heroPosition, monsterPosition);
	//				mRunMonsterMoveDirections[i] = Math::NormalizeVector(mRunMonsterMoveDirections[i]);

	//				mRunMonsterStartBars[i].SetActive(false);
	//				mRunMonsterisMoveables[i] = true;

	//				// hp바를 생성한다.
	//				runMonster.BackgroundHpBar.SetActive(true);
	//				runMonster.HpBar.SetActive(true);
	//			}
	//		}

	//		constexpr float MOVE_ACC = 5.0f;
	//		mRunMonsterMoveSpeeds[i] = min(mRunMonsterMoveSpeeds[i] + MOVE_ACC, 400.0f);
	//		D2D1_POINT_2F velocity = Math::ScaleVector(mRunMonsterMoveDirections[i], mRunMonsterMoveSpeeds[i] * deltaTime);

	//		D2D1_POINT_2F position = runMonsterSprite.GetPosition();
	//		position = Math::AddVector(position, velocity);
	//		runMonster.PrevPosition = position;
	//		runMonsterSprite.SetPosition(position);

	//		// hp를 좌표를 업데이트한다.
	//		const D2D1_SIZE_F scaleOffset =
	//		{
	//			.width = runMonsterSprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f,
	//			.height = runMonsterSprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f
	//		};

	//		const D2D1_POINT_2F offset =
	//		{
	//			.x = runMonsterSprite.GetPosition().x - scaleOffset.width,
	//			.y = runMonsterSprite.GetPosition().y - scaleOffset.height - 10.0f
	//		};

	//		runMonster.BackgroundHpBar.SetPosition(offset);
	//		runMonster.HpBar.SetPosition(offset);
	//	}

	//	// 돌진 몬스터의 체력을 업데이트한다.
	//	static float prevHp[RUN_MONSTER_COUNT];

	//	for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
	//	{
	//		Monster& runMonster = mRunMonsters[i];
	//		Sprite& hpBar = runMonster.HpBar;

	//		if (runMonster.IsDead)
	//		{
	//			continue;
	//		}

	//		if (not runMonster.IsDead
	//			and runMonster.HpValue <= 0)
	//		{
	//			mRunMonsterDeadSound.Replay();

	//			runMonster.HpValue = 0;
	//			runMonster.IsDead = true;
	//		}

	//		if (prevHp[i] > runMonster.HpValue)
	//		{
	//			// 카메라 흔들기를 시작한다.
	//			const float amplitude = Constant::Get().GetHeight() * getRandom(0.012f, 0.02f);
	//			const float duration = getRandom(0.3f, 0.9f);
	//			const float frequency = getRandom(70.0f, 90.0f);
	//			initializeCameraShake(amplitude, duration, frequency);
	//		}

	//		prevHp[i] = runMonster.HpValue;

	//		D2D1_POINT_2F scale = { .x = hpBar.GetScale().width, .y = hpBar.GetScale().height };
	//		scale = Math::LerpVector(scale, 
	//			{ 
	//				.x = RUN_MONSTER_HP_BAR_WIDTH * (float(runMonster.HpValue) / float(RUN_MONSTER_MAX_HP)), 
	//				.y = hpBar.GetScale().height 
	//			}, 10.0f * deltaTime);
	//		hpBar.SetScale({ scale.x, scale.y });
	//	}

	//	// 돌진 몬스터와 총알이 충돌하면, 총알 이펙트가 생성된다.
	//	for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
	//	{
	//		Monster& runMonster = mRunMonsters[i];
	//		Sprite& runMonsterSprite = runMonster.Sprite;

	//		if (not runMonster.IsBulletColliding)
	//		{
	//			continue;
	//		}

	//		runMonster.BulletEffectTimer += deltaTime;

	//		// 크기를 보간한다.
	//		D2D1_POINT_2F scale = { runMonster.BulletEffectScale.width, runMonster.BulletEffectScale.height };
	//		scale = Math::LerpVector(scale, { 60.0f , 60.0f }, 5.0f * deltaTime);
	//		runMonster.BulletEffectScale = { scale.x, scale.y };

	//		// 두께를 보간한다.
	//		runMonster.BulletThick = { .x = 15.0f, .y = 15.0f };
	//		float t = (runMonster.BulletEffectTimer - START_LERP_TIME) / DURING_TIME;
	//		t = std::clamp(t, 0.0f, 1.0f);
	//		runMonster.BulletThick = Math::LerpVector(runMonster.BulletThick, { 0.1f , 0.1f }, t);

	//		if (t >= 1.0f)
	//		{
	//			runMonster.IsBulletColliding = false;
	//			runMonster.BulletEffectTimer = 0.0f;
	//		}

	//		break;
	//	}

	//	// 몬스터가 죽으면, 사라지는 이펙트가 생성된다.
	//	for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
	//	{
	//		Monster& runMonster = mRunMonsters[i];
	//		Sprite& runMonsterSprite = runMonster.Sprite;

	//		if (not runMonster.IsDead)
	//		{
	//			continue;
	//		}

	//		runMonster.DieTimer += deltaTime;

	//		D2D1_POINT_2F startScale = { runMonsterSprite.GetScale().width, runMonsterSprite.GetScale().height };
	//		startScale = Math::LerpVector(startScale, { 1.5f, 1.5f }, 8.0f * deltaTime);

	//		float t = (runMonster.DieTimer - START_LERP_TIME) / DURING_TIME;
	//		t = std::clamp(t, 0.0f, 1.0f);

	//		startScale = Math::LerpVector(startScale, { 0.1f, 0.1f }, t);

	//		if (t >= 1.0f)
	//		{
	//			runMonsterSprite.SetActive(false);

	//			runMonster.BackgroundHpBar.SetActive(false);
	//			runMonster.HpBar.SetActive(false);

	//			runMonster.IsDead = false;
	//			runMonster.DieTimer = 0.0f;
	//		}

	//		runMonsterSprite.SetScale({ startScale.x , startScale.y });
	//	}
	//}

	//// 느린 몬스터를 업데이트한다.
	//{
	//	// 몬스터를 일정 시간마다 스폰한다.
	//	mSlowMonsterSpawnTimer += deltaTime;
	//	if (mSlowMonsterSpawnTimer >= 0.5f)
	//	{
	//		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
	//		{
	//			Monster& slowMonster = mSlowMonsters[i];
	//			Sprite& slowMonsterSprite = slowMonster.Sprite;

	//			if (slowMonsterSprite.IsActive())
	//			{
	//				continue;
	//			}

	//			if (slowMonster.IsDead)
	//			{
	//				continue;
	//			}

	//			const float angle = getRandom(MIN_ANGLE, MAX_ANGLE);
	//			const D2D1_POINT_2F spawnDirection =
	//			{
	//				.x = cos(angle),
	//				.y = sin(angle)
	//			};

	//			const float offset = BOUNDARY_RADIUS - 30.0f;
	//			const D2D1_POINT_2F spawnPositionCircle = Math::ScaleVector(spawnDirection, offset);

	//			slowMonsterSprite.SetPosition(spawnPositionCircle);
	//			slowMonsterSprite.SetScale({ .width = SLOW_MONSTER_SCALE, .height = SLOW_MONSTER_SCALE });
	//			slowMonsterSprite.SetActive(true);

	//			// 스폰할 때 필요한 데이터를 업데이트한다.
	//			slowMonster.IsSpawn = true;
	//			mSlowMonsterState[i] = eSlow_Monster_State::Stop;

	//			slowMonster.BulletEffectScale = { .width = 10.0f, .height = 10.0f };
	//			slowMonster.HpValue = SLOW_MONSTER_MAX_HP;

	//			mSlowMonsterSpawnTimer = 0.0f;
	//			break;
	//		}
	//	}

	//	// 그림자를 업데이트한다.
	//	for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
	//	{
	//		for (Sprite& shadow : mSlowMonsterShadows[i])
	//		{
	//			if (not shadow.IsActive())
	//			{
	//				continue;
	//			}

	//			float opacity = shadow.GetOpacity();
	//			opacity -= 5.0f * deltaTime;
	//			shadow.SetOpacity(opacity);

	//			if (opacity <= 0.0f)
	//			{
	//				shadow.SetActive(false);
	//			}
	//		}
	//	}

	//	// 느린 몬스터가 스폰되면, 커졌다가 작아진다.
	//	for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
	//	{
	//		Monster& slowMonster = mSlowMonsters[i];
	//		Sprite& slowMonsterSprite = slowMonster.Sprite;

	//		if (not slowMonsterSprite.IsActive())
	//		{
	//			continue;
	//		}

	//		if (slowMonster.IsDead)
	//		{
	//			continue;
	//		}

	//		if (not slowMonster.IsSpawn)
	//		{
	//			continue;
	//		}

	//		slowMonster.GrowingTimer += deltaTime;

	//		D2D1_POINT_2F startScale = { slowMonsterSprite.GetScale().width, slowMonsterSprite.GetScale().height };
	//		startScale = Math::LerpVector(startScale, { 3.0f , 3.0f }, 8.0f * deltaTime);

	//		float t = (slowMonster.GrowingTimer - START_LERP_TIME) / DURING_TIME;
	//		t = std::clamp(t, 0.0f, 1.0f);

	//		startScale = Math::LerpVector(startScale, { SLOW_MONSTER_SCALE , SLOW_MONSTER_SCALE }, t);

	//		if (t >= 1.0f)
	//		{
	//			slowMonster.IsSpawn = false;
	//			slowMonster.GrowingTimer = 0.0f;
	//		}

	//		slowMonsterSprite.SetScale({ startScale.x , startScale.y });

	//		slowMonster.BackgroundHpBar.SetActive(true);
	//		slowMonster.HpBar.SetActive(true);
	//		break;
	//	}

	//	// 느린 몬스터와 그림자가 이동한다.
	//	constexpr float LENGTH = 100.0f;
	//	constexpr float MOVE_TIME = 1.5f;
	//	constexpr float STOP_TIME = 1.0f;

	//	for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
	//	{
	//		Monster& slowMonster = mSlowMonsters[i];
	//		Sprite& slowMonsterSprite = slowMonster.Sprite;

	//		if (not slowMonsterSprite.IsActive())
	//		{
	//			for (uint32_t j = 0; j < SHADOW_COUNT; ++j)
	//			{
	//				mSlowMonsterShadows[i][j].SetActive(false);
	//			}

	//			continue;
	//		}

	//		if (slowMonster.IsDead)
	//		{
	//			continue;
	//		}

	//		switch (mSlowMonsterState[i])
	//		{
	//		case eSlow_Monster_State::Moving:
	//		{
	//			mSlowMonsterMovingTimers[i] += deltaTime;

	//			float t = mSlowMonsterMovingTimers[i] / MOVE_TIME;
	//			t = std::clamp(t, 0.0f, 1.0f);

	//			// 갈수록 느려지는 효과이다.
	//			float easeOutT = 1.0f - (1.0f - t) * (1.0f - t);

	//			D2D1_POINT_2F position = Math::LerpVector(mSlowMonsterStartPositions[i], mSlowMonsterEndPositions[i], easeOutT);
	//			slowMonsterSprite.SetPosition(position);

	//			if (easeOutT >= 1.0f)
	//			{
	//				mSlowMonsterState[i] = eSlow_Monster_State::Stop;
	//				mSlowMonsterStopTimers[i] = 0.0f;
	//			}

	//			break;
	//		}

	//		case eSlow_Monster_State::Stop:
	//		{
	//			mSlowMonsterStopTimers[i] += deltaTime;

	//			if (mSlowMonsterStopTimers[i] >= STOP_TIME)
	//			{
	//				mSlowMonsterShadowCoolTimer = 0.0f;
	//				mSlowMonsterMovingTimers[i] = 0.0f;

	//				mSlowMonsterStartPositions[i] = slowMonsterSprite.GetPosition();

	//				D2D1_POINT_2F direction = Math::SubtractVector({}, mSlowMonsterStartPositions[i]);
	//				direction = Math::NormalizeVector(direction);

	//				mSlowMonsterEndPositions[i] = Math::AddVector(mSlowMonsterStartPositions[i], Math::ScaleVector(direction, LENGTH));

	//				mSlowMonsterState[i] = eSlow_Monster_State::Moving;
	//			}

	//			break;
	//		}
	//		}

	//		mSlowMonsterShadowCoolTimer -= deltaTime;

	//		if (mSlowMonsterShadowCoolTimer <= 0.0f)
	//		{
	//			for (uint32_t j = 0; j < SHADOW_COUNT; ++j)
	//			{
	//				Sprite& shadow = mSlowMonsterShadows[i][j];

	//				if (shadow.IsActive())
	//				{
	//					continue;
	//				}

	//				shadow.SetOpacity(1.0f);
	//				shadow.SetPosition(slowMonsterSprite.GetPosition());
	//				shadow.SetActive(true);

	//				break;
	//			}

	//			mSlowMonsterShadowCoolTimer = 0.05f;
	//		}

	//		// hp를 좌표를 업데이트한다.
	//		const D2D1_SIZE_F scaleOffset =
	//		{
	//			.width = slowMonsterSprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f,
	//			.height = slowMonsterSprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f
	//		};

	//		const D2D1_POINT_2F offset =
	//		{
	//			.x = slowMonsterSprite.GetPosition().x - scaleOffset.width + 2.0f,
	//			.y = slowMonsterSprite.GetPosition().y - scaleOffset.height - 10.0f
	//		};

	//		slowMonster.BackgroundHpBar.SetPosition(offset);
	//		slowMonster.HpBar.SetPosition(offset);
	//	}
	//	
	//	// 느린 몬스터의 체력을 업데이트한다.
	//	static float prevHp[SLOW_MONSTER_COUNT];

	//	for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
	//	{
	//		Monster& slowMonster = mSlowMonsters[i];

	//		if (not slowMonster.IsDead
	//			and slowMonster.HpValue <= 0)
	//		{
	//			// TODO: 스폰 - 죽었을 때 꼬임
	//			//mSlowMonsterDeadSound.Replay();

	//			slowMonster.HpValue = 0;
	//			slowMonster.IsDead = true;
	//		}

	//		if (prevHp[i] > slowMonster.HpValue)
	//		{
	//			// 카메라 흔들기를 시작한다.
	//			const float amplitude = Constant::Get().GetHeight() * getRandom(0.012f, 0.02f);
	//			const float duration = getRandom(0.3f, 0.9f);
	//			const float frequency = getRandom(70.0f, 90.0f);
	//			initializeCameraShake(amplitude, duration, frequency);
	//		}

	//		prevHp[i] = slowMonster.HpValue;

	//		Sprite& hpBar = mSlowMonsters[i].HpBar;

	//		D2D1_POINT_2F scale = { .x = hpBar.GetScale().width, .y = hpBar.GetScale().height };
	//		scale = Math::LerpVector(scale, 
	//			{ 
	//				.x = SLOW_MONSTER_HP_BAR_WIDTH * (float(slowMonster.HpValue) / float(SLOW_MONSTER_MAX_HP)), 
	//				.y = hpBar.GetScale().height 
	//			}, 10.0f * deltaTime);
	//		hpBar.SetScale({ scale.x, scale.y });
	//	}

	//	// 총알 이펙트가 생성된다.
	//	for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
	//	{
	//		Monster& slowMonster = mSlowMonsters[i];

	//		if (not slowMonster.IsBulletColliding)
	//		{
	//			continue;
	//		}

	//		slowMonster.BulletEffectTimer += deltaTime;

	//		// 크기를 보간한다.
	//		D2D1_POINT_2F scale = { slowMonster.BulletEffectScale.width, slowMonster.BulletEffectScale.height };
	//		scale = Math::LerpVector(scale, { 60.0f , 60.0f }, 5.0f * deltaTime);
	//		slowMonster.BulletEffectScale = { scale.x, scale.y };

	//		// 두께를 보간한다.
	//		slowMonster.BulletThick = { .x = 15.0f, .y = 15.0f };
	//		float t = (slowMonster.BulletEffectTimer - START_LERP_TIME) / DURING_TIME;
	//		t = std::clamp(t, 0.0f, 1.0f);

	//		slowMonster.BulletThick = Math::LerpVector(slowMonster.BulletThick, { 0.1f , 0.1f }, t);

	//		if (t >= 1.0f)
	//		{
	//			slowMonster.IsBulletColliding = false;
	//			slowMonster.BulletEffectTimer = 0.0f;
	//		}

	//		break;
	//	}

	//	// 느린 몬스터가 사라지는 이펙트가 생성된다.
	//	for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
	//	{
	//		Monster& slowMonster = mSlowMonsters[i];

	//		if (not slowMonster.IsDead)
	//		{
	//			continue;
	//		}

	//		slowMonster.DieTimer += deltaTime;

	//		Sprite& slowMonsterSprite = slowMonster.Sprite;

	//		D2D1_POINT_2F startScale = { slowMonsterSprite.GetScale().width, slowMonsterSprite.GetScale().height };
	//		startScale = Math::LerpVector(startScale, { 1.5f, 1.5f }, 8.0f * deltaTime);

	//		float t = (slowMonster.DieTimer - START_LERP_TIME) / DURING_TIME;
	//		t = std::clamp(t, 0.0f, 1.0f);

	//		startScale = Math::LerpVector(startScale, { 0.1f, 0.1f }, t);

	//		if (t >= 1.0f)
	//		{
	//			slowMonsterSprite.SetActive(false);

	//			slowMonster.BackgroundHpBar.SetActive(false);
	//			slowMonster.HpBar.SetActive(false);

	//			slowMonster.IsDead = false;
	//			slowMonster.DieTimer = 0.0f;
	//		}

	//		slowMonsterSprite.SetScale({ startScale.x , startScale.y });
	//	}
	//}

	// 플레이어 체력에 관련된 부분을 업데이트한다.
	{
		// 플레이어가 죽었을 때 종료된다.
		if (mHeroHpValue <= 0)
		{
			mBackgroundSound.Pause();
			mEndingSound.Play();

			mEndingLabel.SetActive(true);

			mHeroHpValue = 0;
			mHeroVelocity = {};
			mHero.SetActive(false);
		}

		// 플레이어 체력 라벨을 업데이트 한다.
		static int32_t prevHp = mHeroHpValue;

		if (prevHp != mHeroHpValue)
		{
			mHeroSound.Replay();

			mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHeroHpValue) + L" / " + std::to_wstring(HERO_MAX_HP));

			prevHp = mHeroHpValue;
		}

		// 플레이어 체력바를 업데이트한다.
		D2D1_POINT_2F scale = { mUiHpBar.GetScale().width, mUiHpBar.GetScale().height };
		scale = Math::LerpVector(scale,
			{ UI_HP_SCALE_WIDTH * (float(mHeroHpValue) / float(HERO_MAX_HP)), scale.y },
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
		if (mHeroHpValue > 0)
		{
			mGameTimer += deltaTime;
		}

		uint32_t seconds = uint32_t(mGameTimer) % 60;
		uint32_t minutes = uint32_t(mGameTimer) / 60;

		std::wstring name = L"Timer: " + std::to_wstring(minutes) + L":" + std::to_wstring(seconds);
		mTimerLabel.SetText(name);
	}

	// 충돌 처리를 업데이트한다.
	{
		// 플레이어와 원의 충돌을 한다.
		if (not Collision::IsCollidedCircleWithPoint({}, BOUNDARY_RADIUS, mHero.GetPosition()))
		{
			mHeroVelocity = {};

			D2D1_POINT_2F heroPosition = mHero.GetPosition();
			const D2D1_POINT_2F direction = Math::NormalizeVector(heroPosition);

			heroPosition = Math::AddVector(heroPosition, Math::ScaleVector(direction, -2.0f));
			mHero.SetPosition(heroPosition);
		}

		if (Collision::IsCollidedCircleWithPoint({}, IN_BOUNDARY_RADIUS, mHero.GetPosition()))
		{
			mHeroVelocity = {};

			D2D1_POINT_2F heroPosition = mHero.GetPosition();
			const D2D1_POINT_2F direction = Math::NormalizeVector(heroPosition);

			heroPosition = Math::AddVector(heroPosition, Math::ScaleVector(direction, 2.0f));
			mHero.SetPosition(heroPosition);
		}

		// 총알과 원이 충돌한다.
		for (uint32_t i = 0; i < BULLET_COUNT; ++i)
		{
			Bullet& bullet = mBullets[i];
			Sprite& bulletSprite = bullet.sprite;

			const float halfLength = bulletSprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f;
			const D2D1_POINT_2F endPosition =
			{
				.x = bulletSprite.GetPosition().x + bullet.direction.x * halfLength,
				.y = bulletSprite.GetPosition().y + bullet.direction.y * halfLength
			};

			Line line =
			{
				.Point0 = bullet.prevPosition,
				.Point1 = endPosition
			};

			if (not Collision::IsCollidedCircleWithLine({}, BOUNDARY_RADIUS, line))
			{
				bulletSprite.SetActive(false);
			}

			if (Collision::IsCollidedCircleWithLine({}, IN_BOUNDARY_RADIUS, line))
			{
				bulletSprite.SetActive(false);
			}
		}

		constexpr float DAMAGE_COOL_TIMER = 0.05f;

		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Monster& monster = mMonsters[i];
			Sprite& monsterSprite = monster.sprite;

			if (not monsterSprite.IsActive())
			{
				continue;
			}

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			// 몬스터와 플레이어 충돌을 검사한다.
			monster.playerDistance = 999.9f;

			if (Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero), getRectangleFromSprite(monsterSprite)))
			{
				const float playerDistance = Math::GetVectorLength(Math::SubtractVector(monster.prevPosition, mHero.GetPosition()));
				if (playerDistance < monster.playerDistance)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					monster.hpValue -= PLAYER_ATTACK_VALUE * 2;

					monster.playerDistance = playerDistance;
				}
			}

			
			// 몬스터와 내부 원 충돌을 검사한다.
			monster.inBoundryDistance = 999.9f;

			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monsterSprite), {}, IN_BOUNDARY_RADIUS))
			{
				const float inDistance = Math::GetVectorLength(Math::SubtractVector(monster.prevPosition, {}));
				if (inDistance < monster.inBoundryDistance)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					monster.hpValue -= PLAYER_ATTACK_VALUE * 2;

					monster.inBoundryDistance = inDistance;
				}			
			}
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Monster& runMonster = mRunMonsters[i];
			Sprite& runMonsterSprite = runMonster.sprite;

			if (not runMonsterSprite.IsActive())
			{
				continue;
			}

			if (runMonster.state != eMonster_State::Life)
			{
				continue;
			}

			// 돌진 몬스터와 플레이어 충돌을 검사한다.
			runMonster.playerDistance = 999.9f;

			if (Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero), getRectangleFromSprite(runMonsterSprite)))
			{
				const float playerDistance = Math::GetVectorLength(Math::SubtractVector(runMonster.prevPosition, mHero.GetPosition()));
				if (playerDistance < runMonster.playerDistance)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					runMonster.hpValue -= PLAYER_ATTACK_VALUE;

					runMonster.playerDistance = playerDistance;
				}
			}

			// 돌진 몬스터와 내부 원 충돌을 검사한다.
			runMonster.inBoundryDistance = 999.9f;

			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(runMonsterSprite), {}, IN_BOUNDARY_RADIUS))
			{
				const float inDistance = Math::GetVectorLength(Math::SubtractVector(runMonster.prevPosition, {}));
				if (inDistance < runMonster.inBoundryDistance)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					runMonster.hpValue -= BOUNDRY_ATTACK_VALUE;

					runMonster.inBoundryDistance = inDistance;
				}
			}

			// 돌진 몬스터와 외부 원 충돌을 검사한다.			
			runMonster.boundryDistance = 999.9f;

			if (not Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(runMonsterSprite), {}, BOUNDARY_RADIUS))
			{
				const float outDistance = Math::GetVectorLength(Math::SubtractVector(runMonster.prevPosition, {}));
				if (outDistance < runMonster.boundryDistance)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					runMonster.hpValue -= BOUNDRY_ATTACK_VALUE;

					runMonster.boundryDistance = outDistance;
				}
			}
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Monster& slowMonster = mSlowMonsters[i];
			Sprite& slowMonsterSprite = mSlowMonsters[i].sprite;

			if (not slowMonsterSprite.IsActive())
			{
				continue;
			}

			if (slowMonster.state != eMonster_State::Life)
			{
				continue;
			}

			// 느린 몬스터와 플레이어 충돌을 검사한다.
			slowMonster.playerDistance = 999.9f;

			if (Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero), getRectangleFromSprite(slowMonsterSprite)))
			{
				const float playerDistance = Math::GetVectorLength(Math::SubtractVector(slowMonster.prevPosition, mHero.GetPosition()));
				if (playerDistance < slowMonster.playerDistance)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					slowMonster.hpValue -= PLAYER_ATTACK_VALUE;

					slowMonster.playerDistance = playerDistance;
				}
			}

			// 느린 몬스터와 내부 원 충돌을 검사한다.
			slowMonster.inBoundryDistance = 999.9f;

			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(slowMonsterSprite), {}, IN_BOUNDARY_RADIUS))
			{
				const float inDistance = Math::GetVectorLength(Math::SubtractVector(slowMonster.prevPosition, {}));
				if (inDistance < slowMonster.inBoundryDistance)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					slowMonster.hpValue -= BOUNDRY_ATTACK_VALUE;

					slowMonster.inBoundryDistance = inDistance;
				}
			}
		}

		// 총알과 모든 몬스터 충돌을 검사한다.
		for (uint32_t i = 0; i < BULLET_COUNT; ++i)
		{
			Bullet& bullet = mBullets[i];
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

			for (uint32_t j = 0; j < MONSTER_COUNT; ++j)
			{
				Monster& monster = mMonsters[j];
				Sprite& monsterSprite = monster.sprite;

				if (not monsterSprite.IsActive())
				{
					continue;
				}

				if (monster.state != eMonster_State::Life)
				{
					continue;
				}

				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(monsterSprite), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(bullet.prevPosition, monsterSprite.GetPosition()));
				if (distance < targetMonsterDistance)
				{
					monster.hpValue -= BULLET_ATTACK_VALUE;
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
					runMonster.hpValue -= BULLET_ATTACK_VALUE;
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
					slowMonster.hpValue -= BULLET_ATTACK_VALUE;
					slowMonster.isBulletColliding = true;

					targetSlowMonster = &slowMonsterSprite;
					targetSlowMonsterDistance = distance;
				}
			}
			
			if (targetMonster != nullptr
				or targetRunMonster != nullptr
				or targetSlowMonster != nullptr)
			{
				mBullets[i].sprite.SetActive(false);
			}
		}

		// 플레이어 쉴드와 몬스터가 충돌하면 몬스터는 삭제된다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			if (mShieldState != eShield_State::Growing
				and mShieldState != eShield_State::Waiting)
			{
				continue;
			}

			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mMonsters[i].sprite), mHero.GetPosition(), mShieldScale.width * 0.5f))
			{
				mMonsters[i].hpValue = 0;
			}
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			if (mShieldState != eShield_State::Growing
				and mShieldState != eShield_State::Waiting)
			{
				continue;
			}

			Monster& runMonster = mRunMonsters[i];
			Sprite& runMonsterSprite = runMonster.sprite;

			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(runMonsterSprite), mHero.GetPosition(), mShieldScale.width * 0.5f))
			{
				runMonster.hpValue = 0;
			}
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			if (mShieldState != eShield_State::Growing
				and mShieldState != eShield_State::Waiting)
			{
				continue;
			}

			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mSlowMonsters[i].sprite), mHero.GetPosition(), mShieldScale.width * 0.5f))
			{
				mSlowMonsters[i].hpValue = 0;
			}
		}

		// 플레이어 주변을 공전하는 원과 몬스터가 충돌하면 몬스터는 삭제된다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			if (mOrbitState != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::AddVector(mOrbitEllipse.point, mHero.GetPosition());
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mMonsters[i].sprite), center, mOrbitEllipse.radiusX))
			{
				mMonsters[i].hpValue = 0;
			}
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			if (mOrbitState != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::AddVector(mOrbitEllipse.point, mHero.GetPosition());
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mRunMonsters[i].sprite), center, mOrbitEllipse.radiusX))
			{
				mRunMonsters[i].hpValue = 0;
			}
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			if (mOrbitState != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::AddVector(mOrbitEllipse.point, mHero.GetPosition());
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mSlowMonsters[i].sprite), center, mOrbitEllipse.radiusX))
			{
				mSlowMonsters[i].hpValue = 0;
			}
		}
	}

	// 카메라를 업데이트한다.
	{
		D2D1_POINT_2F position = mMainCamera.GetPosition();
		const D2D1_POINT_2F heroPosition = mHero.GetPosition();
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
		const Matrix3x2F worldView = Transformation::getWorldMatrix(mHero.GetPosition()) * view;
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
		const Matrix3x2F worldView = Transformation::getWorldMatrix(mHero.GetPosition()) * view;
		renderTarget->SetTransform(worldView);

		if (mOrbitState == eOrbit_State::Rotating
			and mOrbitBlinkOn)
		{
			renderTarget->DrawEllipse(mOrbitEllipse, mYellowBrush, 5.0f);
		}
	}

	// 총알과 몬스터가 충돌하면, 이펙트를 그린다.
	{
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Monster& monster = mMonsters[i];

			if (not monster.isBulletColliding)
			{
				continue;
			}

			Sprite& monsterSprite = monster.sprite;

			D2D1_POINT_2F direction = Math::NormalizeVector(monsterSprite.GetPosition());
			direction = Math::RotateVector(direction, getRandom(-50.0f, 50.0f));

			const Matrix3x2F worldView = 
				Transformation::getWorldMatrix(
					{
						.x = monsterSprite.GetPosition().x - 10.0f,
						.y = monsterSprite.GetPosition().y + 720.0f
					}
			, direction.y, {1.0f, 3.0f})* view;
			renderTarget->SetTransform(worldView);

			const D2D1_RECT_F collider =
			{
				.left = 0.0f,
				.top = 0.0f,
				.right = monster.bulletEffectScale.width,
				.bottom = monster.bulletEffectScale.height
			};

			renderTarget->FillRectangle(collider, mCyanBrush);
		}
	}

	// 총알과 돌진 몬스터가 충돌하면, 이펙트를 그린다.
	{
		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Monster& runMonster = mRunMonsters[i];

			if (not runMonster.isBulletColliding)
			{
				continue;
			}

			Sprite& runMonsterSprite = runMonster.sprite;

			const Matrix3x2F worldView = 
				Transformation::getWorldMatrix(
					{ 
						.x = runMonsterSprite.GetPosition().x, 
						.y = runMonsterSprite.GetPosition().y + 35.0f 
					}, 45.0f) * view;
			renderTarget->SetTransform(worldView);

			const D2D1_RECT_F colliderSize =
			{
				.left = 0.0f,
				.top = 0.0f,
				.right = runMonster.bulletEffectScale.width,
				.bottom = runMonster.bulletEffectScale.height
			};

			renderTarget->DrawRectangle(colliderSize, mCyanBrush, runMonster.bulletThick.x);
		}
	}

	// 총알과 느린 몬스터가 충돌하면, 이펙트를 그린다.
	{
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Monster& slowMonster = mSlowMonsters[i];

			if (not slowMonster.isBulletColliding)
			{
				continue;
			}

			Sprite& slowMonsterSprite = mSlowMonsters[i].sprite;

			const Matrix3x2F worldView = 
				Transformation::getWorldMatrix(
					{ 
						.x = slowMonsterSprite.GetPosition().x, 
						.y = slowMonsterSprite.GetPosition().y + 35.0f 
					}, 45.0f) * view;
			renderTarget->SetTransform(worldView);

			const D2D1_RECT_F colliderSize =
			{
				.left = 0.0f,
				.top = 0.0f,
				.right = slowMonster.bulletEffectScale.width,
				.bottom = slowMonster.bulletEffectScale.height
			};

			renderTarget->DrawRectangle(colliderSize, mCyanBrush, slowMonster.bulletThick.x);
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

void MainScene::UpdateSpawnMonster(Monster* monster, const float radius, const D2D1_SIZE_F monsterScale, const D2D1_SIZE_F bulletEffectScale,
	const D2D1_POINT_2F hpOffset, const float maxHp, const float deltaTime)
{
	ASSERT(monster != nullptr);

	Sprite& monsterSprite = monster->sprite;

	// 몬스터의 기본 정보를 업데이트한다.
	{
		constexpr float MIN_ANGLE = 0.0f;
		constexpr float MAX_ANGLE = 2.0f * Math::PI;

		const float angle = getRandom(MIN_ANGLE, MAX_ANGLE);
		const D2D1_POINT_2F spawnDirection =
		{
			.x = cos(angle),
			.y = sin(angle)
		};

		const float offset = radius - 30.0f;
		const D2D1_POINT_2F spawnPositionCircle = Math::ScaleVector(spawnDirection, offset);
		monster->prevPosition = spawnPositionCircle;
		monsterSprite.SetPosition(spawnPositionCircle);
		monsterSprite.SetScale(monsterScale);
		monsterSprite.SetActive(true);

		// 초기 정보를 업데이트한다.
		monster->state = eMonster_State::Spawn;
		monster->bulletEffectScale = bulletEffectScale;
		monster->hpValue = maxHp;
	}

	// hp를 좌표를 업데이트한다.
	{
		const D2D1_SIZE_F scaleOffset =
		{
			.width = monsterSprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f,
			.height = monsterSprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f
		};

		const D2D1_POINT_2F monsterPosition = monsterSprite.GetPosition();

		const D2D1_POINT_2F offset =
		{
			.x = monsterPosition.x - scaleOffset.width + hpOffset.x,
			.y = monsterPosition.y - scaleOffset.height - hpOffset.y
		};

		monster->backgroundHpBar.SetPosition(offset);
		monster->hpBar.SetPosition(offset);
	}
}

void MainScene::UpdateSpawnEffectMonster(Monster* monster, const D2D1_SIZE_F effectScale, const D2D1_SIZE_F monsterScale, float effectTime, const float deltaTime)
{
	ASSERT(monster != nullptr);

	monster->growingTimer += deltaTime;
	
	float t = monster->growingTimer / effectTime;
	t = std::clamp(t, 0.0f, 1.0f);

	D2D1_POINT_2F scale = Math::LerpVector({ effectScale.width, effectScale.height }, { monsterScale.width, monsterScale.height }, t);

	if (t >= 1.0f)
	{
		monster->state = eMonster_State::Life;
		monster->growingTimer = 0.0f;
	}

	monster->sprite.SetScale({ scale.x, scale.y });

	// 이펙트가 끝나면 체력바를 업데이트한다.
	monster->backgroundHpBar.SetActive(true);
	monster->hpBar.SetActive(true);
}
