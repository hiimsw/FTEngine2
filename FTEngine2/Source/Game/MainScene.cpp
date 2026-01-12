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
		mHpFont.Initialize(GetHelper(), L"Arial", 20.0f);
		mEndingFont.Initialize(GetHelper(), L"Arial", 50.0f);

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

		mCircleTexture.Initialize(GetHelper(), L"Resource/Circle.png");
		mRedCircleTexture.Initialize(GetHelper(), L"Resource/RedCircle.png");
	}

	// 플레이어를 초기화한다.
	{
		mHero.SetPosition({ .x = -200.0f, .y = 0.0f });
		mHero.SetTexture(&mCircleTexture);
		mSpriteLayers[uint32_t(Layer::Player)].push_back(&mHero);
	}

	// 총알을 초기화한다.
	{
		for (uint32_t i = 0; i < BULLET_COUNT; ++i)
		{
			mBullets[i].SetPosition(mHero.GetPosition());
			mBullets[i].SetScale({ .width = 0.7f, .height = 0.7f });
			mBullets[i].SetActive(false);
			mBullets[i].SetTexture(&mCircleTexture);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&mBullets[i]);
		}
	}

	// 몬스터를 초기화한다.
	{
		constexpr float MONSTER_SCALE = 0.7f;

		// 랜덤 좌표를 생성한다.
		for (Sprite& monster : mMonsters)
		{
			monster.SetTexture(&mRectangleTexture);
			monster.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });
			monster.SetActive(false);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&monster);
		}
	}

	// 돌진 몬스터를 초기화한다.
	{
		constexpr float MONSTER_SCALE = 0.4f;

		// 랜덤 좌표를 생성한다.
		for (Sprite& monster : mRunMonsters)
		{
			monster.SetTexture(&mRectangleTexture);
			monster.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });
			monster.SetActive(false);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&monster);
		}
	}

	// 줌을 초기화한다.
	{
		mZoom.SetScale({ .width = 0.7f, .height = 0.7f });
		mZoom.SetAngle(0.0f);
		mZoom.SetUI(true);
		mZoom.SetTexture(&mRedCircleTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mZoom);
	}

	// 카메라를 초기화한다.
	{
		const D2D1_POINT_2F heroPosition = mHero.GetPosition();
		const D2D1_POINT_2F cameraPosition{ .x = heroPosition.x * 0.5f, .y = heroPosition.y * 0.5f };

		mMainCamera.SetPosition(cameraPosition);
	}

	// HP바를 초기화한다.
	{
		mHpBar.SetPosition({ .x = -UI_HP_SCALE_WIDTH * 0.5f * mRedRectangleTexture.GetWidth(), .y = -UI_CENTER_POSITION_Y });
		mHpBar.SetScale({ .width = UI_HP_SCALE_WIDTH, .height = 1.0f });
		mHpBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mHpBar.SetUI(true);
		mHpBar.SetTexture(&mRedRectangleTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mHpBar);
	}

	// 라벨을 초기화한다.
	{
		// 타이머
		mTimerLabel.SetFont(&mTimerFont);
		mTimerLabel.SetUI(true);
		mTimerLabel.SetPosition({ .x = 0.0f, .y = UI_CENTER_POSITION_Y });
		mLabels.push_back(&mTimerLabel);

		// 현재 체력
		mHpValueLabel.SetFont(&mHpFont);
		mHpValueLabel.SetUI(true);

		const D2D1_POINT_2F hpBarPosition = mHpBar.GetPosition();
		constexpr float OFFSET_Y = 50.0f;
		D2D1_POINT_2F offset = { .x = hpBarPosition.x, .y = hpBarPosition.y + OFFSET_Y };
		mHpValueLabel.SetPosition(offset);

		mHpValueLabel.SetCenter({ .x = -0.5f, .y = 0.0f });
		mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHeroHpValue) + L" / " + std::to_wstring(mHeroHpMax));
		mLabels.push_back(&mHpValueLabel);

		// 엔딩
		mEndingLabel.SetFont(&mEndingFont);
		mEndingLabel.SetUI(true);
		mEndingLabel.SetPosition({});
		mEndingLabel.SetActive(false);
		mEndingLabel.SetText(L"GameOver");
		mLabels.push_back(&mEndingLabel);
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

		constexpr float MIN_LENGTH = 120.0f;
		const D2D1_POINT_2F heroPosition = mHero.GetPosition();
		const D2D1_POINT_2F zoomPosition = getMouseWorldPosition();

		D2D1_POINT_2F toTarget = Math::SubtractVector(zoomPosition, heroPosition);
		const float length = Math::GetVectorLength(toTarget);

		D2D1_SIZE_F zoomScale = mZoom.GetScale();
		float scaleSpeed = 0.8f;

		if (fabs(length) <= MIN_LENGTH)
		{
			if (zoomScale.width >= 1.3f)
			{
				scaleSpeed = 0.0f;
			}

			zoomScale.width += scaleSpeed * deltaTime;
			zoomScale.height += scaleSpeed * deltaTime;
		}
		else
		{
			if (zoomScale.width <= 0.7f)
			{
				scaleSpeed = 0.0f;
			}

			zoomScale.width -= scaleSpeed * deltaTime;
			zoomScale.height -= scaleSpeed * deltaTime;
		}

		mZoom.SetScale(zoomScale);
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
			constexpr float MAX_SPEED = 280.0f;
			constexpr float ACC = 20.0f; // 가속도

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
			constexpr float MAX_DASH_SPEED = 400.0f;
			constexpr float DASH_ACC = 30.0f;
			static float dashSpeed = 0.0f;
			static bool bDashing = false;
			static D2D1_POINT_2F dashDirection{};

			if (Math::GetVectorLength(mHeroVelocity) != 0.0f)
			{
				float speed = min(Math::GetVectorLength(mHeroVelocity), MAX_SPEED);
				const D2D1_POINT_2F direction = Math::NormalizeVector(mHeroVelocity);
				D2D1_POINT_2F adjustVelocity = Math::ScaleVector(direction, speed);
				adjustVelocity = Math::ScaleVector(adjustVelocity, deltaTime);

				if (not bDashing and Input::Get().GetKeyDown(VK_SPACE))
				{
					bDashing = true;
					dashDirection = direction;
				}

				const D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), adjustVelocity);
				mHero.SetPosition(position);
			}

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
			}
		}

		// 총알을 업데이트한다.
		{
			static float lifetime[BULLET_COUNT];
			static D2D1_POINT_2F direction[BULLET_COUNT];

			// 총알을 스폰한다.
			if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
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
					direction[i] = Math::SubtractVector(getMouseWorldPosition(), spawnPosition);
					direction[i] = Math::RotateVector(direction[i], getRandom(-5.0f, 5.0f));
					direction[i] = Math::NormalizeVector(direction[i]);

					lifetime[i] = 0.0f;
					bullet.SetActive(true);

					// 카메라 흔들기를 시작합니다.
					{
						const float amplitude = Constant::Get().GetHeight() * getRandom(0.008f, 0.012f);
						const float duration = getRandom(0.05f, 0.08f);
						const float frequency = getRandom(50.0f, 60.0f);
						initializeCameraShake(amplitude, duration, frequency);
					}

					break;
				}
			}

			constexpr float MOVE_SPEED = 1200.0f;

			// 총알을 이동시킨다.
			for (uint32_t i = 0; i < BULLET_COUNT; ++i)
			{
				Sprite& bullet = mBullets[i];
				if (not bullet.IsActive())
				{
					continue;
				}

				const D2D1_POINT_2F velocity = Math::ScaleVector(direction[i], MOVE_SPEED * deltaTime);
				const D2D1_POINT_2F position = Math::AddVector(bullet.GetPosition(), velocity);

				lifetime[i] += deltaTime;
				if (lifetime[i] >= 1.5f)
				{
					bullet.SetActive(false);
				}

				mPrevBulletPosition[i] = bullet.GetPosition();
				bullet.SetPosition(position);
			}
		}
	}

	// 몬스터를 업데이트한다.
	{		
		static float speed[MONSTER_COUNT];

		// 몬스터를 일정 시간마다 스폰한다.
		mSpawnTimer += deltaTime;
		if (mSpawnTimer >= 0.5f)
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
				monster.SetActive(true);

				mSpawnTimer = 0.0f;		

				// 스폰할 때 속도도 같이 초기화한다.
				speed[i] = getRandom(10.0f, 80.0f);

				break;
			}
		}

		// 몬스터를 이동시킨다.
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];
			if (not monster.IsActive())
			{
				continue;
			}

			D2D1_POINT_2F position = monster.GetPosition();
			const D2D1_POINT_2F toTarget = Math::SubtractVector({}, position);

			if (Math::GetVectorLength(toTarget) != 0.0f)
			{
				const D2D1_POINT_2F direction = Math::NormalizeVector(toTarget);
				const D2D1_POINT_2F velocity = Math::ScaleVector(direction, speed[i] * deltaTime);
				
				position = Math::AddVector(position, velocity);
				monster.SetPosition(position);
			}
		}
	}

	// 돌진 몬스터를 업데이트한다.
	{
		static float speed[RUN_MONSTER_COUNT];
		static D2D1_POINT_2F toTarget;
		static D2D1_POINT_2F direction[RUN_MONSTER_COUNT];

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
				monster.SetActive(true);

				mRunMonsterSpawnTimer = 0.0f;

				// 스폰할 때 속도와 이동방향을 세팅한다.
				speed[i] = getRandom(10.0f, 80.0f);

				D2D1_POINT_2F monsterPosition = monster.GetPosition();
				D2D1_POINT_2F heroPosition = mHero.GetPosition();
				toTarget = Math::SubtractVector(heroPosition, monsterPosition);
				direction[i] = Math::NormalizeVector(toTarget);

				break;
			}
		}

		// 이동한다.
		for (uint32_t i = 0; i < RUN_MONSTER_COUNT; ++i)
		{
			Sprite& monster = mRunMonsters[i];
			if (not monster.IsActive())
			{
				continue;
			}

			if (Math::GetVectorLength(toTarget) != 0.0f)
			{
				const D2D1_POINT_2F velocity = Math::ScaleVector(direction[i], speed[i] * deltaTime);

				D2D1_POINT_2F position = monster.GetPosition();
				position = Math::AddVector(position, velocity);
				monster.SetPosition(position);
			}
		}

	}

	// 플레이어 체력바를 업데이트한다.
	{
		static int32_t prevHp = mHeroHpValue;

		if (mHeroHpValue <= 0)
		{
			mEndingLabel.SetActive(true);

			mHeroHpValue = 0;
			mHeroVelocity = {};
			mHero.SetActive(false);
		}

		if (prevHp != mHeroHpValue)
		{
			const D2D1_SIZE_F prevScale{ .width = UI_HP_SCALE_WIDTH * float(mHeroHpValue) / mHeroHpMax, .height = 1.0f };
			mHpBar.SetScale(prevScale);

			mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHeroHpValue) + L" / " + std::to_wstring(mHeroHpMax));

			prevHp = mHeroHpValue;
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
			if (Collision::IsCollidedCircleWithPoint(mLine.Point0, RADIUS + offset, getMouseWorldPosition()))
			{
				isLeft = true;
			}
			else if (Collision::IsCollidedCircleWithPoint(mLine.Point1, RADIUS + offset, getMouseWorldPosition()))
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

		constexpr float DAMAGE_COOL_TIMER = 0.5f;

		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];

			if (not monster.IsActive())
			{
				continue;
			}

			mDamageTimer += deltaTime;

			// 플레이어 - 몬스터가 충돌하면 몬스터는 삭제된다.
			if (Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero), getRectangleFromSprite(monster)))
			{
				if (mDamageTimer >= DAMAGE_COOL_TIMER)
				{
					mHeroHpValue -= mMonsterAttackValue;
					monster.SetActive(false);
					mDamageTimer = 0.0f;
				}

				break;
			}

			// 내부 원과 충돌하면 몬스터는 삭제된다.
			if (Collision::IsCollidedCircleWithPoint({}, IN_BOUNDARY_RADIUS, monster.GetPosition()))
			{
				if (mDamageTimer >= DAMAGE_COOL_TIMER)
				{
					mHeroHpValue -= mMonsterAttackValue;
					monster.SetActive(false);
					mDamageTimer = 0.0f;
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
			const Line line =
			{
				.Point0 = mPrevBulletPosition[i],
				.Point1 = bullet.GetPosition()
			};

			Sprite* targetMonster = nullptr;
			float targetMonsterDistance = 999.9f;

			for (Sprite& monster : mMonsters)
			{
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
					targetMonster = &monster;
					targetMonsterDistance = distance;
				}
			}

			if (targetMonster != nullptr)
			{
				bullet.SetActive(false);
				targetMonster->SetActive(false);
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

	// 라인을 그린다.
	{
		const D2D1_ELLIPSE CIRCLE{ .radiusX = RADIUS, .radiusY = RADIUS };

		Matrix3x2F point0WorldView = Transformation::getWorldMatrix(mLine.Point0) * view;
		Matrix3x2F point1WorldView = Transformation::getWorldMatrix(mLine.Point1) * view;

		// 두 점을 그린다.
		{
			renderTarget->SetTransform(point0WorldView);
			renderTarget->DrawEllipse(CIRCLE, mDefaultBrush);

			renderTarget->SetTransform(point1WorldView);
			renderTarget->DrawEllipse(CIRCLE, mDefaultBrush);
		}

		// 라인을 그린다.
		{
			const D2D1_POINT_2F point0 = D2D1_POINT_2F{ .x = 0.0f, .y = 0.0f } *point0WorldView;
			const D2D1_POINT_2F point1 = D2D1_POINT_2F{ .x = 0.0f, .y = 0.0f } *point1WorldView;
			renderTarget->SetTransform(Matrix3x2F::Identity());
			renderTarget->DrawLine(point0, point1, mDefaultBrush);
		}
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
}

void MainScene::Finalize()
{
	RELEASE_D2D1(mCyanBrush);
	RELEASE_D2D1(mYellowBrush);
	RELEASE_D2D1(mDefaultBrush);

	mRectangleTexture.Finalize();
	mRedRectangleTexture.Finalize();

	mCircleTexture.Finalize();
	mRedCircleTexture.Finalize();
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
	mCameraShakeAmplitude = amplitude;
	mCameraShakeDuration = duration;
	mCameraShakeFrequency = frequency;
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
