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

		mCircleTexture.Initialize(GetHelper(), L"Resource/Circle.png");
		mRedCircleTexture.Initialize(GetHelper(), L"Resource/RedCircle.png");

		mWhiteBarTexture.Initialize(GetHelper(), L"Resource/WhiteBar.png");
		mRedBarTexture.Initialize(GetHelper(), L"Resource/RedBar.png");
		mYellowBarTexture.Initialize(GetHelper(), L"Resource/YellowBar.png");
	}

	// 플레이어를 초기화한다.
	{
		mHero.SetPosition({ .x = -200.0f, .y = 0.0f });
		mHero.SetScale({ .width = 0.75f, .height = 0.75f });
		mHero.SetTexture(&mRectangleTexture);
		mSpriteLayers[uint32_t(Layer::Player)].push_back(&mHero);

		for (Sprite& shadow : mDashShadow)
		{
			shadow.SetScale(mHero.GetScale());
			shadow.SetTexture(&mRectangleTexture);
			shadow.SetActive(false);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&shadow);
		}
	}

	// 총알을 초기화한다.
	{
		for (Sprite& bullet : mBullets)
		{
			bullet.SetPosition(mHero.GetPosition());
			bullet.SetCenter({ .x = -0.5f, .y = 0.0f });
			bullet.SetScale({ 2.5f, 0.1f });
			bullet.SetActive(false);
			bullet.SetTexture(&mRedRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&bullet);
		}
	}

	// 탄피를 초기화한다.
	{
		for (Sprite& casing : mCasings)
		{
			casing.SetScale({ .width = 0.2f, .height = 0.2f });
			casing.SetCenter({ -0.5, 0.0f });
			casing.SetOpacity(0.3f);
			casing.SetActive(false);
			casing.SetTexture(&mRedRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&casing);
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
		for (Sprite& monster : mMonsters)
		{
			monster.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });
			monster.SetActive(false);
			monster.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&monster);
		}
	}

	// 돌진 몬스터를 초기화한다.
	{
		for (Sprite& monster : mRunMonsters)
		{
			monster.SetScale({ .width = RUN_MONSTER_SCALE, .height = RUN_MONSTER_SCALE });
			monster.SetActive(false);
			monster.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&monster);
		}

		constexpr D2D1_POINT_2F BAR_SCALE = { .x = RUN_MONSTER_WIDTH, .y = 0.1f };

		// 시작바를 생성한다.
		for (Sprite& bar : mRunMonsterBars)
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
		for (Sprite& monster : mSlowMonsters)
		{
			monster.SetScale({ .width = SLOW_MONSTER_SCALE, .height = SLOW_MONSTER_SCALE });
			monster.SetActive(false);
			monster.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&monster);
		}

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
		const D2D1_POINT_2F heroPosition = mHero.GetPosition();
		const D2D1_POINT_2F cameraPosition{ .x = heroPosition.x * 0.5f, .y = heroPosition.y * 0.5f };

		mMainCamera.SetPosition(cameraPosition);
	}

	// Dash 게이지를 초기화한다.
	{
		mDashUiBar.SetPosition({ .x = -380.0f, .y = -UI_CENTER_POSITION_Y });
		mDashUiBar.SetScale({ .width = UI_DASH_SCALE_WIDTH, .height = 1.0f });
		mDashUiBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mDashUiBar.SetUI(true);
		mDashUiBar.SetTexture(&mWhiteBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mDashUiBar);

		mDashValue.SetPosition(mDashUiBar.GetPosition());
		mDashValue.SetScale(mDashUiBar.GetScale());
		mDashValue.SetCenter({ .x = -0.5f, .y = 0.0f });
		mDashValue.SetUI(true);
		mDashValue.SetTexture(&mYellowBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mDashValue);
	}

	// HP바를 초기화한다.
	{
		mHpUiBar.SetPosition({ .x = mDashUiBar.GetPosition().x, .y = mDashUiBar.GetPosition().y + 50.f });
		mHpUiBar.SetScale({ .width = UI_HP_SCALE_WIDTH, .height = 1.0f });
		mHpUiBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mHpUiBar.SetUI(true);
		mHpUiBar.SetTexture(&mWhiteBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mHpUiBar);

		mHpBar.SetPosition(mHpUiBar.GetPosition());
		mHpBar.SetScale(mHpUiBar.GetScale());
		mHpBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mHpBar.SetUI(true);
		mHpBar.SetTexture(&mRedBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mHpBar);
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

			const D2D1_POINT_2F dashBarPosition = mDashUiBar.GetPosition();
			constexpr float OFFSET_X = 5.0f;
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

			const D2D1_POINT_2F hpBarPosition = mHpBar.GetPosition();
			constexpr float OFFSET_X = 5.0f;
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

	mLine.Point0 = { .x = -200.0f, .y = 200.0f };
	mLine.Point1 = { .x = 150.0f, .y = 100.0f };

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
		// Case 0) 가속도 없는 단순한 이동
		//{
		//	// 누르면 1, 안 누르면 0
		//	int32_t moveX = Input::Get().GetKey('D') - Input::Get().GetKey('A');
		//	int32_t moveY = Input::Get().GetKey('W') - Input::Get().GetKey('S');

		//	if (moveX != 0 or moveY != 0)
		//	{
		//		float speed = 180.0f * deltaTime;

		//		D2D1_POINT_2F direction = Math::GetNormalizeVector({ .x = float(moveX), .y = float(moveY) });
		//		D2D1_POINT_2F velocity = Math::ScaleVector(direction, speed);

		//		D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), velocity);
		//		mHero.SetPosition(position);
		//	}
		//}

		//  Case 1) X축만 가속도 반영하여 이동 구현
		{
			//int32_t moveX = Input::Get().GetKey('D') - Input::Get().GetKey('A');
			//D2D1_POINT_2F position = mHero.GetPosition();

			//constexpr float MAX_VELOCITY = 250.0f;
			//constexpr float ACC = 4.0f;

			//static int32_t previousMoveX;
			//static float velocity = 0.0f;

			//if (moveX != 0)
			//{
			//	velocity += ACC * moveX;
			//	velocity = std::clamp(velocity, -MAX_VELOCITY, MAX_VELOCITY);

			//	previousMoveX = moveX;
			//}
			//else
			//{
			//	if (previousMoveX > 0)
			//	{
			//		velocity = max(velocity - ACC, 0.0f);
			//	}
			//	else
			//	{
			//		velocity = min(velocity + ACC, 0.0f);
			//	}
			//}

			//DEBUG_LOG("%f", velocity);

			//position.x += velocity * deltaTime;
			//mHero.SetPosition(position);
		}

		// Case 2) 최종 이동 구현
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

			// TODO(이수원): 코드 정리가 필요하다.
			constexpr float MAX_DASH_SPEED = 600.0f;
			constexpr float DASH_ACC = 30.0f;
			static float dashSpeed = 0.0f;
			static bool bDashing = false;
			static float dashShadowCoolTime = 0.0f;
			static D2D1_POINT_2F dashDirection{};

			if (Math::GetVectorLength(mHeroVelocity) != 0.0f)
			{
				float speed = min(Math::GetVectorLength(mHeroVelocity), MAX_SPEED);
				const D2D1_POINT_2F direction = Math::NormalizeVector(mHeroVelocity);
				D2D1_POINT_2F adjustVelocity = Math::ScaleVector(direction, speed);
				adjustVelocity = Math::ScaleVector(adjustVelocity, deltaTime);

				if (not bDashing and Input::Get().GetKeyDown(VK_SPACE))
				{
					dashDirection = direction;

					if (mDashCount != 0)
					{
						mDashCount--;
						bDashing = true;
						dashShadowCoolTime = 0.0f;
					}
				}

				const D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), adjustVelocity);
				mHero.SetPosition(position);
			}

			static float dashScaleTimer;
			static float dashTimer[SHADOW_COUNT];

			if (bDashing)
			{
				dashSpeed = min(dashSpeed + DASH_ACC, MAX_DASH_SPEED);
				const D2D1_POINT_2F velocity = Math::ScaleVector(dashDirection, dashSpeed * deltaTime);

				if (dashSpeed >= MAX_DASH_SPEED)
				{
					dashSpeed = 0.0f;
					bDashing = false;
				}

				const D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), velocity);
				mHero.SetPosition(position);

				dashShadowCoolTime -= deltaTime;
				if (dashShadowCoolTime <= 0.0f)
				{
					for (Sprite& shadow : mDashShadow)
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
					
					dashShadowCoolTime = 0.05f;
				}
			}

			for (Sprite& shadow : mDashShadow)
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
				dashScaleTimer += deltaTime;

				if (dashScaleTimer >= 2.0f)
				{
					mDashCount++;
					dashScaleTimer = 0.0f;
				}
			}
		}

		// 총알을 업데이트한다.
		{
			static float shootingCoolTimer = 0.0f;

			static D2D1_POINT_2F startPosition[CASING_COUNT];
			static D2D1_POINT_2F endPosition[CASING_COUNT];
			constexpr float LENGTH = 100.0f;

			shootingCoolTimer = max(shootingCoolTimer - deltaTime, 0.0f);

			// 총알을 스폰한다.
			if (Input::Get().GetMouseButton(Input::eMouseButton::Left)
				and shootingCoolTimer <= 0.001f
				and mBulletValue != 0 
				and not misKeyDownReload)
			{
				for (uint32_t i = 0; i < BULLET_COUNT; ++i)
				{
					Sprite& bullet = mBullets[i];
					if (bullet.IsActive())
					{
						continue;
					}

					const D2D1_POINT_2F spawnPosition = mHero.GetPosition();
					bullet.SetPosition(spawnPosition);
					mPrevBulletPosition[i] = spawnPosition;

					// 이동 방향을 구한다.
					// TODO(이수원): direction의 길이가 0인 경우 normalize 처리할 때 문제가 생기므로 예외 처리가 필요하다.
					mBulletDirections[i] = Math::SubtractVector(getMouseWorldPosition(), spawnPosition);

					// 거리에 따라 반동효과가 다르다.
					const float length = Math::GetVectorLength(mBulletDirections[i]);
					mBulletDirections[i] = (length >= 200.0f) ?
						Math::RotateVector(mBulletDirections[i], getRandom(-10.0f, 10.0f))
						: Math::RotateVector(mBulletDirections[i], getRandom(-5.0f, 5.0f));

					mBulletDirections[i] = Math::NormalizeVector(mBulletDirections[i]);

					float angle = Math::ConvertRadianToDegree(atan2f(mBulletDirections[i].y, mBulletDirections[i].x));
					bullet.SetAngle(-angle);

					bullet.SetActive(true);

					bullet.SetOpacity(0.0f);

					mBulletValue--;

					// 탄피를 생성한다.
					{
						for (uint32_t j = 0; j < CASING_COUNT; ++j)
						{
							Sprite& casing = mCasings[j];
							
							if (casing.IsActive())
							{
								continue;
							}

							casing.SetOpacity(1.0f);
							casing.SetActive(true);

							D2D1_POINT_2F& casingDirection = mCasingDirections[j];
							casingDirection = Math::NormalizeVector(mBulletDirections[i]);
							casingDirection = Math::RotateVector(casingDirection, getRandom(-30.0f, 30.0f));
							casingDirection = Math::ScaleVector(casingDirection, -1.0f);	// 뒤로 가도록 조정한다.

							D2D1_POINT_2F spawnPosition = mHero.GetPosition();
							constexpr float OFFSET = 50.0f;
							spawnPosition = Math::AddVector(spawnPosition, Math::ScaleVector(casingDirection, OFFSET));
							casing.SetPosition((spawnPosition));

							// 탄피의 이동 좌표를 생성한다.
							startPosition[j] = casing.GetPosition();
							endPosition[j] = Math::AddVector(startPosition[i], Math::ScaleVector(casingDirection, LENGTH));

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

				shootingCoolTimer = 0.12f;
			}

			// 총알을 이동시킨다.
			{
				constexpr float MOVE_SPEED = 1500.0f;

				for (uint32_t i = 0; i < BULLET_COUNT; ++i)
				{
					Sprite& bullet = mBullets[i];
					if (not bullet.IsActive())
					{
						continue;
					}

					const D2D1_POINT_2F velocity = Math::ScaleVector(mBulletDirections[i], MOVE_SPEED * deltaTime);
					const D2D1_POINT_2F position = Math::AddVector(bullet.GetPosition(), velocity);

					mPrevBulletPosition[i] = bullet.GetPosition();
					bullet.SetPosition(position);

					float opacity = bullet.GetOpacity();

					D2D1_POINT_2F lerp = { opacity, opacity };
					lerp = Math::LerpVector(lerp, { 1.0f, 1.0f }, 10.0f * deltaTime);
					bullet.SetOpacity(lerp.x);
				}
			}

			// 탄피를 이동시킨다.
			{
				constexpr float SPEED = 400.0f;
				constexpr float MOVE_TIME = 1.0f;

				for (uint32_t i = 0; i < CASING_COUNT; ++i)
				{
					Sprite& casing = mCasings[i];
					if (not casing.IsActive())
					{
						continue;
					}

					mCasingTimer[i] += deltaTime;

					float t = mCasingTimer[i] / MOVE_TIME;
					t = std::clamp(t, 0.0f, 1.0f);

					D2D1_POINT_2F position = Math::LerpVector(startPosition[i], endPosition[i], t);
					casing.SetPosition(position);

					if (t >= 1.0f)
					{
						casing.SetActive(false);
						mCasingTimer[i] = 0.0f;
					}

					float opacity = casing.GetOpacity();
					opacity -= 0.8f * deltaTime;
					casing.SetOpacity(opacity);
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
		static float growingTimer[MONSTER_COUNT];

		// 몬스터를 일정 시간마다 스폰한다.
		mMonsterSpawnTimer += deltaTime;
		if (mMonsterSpawnTimer >= 0.5f)
		{
			for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
			{
				Sprite& monster = mMonsters[i];
				if (monster.IsActive())
				{
					continue;
				}

				const float angle = getRandom(MIN_ANGLE, MAX_ANGLE);
				const D2D1_POINT_2F spawnDirection =
				{
					.x = cos(angle),
					.y = sin(angle)
				};

				const float offset = BOUNDARY_RADIUS - 30.0f;
				const D2D1_POINT_2F spawnPositionCircle = Math::ScaleVector(spawnDirection, offset);
				monster.SetPosition(spawnPositionCircle);
				monster.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });
				monster.SetActive(true);

				mMonsterSpawnTimer = 0.0f;

				mIsMonsterSpawns[i] = true;

				// 스폰할 때 속도도 같이 초기화한다.
				speed[i] = getRandom(10.0f, 80.0f);

				// 이펙트 크기도 초기화한다.
				mMonsterBulletEffectScales[i] = { .width = 10.0f, .height = 10.0f };
				break;
			}
		}

		// 몬스터가 스폰되면, 커졌다가 작아진다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];

			if (not monster.IsActive())
			{
				continue;
			}

			if (not mIsMonsterSpawns[i])
			{
				continue;
			}

			growingTimer[i] += deltaTime;

			D2D1_POINT_2F startScale = { monster.GetScale().width, monster.GetScale().height };
			startScale = Math::LerpVector(startScale, { 3.0f , 3.0f }, 8.0f * deltaTime);

			float t = (growingTimer[i] - START_LERP_TIME) / DURING_TIME;
			t = std::clamp(t, 0.0f, 1.0f);

			startScale = Math::LerpVector(startScale, { MONSTER_SCALE , MONSTER_SCALE }, t);

			if (t >= 1.0f)
			{
				growingTimer[i] = 0.0f;
				mIsMonsterSpawns[i] = false;
			}

			monster.SetScale({ startScale.x , startScale.y });

			break;
		}

		// 몬스터를 이동시킨다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];
			if (not monster.IsActive())
			{
				continue;
			}

			if (mIsMonsterToBullets[i])
			{
				continue;
			}

			D2D1_POINT_2F position = monster.GetPosition();
			D2D1_POINT_2F direction = Math::SubtractVector({}, position);
			direction = Math::NormalizeVector(direction);
			const D2D1_POINT_2F velocity = Math::ScaleVector(direction, speed[i] * deltaTime);

			position = Math::AddVector(position, velocity);
			monster.SetPosition(position);
		}
	}


	// 돌진 몬스터를 업데이트한다.
	{
		static D2D1_POINT_2F moveDirection[RUN_MONSTER_COUNT];
		static float moveSpeed[RUN_MONSTER_COUNT];
		static bool isMoveables[RUN_MONSTER_COUNT]{};

		// 몬스터를 일정 시간마다 스폰한다.
		mRunMonsterSpawnTimer += deltaTime;

		if (mRunMonsterSpawnTimer >= 0.5f)
		{
			for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
			{
				Sprite& monster = mRunMonsters[i];
				if (monster.IsActive())
				{
					continue;
				}

				const float angle = getRandom(MIN_ANGLE, MAX_ANGLE);
				const D2D1_POINT_2F spawnDirection =
				{
					.x = cos(angle),
					.y = sin(angle)
				};

				const float offset = BOUNDARY_RADIUS - 30.0f;
				const D2D1_POINT_2F spawnPositionCircle = Math::ScaleVector(spawnDirection, offset);

				monster.SetPosition(spawnPositionCircle);
				monster.SetScale({ .width = RUN_MONSTER_SCALE, .height = RUN_MONSTER_SCALE });
				monster.SetActive(true);

				mRunMonsterSpawnTimer = 0.0f;

				isMoveables[i] = false;

				mRunMonsterBulletEffectScales[i] = { .width = 10.0f, .height = 10.0f };

				if (mIsRunMonsterToBullets[i])
				{
					continue;
				}

				mRunMonsterBars[i].SetPosition({ .x = spawnPositionCircle.x - 10.0f, .y = spawnPositionCircle.y - 20.0f });
				mRunMonsterBars[i].SetScale({ .width = 0.0f, .height = 0.1f });
				mRunMonsterBars[i].SetActive(true);

				break;
			}
		}

		// 이동한다.
		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Sprite& monster = mRunMonsters[i];
			if (not monster.IsActive())
			{
				mRunMonsterBars[i].SetActive(false);
				continue;
			}

			if (not isMoveables[i])
			{
				// 출발바가 꽉 차면 돌진 몬스터는 이동한다.
				constexpr float START_COOL_TIME = 2.0f;
				float barSpeed = RUN_MONSTER_WIDTH / START_COOL_TIME;

				D2D1_SIZE_F scale = mRunMonsterBars[i].GetScale();
				if (scale.width < RUN_MONSTER_WIDTH)
				{
					scale.width += barSpeed * deltaTime;
					mRunMonsterBars[i].SetScale(scale);

					continue;
				}
				else
				{
					scale.width = RUN_MONSTER_WIDTH;
					mRunMonsterBars[i].SetScale(scale);

					const D2D1_POINT_2F monsterPosition = monster.GetPosition();
					const D2D1_POINT_2F heroPosition = mHero.GetPosition();

					// TODO: 길이가 0인 경우 예외 처리가 필요하다.
					D2D1_POINT_2F direction = Math::SubtractVector(heroPosition, monsterPosition);
					direction = Math::NormalizeVector(direction);

					moveSpeed[i] = 0.0f;

					// TODO: 길이가 0인 경우 예외 처리가 필요하다.
					moveDirection[i] = Math::SubtractVector(heroPosition, monsterPosition);
					moveDirection[i] = Math::NormalizeVector(moveDirection[i]);

					mRunMonsterBars[i].SetActive(false);
					isMoveables[i] = true;
				}
			}

			if (mIsRunMonsterToBullets[i])
			{
				continue;
			}

			constexpr float MOVE_ACC = 5.0f;
			moveSpeed[i] = min(moveSpeed[i] + MOVE_ACC, 400.0f);
			D2D1_POINT_2F velocity = Math::ScaleVector(moveDirection[i], moveSpeed[i] * deltaTime);

			D2D1_POINT_2F position = monster.GetPosition();
			position = Math::AddVector(position, velocity);
			monster.SetPosition(position);
		}
	}

	// 느린 몬스터를 업데이트한다.
	{
		static D2D1_POINT_2F moveDirection[SLOW_MONSTER_COUNT];
		static float moveSpeed[SLOW_MONSTER_COUNT];

		// 몬스터를 일정 시간마다 스폰한다.
		mSlowMonsterSpawnTimer += deltaTime;

		if (mSlowMonsterSpawnTimer >= 0.5f)
		{
			for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
			{
				Sprite& slowMonster = mSlowMonsters[i];
				if (slowMonster.IsActive())
				{
					continue;
				}

				const float angle = getRandom(MIN_ANGLE, MAX_ANGLE);
				const D2D1_POINT_2F spawnDirection =
				{
					.x = cos(angle),
					.y = sin(angle)
				};

				const float offset = BOUNDARY_RADIUS - 30.0f;
				const D2D1_POINT_2F spawnPositionCircle = Math::ScaleVector(spawnDirection, offset);

				slowMonster.SetPosition(spawnPositionCircle);
				slowMonster.SetScale({ .width = SLOW_MONSTER_SCALE, .height = SLOW_MONSTER_SCALE });
				slowMonster.SetActive(true);

				mSlowMonsterSpawnTimer = 0.0f;

				mSlowMonsterState[i] = eSlow_Monster_State::Stop;

				mSlowMonsterBulletEffectScales[i] = { .width = 10.0f, .height = 10.0f };

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

		// 느린 몬스터와 그림자가 이동한다.
		static float movingTimer[SLOW_MONSTER_COUNT];
		static float stopTimer[SLOW_MONSTER_COUNT];
		static float speed[SLOW_MONSTER_COUNT];
		static float shadowCoolTime[SLOW_MONSTER_COUNT];
		constexpr float LENGTH = 100.0f;
		constexpr float MOVE_TIME = 1.5f;
		constexpr float STOP_TIME = 1.0f;
		static D2D1_POINT_2F startPosition[SLOW_MONSTER_COUNT];
		static D2D1_POINT_2F endPosition[SLOW_MONSTER_COUNT];

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Sprite& slowMonster = mSlowMonsters[i];

			if (not slowMonster.IsActive())
			{
				for (uint32_t j = 0; j < SHADOW_COUNT; ++j)
				{
					mSlowMonsterShadows[i][j].SetActive(false);
				}

				continue;
			}

			switch (mSlowMonsterState[i])
			{
			case eSlow_Monster_State::Moving:
			{
				movingTimer[i] += deltaTime;

				float t = movingTimer[i] / MOVE_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				// 갈수록 느려지는 효과이다.
				float easeOutT = 1.0f - (1.0f - t) * (1.0f - t);

				D2D1_POINT_2F position = Math::LerpVector(startPosition[i], endPosition[i], easeOutT);
				slowMonster.SetPosition(position);

				if (easeOutT >= 1.0f)
				{
					mSlowMonsterState[i] = eSlow_Monster_State::Stop;
					stopTimer[i] = 0.0f;
				}

				break;
			}

			case eSlow_Monster_State::Stop:
			{
				stopTimer[i] += deltaTime;
				if (stopTimer[i] >= STOP_TIME)
				{
					movingTimer[i] = 0.0f;

					startPosition[i] = slowMonster.GetPosition();

					D2D1_POINT_2F direction = Math::SubtractVector({}, startPosition[i]);
					direction = Math::NormalizeVector(direction);

					endPosition[i] = Math::AddVector(startPosition[i], Math::ScaleVector(direction, LENGTH));

					mSlowMonsterState[i] = eSlow_Monster_State::Moving;
				}

				break;
			}
			}

			shadowCoolTime[i] -= deltaTime;

			if (shadowCoolTime[i] <= 0.0f)
			{
				for (uint32_t j = 0; j < SHADOW_COUNT; ++j)
				{
					Sprite& shadow = mSlowMonsterShadows[i][j];

					if (shadow.IsActive())
					{
						continue;
					}

					shadow.SetOpacity(1.0f);
					shadow.SetPosition(slowMonster.GetPosition());
					shadow.SetActive(true);

					break;
				}

				shadowCoolTime[i] = 0.15f;
			}
		}
	}

	// 플레이어 체력에 관련된 부분을 업데이트한다.
	{
		// 플레이어가 죽었을 때 종료된다.
		if (mHeroHpValue <= 0)
		{
			mEndingLabel.SetActive(true);

			mHeroHpValue = 0;
			mHeroVelocity = {};
			mHero.SetActive(false);
		}

		// 플레이어 체력 라벨을 업데이트 한다.
		static int32_t prevHp = mHeroHpValue;

		if (prevHp != mHeroHpValue)
		{
			mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHeroHpValue) + L" / " + std::to_wstring(HERO_MAX_HP));

			prevHp = mHeroHpValue;
		}

		// 플레이어 체력바를 업데이트한다.
		D2D1_POINT_2F scale = { mHpBar.GetScale().width, mHpBar.GetScale().height };
		scale = Math::LerpVector(scale,
			{ UI_HP_SCALE_WIDTH * (float(mHeroHpValue) / float(HERO_MAX_HP)), scale.y },
			10.0f * deltaTime);
		mHpBar.SetScale({ scale.x, scale.y });
	}

	// 플레이어 대쉬바를 업데이트한다.
	{
		static int32_t prevDashCount = mDashCount;

		if (prevDashCount != mDashCount)
		{
			mDashValueLabel.SetText(L"Dash: " + std::to_wstring(mDashCount) + L" / " + std::to_wstring(DASH_MAX_COUNT));

			prevDashCount = mDashCount;
		}

		D2D1_POINT_2F scale = { mDashValue.GetScale().width, mDashValue.GetScale().height };
		scale = Math::LerpVector(scale, { UI_DASH_SCALE_WIDTH * (float(mDashCount) / float(DASH_MAX_COUNT)),  scale.y }, 8.0f * deltaTime);
		mDashValue.SetScale({ scale.x, scale.y });
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
		// 선과 마우스 커서의 충돌체크를 하고 좌표 이동을 한다.
		constexpr float offset = 5.0f;
		static bool isLeft, isRight, isDrag;

		if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
		{
			if (Collision::IsCollidedCircleWithPoint(mLine.Point0, TEST_RADIUS + offset, getMouseWorldPosition()))
			{
				isLeft = true;
			}
			else if (Collision::IsCollidedCircleWithPoint(mLine.Point1, TEST_RADIUS + offset, getMouseWorldPosition()))
			{
				isRight = true;
			}
		}

		if (Input::Get().GetMouseButton(Input::eMouseButton::Left))
		{
			if (isLeft)
			{
				mLine.Point0 = getMouseWorldPosition();
			}
			if (isRight)
			{
				mLine.Point1 = getMouseWorldPosition();
			}
		}
		else if (Input::Get().GetMouseButtonUp(Input::eMouseButton::Left))
		{
			isLeft = false;
			isRight = false;
		}

		// 원과 원의 충돌체크를 한다.
		const D2D1_ELLIPSE heroEllipse = { .point = mHero.GetPosition(), .radiusX = getCircleFromSprite(mHero).radiusX };

		const D2D1_POINT_2F zoomPosition = getMouseWorldPosition();
		const D2D1_ELLIPSE zoomEllipse = { .point = zoomPosition, .radiusX = getCircleFromSprite(mZoom).radiusX };

		if (Collision::IsCollidedCircleWithCircle(heroEllipse, zoomEllipse))
		{
			//DEBUG_LOG("ㅇㅇ");
		}
		else
		{
			//DEBUG_LOG("ss");
		}

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
			Sprite& bullet = mBullets[i];

			const float halfLength = bullet.GetScale().width * mRectangleTexture.GetWidth() * 0.5f;
			const D2D1_POINT_2F endPosition =
			{
				.x = bullet.GetPosition().x + mBulletDirections[i].x * halfLength,
				.y = bullet.GetPosition().y + mBulletDirections[i].y * halfLength
			};

			Line line = 
			{
				.Point0 = mPrevBulletPosition[i],
				.Point1 = endPosition
			};

			if (not Collision::IsCollidedCircleWithLine({}, BOUNDARY_RADIUS, line))
			{
				bullet.SetActive(false);
			}

			if (Collision::IsCollidedCircleWithLine({}, IN_BOUNDARY_RADIUS, line))
			{
				bullet.SetActive(false);
			}
		}

		constexpr float DAMAGE_COOL_TIMER = 0.5f;

		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];

			if (not monster.IsActive())
			{
				continue;
			}

			mMonsterDamageTimer[i] += deltaTime;

			// 플레이어 - 몬스터가 충돌하면 몬스터는 삭제된다.
			if (Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero), getRectangleFromSprite(monster)))
			{
				// 카메라 흔들기를 시작합니다.
				const float amplitude = Constant::Get().GetHeight() * getRandom(0.008f, 0.012f);
				const float duration = getRandom(0.5f, 0.8f);
				const float frequency = getRandom(50.0f, 60.0f);
				initializeCameraShake(amplitude, duration, frequency);

				if (mMonsterDamageTimer[i] >= DAMAGE_COOL_TIMER)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					monster.SetActive(false);
					mMonsterDamageTimer[i] = 0.0f;
				}

				break;
			}

			// 내부 원과 충돌하면 몬스터는 삭제된다.
			if (Collision::IsCollidedCircleWithPoint({}, IN_BOUNDARY_RADIUS, monster.GetPosition()))
			{
				mInBoundaryToMonsterTimer[i] += deltaTime;

				D2D1_POINT_2F startScale = { monster.GetScale().width, monster.GetScale().height };
				startScale = Math::LerpVector(startScale, { 0.1f, 0.1f }, 8.0f * deltaTime);
				monster.SetScale({ startScale.x, startScale.y });

				float t = (mInBoundaryToMonsterTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				startScale = Math::LerpVector(startScale, { 0.1f , 0.1f }, t);
				if (t >= 1.0f)
				{
					monster.SetActive(false);
					mInBoundaryToMonsterTimer[i] = 0.0f;
				}

				if (mMonsterDamageTimer[i] >= DAMAGE_COOL_TIMER)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					mMonsterDamageTimer[i] = 0.0f;
				}

				break;
			}
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Sprite& runMonster = mRunMonsters[i];
			if (not runMonster.IsActive())
			{
				continue;
			}

			mRunMonsterDamageTimer[i] += deltaTime;

			// 플레이어 - 돌진 몬스터가 충돌하면 몬스터는 삭제된다.
			if (Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero), getRectangleFromSprite(runMonster)))
			{
				// 카메라 흔들기를 시작합니다.
				const float amplitude = Constant::Get().GetHeight() * getRandom(0.012f, 0.020f);
				const float duration = getRandom(0.3f, 0.9f);
				const float frequency = getRandom(70.0f, 90.0f);
				initializeCameraShake(amplitude, duration, frequency);

				if (mRunMonsterDamageTimer[i] >= DAMAGE_COOL_TIMER)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					runMonster.SetActive(false);
					mRunMonsterDamageTimer[i] = 0.0f;
				}

				break;
			}

			// 내부 원과 충돌하면 돌진 몬스터는 삭제된다.
			if (Collision::IsCollidedCircleWithPoint({}, IN_BOUNDARY_RADIUS, runMonster.GetPosition()))
			{
				mInBoundaryToRunMonsterTimer[i] += deltaTime;

				D2D1_POINT_2F startScale = { runMonster.GetScale().width, runMonster.GetScale().height };
				startScale = Math::LerpVector(startScale, { 0.1f, 0.1f }, 8.0f * deltaTime);
				runMonster.SetScale({ startScale.x, startScale.y });

				float t = (mInBoundaryToRunMonsterTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				startScale = Math::LerpVector(startScale, { 0.1f , 0.1f }, t);
				if (t >= 1.0f)
				{
					runMonster.SetActive(false);
					mInBoundaryToRunMonsterTimer[i] = 0.0f;
				}

				if (mRunMonsterDamageTimer[i] >= DAMAGE_COOL_TIMER)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					mRunMonsterDamageTimer[i] = 0.0f;
				}

				break;
			}

			// 외부 원과 충돌하면 돌진 몬스터는 삭제된다.
			if (not Collision::IsCollidedCircleWithPoint({}, BOUNDARY_RADIUS, runMonster.GetPosition()))
			{
				if (mRunMonsterDamageTimer[i] >= DAMAGE_COOL_TIMER)
				{
					runMonster.SetActive(false);
					mRunMonsterDamageTimer[i] = 0.0f;
				}

				break;
			}
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Sprite& slowMonster = mSlowMonsters[i];
			if (not slowMonster.IsActive())
			{
				continue;
			}

			mSlowMonsterDamageTimer[i] += deltaTime;

			// 플레이어 - 느린 몬스터가 충돌하면 몬스터는 삭제된다.
			if (Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero), getRectangleFromSprite(slowMonster)))
			{
				// 카메라 흔들기를 시작합니다.
				const float amplitude = Constant::Get().GetHeight() * getRandom(0.008f, 0.012f);
				const float duration = getRandom(0.5f, 0.8f);
				const float frequency = getRandom(50.0f, 60.0f);
				initializeCameraShake(amplitude, duration, frequency);

				if (mSlowMonsterDamageTimer[i] >= DAMAGE_COOL_TIMER)
				{
					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					slowMonster.SetActive(false);
					mSlowMonsterDamageTimer[i] = 0.0f;
				}

				break;
			}

			// 내부 원과 충돌하면 느린 몬스터는 삭제된다.
			if (Collision::IsCollidedCircleWithPoint({}, IN_BOUNDARY_RADIUS, slowMonster.GetPosition()))
			{
				mInBoundaryToSlowMonsterTimer[i] += deltaTime;

				D2D1_POINT_2F startScale = { slowMonster.GetScale().width, slowMonster.GetScale().height };
				startScale = Math::LerpVector(startScale, { 0.1f, 0.1f }, 8.0f * deltaTime);
				slowMonster.SetScale({ startScale.x, startScale.y });

				float t = (mInBoundaryToSlowMonsterTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				startScale = Math::LerpVector(startScale, { 0.1f , 0.1f }, t);
				if (t >= 1.0f)
				{
					mInBoundaryToSlowMonsterTimer[i] = 0.0f;
				}

				if (mSlowMonsterDamageTimer[i] >= DAMAGE_COOL_TIMER)
				{
					slowMonster.SetActive(false);

					mHeroHpValue -= MONSTER_ATTACK_VALUE;
					mSlowMonsterDamageTimer[i] = 0.0f;
				}

				break;
			}
		}

		// 몬스터 - 총알에 충돌하면 몬스터는 삭제된다.
		for (uint32_t i = 0; i < BULLET_COUNT; ++i)
		{
			Sprite& bullet = mBullets[i];
			if (not bullet.IsActive())
			{
				continue;
			}

			// 이전 좌표와 현재 좌표의 직선을 그려서 충돌체크를 한다.
			const float halfLength = bullet.GetScale().width * mRectangleTexture.GetWidth() * 0.5f;
			const D2D1_POINT_2F endPosition =
			{
				.x = bullet.GetPosition().x + mBulletDirections[i].x * halfLength,
				.y = bullet.GetPosition().y + mBulletDirections[i].y * halfLength
			};

			const Line line =
			{
				.Point0 = mPrevBulletPosition[i],
				.Point1 = endPosition
			};

			// 기본 몬스터를 계산한다.
			Sprite* targetMonster = nullptr;
			float targetMonsterDistance = 999.9f;

			for (uint32_t j = 0; j < MONSTER_COUNT; ++j)
			{
				Sprite& monster = mMonsters[j];
				if (not monster.IsActive())
				{
					continue;
				}

				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(monster), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(mPrevBulletPosition[i], monster.GetPosition()));
				if (distance < targetMonsterDistance)
				{
					mIsMonsterToBullets[j] = true;

					targetMonster = &monster;
					targetMonsterDistance = distance;
				}
			}

			// 돌진 몬스터를 계산한다.
			Sprite* targetRunMonster = nullptr;
			float targetRunMonsterDistance = 999.9f;

			for (uint32_t j = 0; j < RUN_MONSTER_COUNT; ++j)
			{
				Sprite& runMonster = mRunMonsters[j];
				if (not runMonster.IsActive())
				{
					continue;
				}

				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(runMonster), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(mPrevBulletPosition[i], runMonster.GetPosition()));
				if (distance < targetRunMonsterDistance)
				{
					mIsRunMonsterToBullets[j] = true;

					targetRunMonster = &runMonster;
					targetRunMonsterDistance = distance;
				}
			}

			// 느린 몬스터를 계산한다
			Sprite* targetSlowMonster = nullptr;
			float targetSlowMonsterDistance = 999.9f;

			for (uint32_t j = 0; j < SLOW_MONSTER_COUNT; ++j)
			{
				Sprite& slowMonster = mSlowMonsters[j];
				if (not slowMonster.IsActive())
				{
					continue;
				}

				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(slowMonster), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(mPrevBulletPosition[i], slowMonster.GetPosition()));
				if (distance < targetMonsterDistance)
				{
					mIsSlowMonsterToBullets[j] = true;

					targetSlowMonster = &slowMonster;
					targetSlowMonsterDistance = distance;
				}
			}

			if (targetMonster != nullptr
				or targetRunMonster != nullptr
				or targetSlowMonster != nullptr)
			{
				bullet.SetActive(false);
			}
		}

		// 총알과 몬스터가 충돌하면, 몬스터는 사라지고 이펙트가 생성된다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			if (not mIsMonsterToBullets[i])
			{
				continue;
			}

			// 몬스터가 사라지는 이펙트가 생성된다.
			{
				mMonsterDieTimer[i] += deltaTime;

				Sprite& monster = mMonsters[i];

				D2D1_POINT_2F startScale = { monster.GetScale().width, monster.GetScale().height };
				startScale = Math::LerpVector(startScale, { 3.0f , 3.0f }, 8.0f * deltaTime);

				float t = (mMonsterDieTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				startScale = Math::LerpVector(startScale, { 0.1f , 0.1f }, t);

				if (t >= 1.0f)
				{
					monster.SetActive(false);
					mIsMonsterToBullets[i] = false;
					mMonsterDieTimer[i] = 0.0f;
				}

				monster.SetScale({ startScale.x , startScale.y });
			}

			// 총알 이펙트가 생성된다.
			{
				mMonsterDieEffectTimer[i] += deltaTime;

				// 크기를 보간한다.
				D2D1_POINT_2F scale = { mMonsterBulletEffectScales[i].width, mMonsterBulletEffectScales[i].height };
				scale = Math::LerpVector(scale, { 100.0f , 100.0f }, 5.0f * deltaTime);
				mMonsterBulletEffectScales[i] = { scale.x, scale.y };

				// 두께를 보간한다.
				mMonsterThicks[i] = { .x = 20.0f, .y = 20.0f };
				float t = (mMonsterDieEffectTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);
				mMonsterThicks[i] = Math::LerpVector(mMonsterThicks[i], { 0.1f , 0.1f }, t);

				if (t >= 1.0f)
				{
					mMonsterDieEffectTimer[i] = 0.0f;
				}
			}
		}

		// 총알과 돌진 몬스터가 충돌하면, 돌진 몬스터는 사라지고 이펙트가 생성된다.
		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			if (not mIsRunMonsterToBullets[i])
			{
				continue;
			}

			// 돌진 몬스터가 사라지는 이펙트가 생성된다.
			{
				mRunMonsterDieTimer[i] += deltaTime;

				Sprite& runMonster = mRunMonsters[i];

				D2D1_POINT_2F startScale = { runMonster.GetScale().width, runMonster.GetScale().height };
				startScale = Math::LerpVector(startScale, { 1.5f, 1.5f }, 8.0f * deltaTime);

				float t = (mRunMonsterDieTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				startScale = Math::LerpVector(startScale, { 0.1f, 0.1f }, t);

				if (t >= 1.0f)
				{
					runMonster.SetActive(false);
					mIsRunMonsterToBullets[i] = false;
					mRunMonsterDieTimer[i] = 0.0f;
				}

				runMonster.SetScale({ startScale.x , startScale.y });
			}

			// 총알 이펙트가 생성된다.
			{
				mRunMonsterDieEffectTimer[i] += deltaTime;

				// 크기를 보간한다.
				D2D1_POINT_2F scale = { mRunMonsterBulletEffectScales[i].width, mRunMonsterBulletEffectScales[i].height };
				scale = Math::LerpVector(scale, { 60.0f , 60.0f }, 5.0f * deltaTime);
				mRunMonsterBulletEffectScales[i] = { scale.x, scale.y };

				// 두께를 보간한다.
				mRunMonsterThicks[i] = { .x = 15.0f, .y = 15.0f };
				float t = (mRunMonsterDieEffectTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);
				mRunMonsterThicks[i] = Math::LerpVector(mRunMonsterThicks[i], { 0.1f , 0.1f }, t);

				if (t >= 1.0f)
				{
					mRunMonsterDieEffectTimer[i] = 0.0f;
				}
			}
		}

		// 총알과 느린 몬스터가 충돌하면, 느린 몬스터는 사라지고 이펙트가 생성된다.
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			if (not mIsSlowMonsterToBullets[i])
			{
				continue;
			}
			
			// 느린 몬스터가 사라지는 이펙트가 생성된다.
			{
				mSlowMonsterDieTimer[i] += deltaTime;

				Sprite& slowMonster = mSlowMonsters[i];

				D2D1_POINT_2F startScale = { slowMonster.GetScale().width, slowMonster.GetScale().height };
				startScale = Math::LerpVector(startScale, { 1.5f, 1.5f }, 8.0f * deltaTime);

				float t = (mSlowMonsterDieTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				startScale = Math::LerpVector(startScale, { 0.1f, 0.1f }, t);
				
				if (t >= 1.0f)
				{
					slowMonster.SetActive(false);
					mIsSlowMonsterToBullets[i] = false;
					mSlowMonsterDieTimer[i] = 0.0f;
				}

				slowMonster.SetScale({ startScale.x , startScale.y });
			}

			// 총알 이펙트가 생성된다.
			{
				mSlowMonsterDieEffectTimer[i] += deltaTime;

				// 크기를 보간한다.
				D2D1_POINT_2F scale = { mSlowMonsterBulletEffectScales[i].width, mSlowMonsterBulletEffectScales[i].height };
				scale = Math::LerpVector(scale, { 60.0f , 60.0f }, 5.0f * deltaTime);
				mSlowMonsterBulletEffectScales[i] = { scale.x, scale.y };

				// 두께를 보간한다.
				mSlowMonsterThicks[i] = {.x = 15.0f, .y = 15.0f };
				float t = (mSlowMonsterDieEffectTimer[i] - START_LERP_TIME) / DURING_TIME;
				t = std::clamp(t, 0.0f, 1.0f);

				mSlowMonsterThicks[i] = Math::LerpVector(mSlowMonsterThicks[i], { 0.1f , 0.1f }, t);

				if (t >= 1.0f)
				{
					mSlowMonsterDieEffectTimer[i] = 0.0f;
				}
			}
		}

		// 플레이어 쉴드와 몬스터가 충돌하면 몬스터는 삭제된다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];

			D2D1_RECT_F rect = getRectangleFromSprite(monster);

			Line leftLine =
			{
				.Point0 = { .x = rect.left, .y = rect.top },
				.Point1 = {.x = rect.right, .y = rect.bottom },
			};

			if (Collision::IsCollidedCircleWithLine(mHero.GetPosition(), mShieldScale.width * 0.5f, leftLine))
			{
				monster.SetActive(false);
			}

			Line topLine =
			{
				.Point0 = {.x = rect.left, .y = rect.top },
				.Point1 = {.x = rect.right, .y = rect.top },
			};

			if (Collision::IsCollidedCircleWithLine(mHero.GetPosition(), mShieldScale.width * 0.5f, topLine))
			{
				monster.SetActive(false);

			}

			Line rightLine =
			{
				.Point0 = {.x = rect.right, .y = rect.top },
				.Point1 = {.x = rect.right, .y = rect.bottom },
			};

			if (Collision::IsCollidedCircleWithLine(mHero.GetPosition(), mShieldScale.width * 0.5f, rightLine))
			{
				monster.SetActive(false);
			}

			Line bottomLine =
			{
				.Point0 = {.x = rect.left, .y = rect.bottom },
				.Point1 = {.x = rect.right, .y = rect.bottom },
			};

			if (Collision::IsCollidedCircleWithLine(mHero.GetPosition(), mShieldScale.width * 0.5f, bottomLine))
			{
				monster.SetActive(false);
			}
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Sprite& runMonster = mRunMonsters[i];
			if (Collision::IsCollidedCircleWithPoint(mHero.GetPosition(), mShieldScale.width * 0.5f, runMonster.GetPosition()))
			{
				runMonster.SetActive(false);
			}
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Sprite& slowMonster = mSlowMonsters[i];
			if (Collision::IsCollidedCircleWithPoint(mHero.GetPosition(), mShieldScale.width * 0.5f, slowMonster.GetPosition()))
			{
				slowMonster.SetActive(false);
			}
		}

		// 플레이어 주변을 공전하는 원과 몬스터가 충돌하면 몬스터는 삭제된다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];
			if (Collision::IsCollidedCircleWithPoint(Math::AddVector(mOrbitEllipse.point, mHero.GetPosition()), mOrbitEllipse.radiusX, monster.GetPosition()))
			{
				monster.SetActive(false);
			}
		}

		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Sprite& runMonster = mRunMonsters[i];
			if (Collision::IsCollidedCircleWithPoint(Math::AddVector(mOrbitEllipse.point, mHero.GetPosition()), mOrbitEllipse.radiusX, runMonster.GetPosition()))
			{
				runMonster.SetActive(false);
			}
		}

		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			Sprite& slowMonster = mSlowMonsters[i];
			if (Collision::IsCollidedCircleWithPoint(Math::AddVector(mOrbitEllipse.point, mHero.GetPosition()), mOrbitEllipse.radiusX, slowMonster.GetPosition()))
			{
				slowMonster.SetActive(false);
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
			if (not mIsMonsterToBullets[i])
			{
				continue;
			}

			Sprite& monster = mMonsters[i];

			const Matrix3x2F worldView = Transformation::getWorldMatrix({ monster.GetPosition().x, monster.GetPosition().y + 50.0f }, 45.0f) * view;
			renderTarget->SetTransform(worldView);

			const D2D1_RECT_F colliderSize =
			{
				.left = 0.0f,
				.top = 0.0f,
				.right = mMonsterBulletEffectScales[i].width,
				.bottom = mMonsterBulletEffectScales[i].height
			};

			renderTarget->DrawRectangle(colliderSize, mCyanBrush, mMonsterThicks[i].x);
		}
	}

	// 총알과 돌진 몬스터가 충돌하면, 이펙트를 그린다.
	{
		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			if (not mIsRunMonsterToBullets[i])
			{
				continue;
			}

			Sprite& runMonster = mRunMonsters[i];

			const Matrix3x2F worldView = Transformation::getWorldMatrix({ runMonster.GetPosition().x, runMonster.GetPosition().y + 35.0f }, 45.0f) * view;
			renderTarget->SetTransform(worldView);

			const D2D1_RECT_F colliderSize =
			{
				.left = 0.0f,
				.top = 0.0f,
				.right = mRunMonsterBulletEffectScales[i].width,
				.bottom = mRunMonsterBulletEffectScales[i].height
			};

			renderTarget->DrawRectangle(colliderSize, mCyanBrush, mRunMonsterThicks[i].x);
		}
	}

	// 총알과 느린 몬스터가 충돌하면, 이펙트를 그린다.
	{
		for (uint32_t i = 0; i < SLOW_MONSTER_COUNT; ++i)
		{
			if (not mIsSlowMonsterToBullets[i])
			{
				continue;
			}

			Sprite& slowMonster = mSlowMonsters[i];

			const Matrix3x2F worldView = Transformation::getWorldMatrix({ slowMonster.GetPosition().x, slowMonster.GetPosition().y + 35.0f }, 45.0f) * view;
			renderTarget->SetTransform(worldView);

			const D2D1_RECT_F colliderSize =
			{
				.left = 0.0f,
				.top = 0.0f,
				.right = mSlowMonsterBulletEffectScales[i].width,
				.bottom = mSlowMonsterBulletEffectScales[i].height
			};

			renderTarget->DrawRectangle(colliderSize, mCyanBrush, mSlowMonsterThicks[i].x);
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
			Sprite& monster = mMonsters[i];

			if (mIsColliderKeyDown and monster.IsActive())
			{
				const Matrix3x2F worldView = Transformation::getWorldMatrix({ .x = getRectangleFromSprite(monster).left, .y = getRectangleFromSprite(monster).top }) * view;
				renderTarget->SetTransform(worldView);

				const D2D1_SIZE_F scale = monster.GetScale();

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
			Sprite& bullet = mBullets[i];

			if (mIsColliderKeyDown and bullet.IsActive())
			{
				const Matrix3x2F worldView = Transformation::getWorldMatrix(getCircleFromSprite(bullet).point) * view;
				renderTarget->SetTransform(worldView);

				const D2D1_SIZE_F scale = bullet.GetScale();

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
