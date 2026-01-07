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
	{
		for (std::vector<Sprite*>& layer : mSpriteLayers)
		{
			layer.reserve(128);
		}

		SetSpriteLayers(mSpriteLayers.data(), uint32_t(mSpriteLayers.size()));

		SetCamera(&mMainCamera);

		mLabels.reserve(1);
		SetLabels(&mLabels);

		mTimerFont.Initialize(GetHelper(), L"Arial", 40.0f);

		Input::Get().SetCursorVisible(false);
		Input::Get().SetCursorLockState(Input::eCursorLockState::Confined);

		ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();
		HR(renderTarget->CreateSolidColorBrush(ColorF(1.0f, 1.0f, 1.0f), &mDefaultBrush));

		mIsCursorConfined = (Input::Get().GetCursorLockState() == Input::eCursorLockState::Confined);
	}

	mRectangleTexture.Initialize(GetHelper(), L"Resource/Rectangle.png");
	mRedRectangleTexture.Initialize(GetHelper(), L"Resource/RedRectangle.png");

	mCircleTexture.Initialize(GetHelper(), L"Resource/Circle.png");
	mRedCircleTexture.Initialize(GetHelper(), L"Resource/RedCircle.png");

	// 플레이어
	{
		// 네모
		mHero.SetTexture(&mRectangleTexture);
		mSpriteLayers[uint32_t(Layer::Player)].push_back(&mHero);

		// 동그라미
		//mHero.SetScale({ .width = 5.0f, .height = 5.0f });
		//mHero.SetTexture(&mCircleTexture);
		//mSpriteLayers[uint32_t(Layer::Player)].push_back(&mHero);
	}

	// 총알
	{
		mBullet.SetPosition(mHero.GetPosition());
		mBullet.SetScale({ .width = 0.7f, .height = 0.7f });
		mBullet.SetActive(false);
		mBullet.SetTexture(&mCircleTexture);
		mSpriteLayers[uint32_t(Layer::Player)].push_back(&mBullet);
	}

	// 몬스터를 초기화한다.
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(20, 999);
		std::uniform_int_distribution<uint32_t> distDir(0, 1);

		uint32_t cnt{};

		constexpr float MONSTER_SCALE = 0.5f;

		// 랜덤 좌표를 생성한다.
		while (cnt != MONSTER_COUNT)
		{
			Sprite& monster = mMonsters[cnt];

			D2D1_POINT_2F spawnPosition = { .x = dist(gen), .y = dist(gen) };

			if (distDir(gen))
			{
				spawnPosition.x *= -1.0f;
			}
			if (distDir(gen))
			{
				spawnPosition.y *= -1.0f;
			}

			const D2D1_POINT_2F spawnDirection = Math::GetNormalizeVector(spawnPosition);
			const D2D1_POINT_2F spawnPositionInCircle = Math::ScaleVector(spawnDirection, OUTLINE_OFFSET);

			monster.SetTexture(&mRectangleTexture);
			monster.SetPosition(spawnPositionInCircle);
			monster.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });
			monster.SetActive(true);

			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&monster);
			cnt++;
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

	// HP바를 초기화한다.
	{
		mHp.SetPosition({ .x = 0.0f, .y = -UI_CENTER_POSITION_Y });
		mHp.SetScale({ .width = UI_HP_SCALE_WIDTH, .height = 1.0f });
		mHp.SetUI(true);
		mHp.SetTexture(&mRedRectangleTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mHp);
	}

	// 라벨을 초기화한다.
	{
		mTimerLabel.SetFont(&mTimerFont);
		mTimerLabel.SetUI(true);
		mTimerLabel.SetPosition({ .x = 0.0f, .y = UI_CENTER_POSITION_Y });
		mLabels.push_back(&mTimerLabel);

	}

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
	}

	// 카메라를 업데이트한다.
	{
		D2D1_POINT_2F position = mMainCamera.GetPosition();
		position = mHero.GetPosition();

		//std::lerp()

		mMainCamera.SetPosition(position);
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
	}

	// 플레이어를 업데이트한다.
	{
		if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
		{
			mIsBulletKeyDown = true;
		}

		// Case 0) 가속도 없는 단순한 이동
		/*{
			// 누르면 1, 안 누르면 0
			int32_t moveX = Input::Get().GetKey('D') - Input::Get().GetKey('A');
			int32_t moveY = Input::Get().GetKey('W') - Input::Get().GetKey('S');

			if (moveX != 0 or moveY != 0)
			{
				float speed = 180.0f * deltaTime;
				D2D1_POINT_2F direction = Math::GetNormalizeVector({ .x = float(moveX), .y = float(moveY) });
				D2D1_POINT_2F velocity = Math::ScaleVector(direction, speed);

				D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), velocity);
				mHero.SetPosition(position);
			}
		}*/

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
			constexpr float ACC = 8.0f; // 가속도

			int32_t moveX = Input::Get().GetKey('D') - Input::Get().GetKey('A');
			int32_t moveY = Input::Get().GetKey('W') - Input::Get().GetKey('S');

			static int32_t previousMoveX;
			static int32_t previousMoveY;
			static D2D1_POINT_2F velocity;

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

			if (Math::GetVectorLength(velocity) != 0.0f)
			{
				float speed = min(Math::GetVectorLength(velocity), MAX_SPEED);
				const D2D1_POINT_2F direction = Math::GetNormalizeVector(velocity);
				D2D1_POINT_2F adjustVelocity = Math::ScaleVector(direction, speed);
				adjustVelocity = Math::ScaleVector(adjustVelocity, deltaTime);

				D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), adjustVelocity);
				mHero.SetPosition(position);
			}
		}

		// 총알을 업데이트한다.
		{
			static bool isMoving;
			static D2D1_POINT_2F bulletPosition;
			static D2D1_POINT_2F toTarget;

			// 키를 눌렀을 때마다 총알 좌표를 플레이어 좌표로 바꾼다.
			// 그에 맞게 계산을 한다.
			if (mIsBulletKeyDown and not isMoving)
			{
				mBullet.SetActive(true);

				bulletPosition = mHero.GetPosition();
				mPrevBulletPosition = bulletPosition;

				const D2D1_POINT_2F zoomPosition = mZoom.GetPosition();
				const D2D1_POINT_2F cameraPosition = mMainCamera.GetPosition();

				// 줌 좌표를 월드 좌표로 바꾼다.
				const D2D1_POINT_2F zoomWorldPosition = Math::AddVector(zoomPosition, cameraPosition);

				// 총알과 줌의 벡터를 구한다.
				toTarget = Math::SubtractVector(zoomWorldPosition, bulletPosition);

				isMoving = true;
			}

			if (isMoving)
			{
				static float elapsedTime;
				elapsedTime += deltaTime;

				constexpr uint32_t MAX_SPEED = 600;

				if (Math::GetVectorLength(toTarget) != 0.0f)
				{
					const D2D1_POINT_2F direction = Math::GetNormalizeVector(toTarget);
					D2D1_POINT_2F velocity = Math::ScaleVector(direction, MAX_SPEED);
					D2D1_POINT_2F movePosition = Math::ScaleVector(velocity, deltaTime);

					bulletPosition = Math::AddVector(bulletPosition, movePosition);
				}

				mBullet.SetPosition(bulletPosition);

				if (elapsedTime >= 0.7f)
				{
					isMoving = false;

					mBullet.SetActive(false);
					mIsBulletKeyDown = false;
					elapsedTime = 0.0f;
				}
			}
		}
	}

	// 몬스터를 업데이트한다.
	{
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];
			D2D1_POINT_2F position = monster.GetPosition();

			if (monster.IsActive())
			{
				const D2D1_POINT_2F toTarget = Math::SubtractVector({}, position);

				if (Math::GetVectorLength(toTarget) != 0.0f)
				{
					const D2D1_POINT_2F direction = Math::GetNormalizeVector(toTarget);

					constexpr int32_t MAX_SPEED = 70;
					const D2D1_POINT_2F velocity = Math::ScaleVector(direction, MAX_SPEED);
					const D2D1_POINT_2F movePosition = Math::ScaleVector(velocity, deltaTime);

					position = Math::AddVector(position, movePosition);
				}
			}

			static float spawnTimer;
			spawnTimer += deltaTime;

			// 충돌이 되면 IsActive() = false니까 내부에서 true로 바꿔줘야 한다.
			if (not monster.IsActive() and mIsMonsterSpwan[i])
			{
				if (spawnTimer >= 0.5f)
				{
					// 여기 부분을 말한다.
					mMonsters[i].SetActive(true);

					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_real_distribution<float> dist(20, 999);
					std::uniform_int_distribution<uint32_t> distDir(0, 1);

					position = { .x = dist(gen), .y = dist(gen) };

					if (distDir(gen))
					{
						position.x *= -1.0f;
					}
					if (distDir(gen))
					{
						position.y *= -1.0f;
					}

					const D2D1_POINT_2F spawnDirection = Math::GetNormalizeVector(position);
					const D2D1_POINT_2F spawnPositionInCircle = Math::ScaleVector(spawnDirection, OUTLINE_OFFSET);
					position = spawnPositionInCircle;

					spawnTimer = 0.0f;
				}

				mIsMonsterSpwan[i] = false;
			}

			monster.SetPosition(position);
		}
	}
	
	// 플레이어 체력바를 업데이트한다.
	{
		static int32_t prevHp = mHeroHpValue;

		if (mHeroHpValue <= 0)
		{
			mHeroHpValue = 0;
		}
		if (prevHp != mHeroHpValue)
		{
			D2D1_SIZE_F prevScale = mHp.GetScale();
			prevScale = { .width = UI_HP_SCALE_WIDTH * float(mHeroHpValue) / mHeroMaxHp, .height = 1.0f };
			mHp.SetScale(prevScale);

			prevHp = mHeroHpValue;
		}

		DEBUG_LOG("%d", mHeroHpValue);
	}

	// 라벨을 업데이트한다.
	{
		static float gameTimer;
		gameTimer += deltaTime;

		uint32_t seconds = uint32_t(gameTimer) % 60;
		uint32_t minutes = uint32_t(gameTimer) / 60;

		std::wstring name = L"Timer: " + std::to_wstring(minutes) + L":" + std::to_wstring(seconds);
		mTimerLabel.SetText(name);
	}

	// 충돌	처리를 업데이트한다.
	{
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			Sprite& monster = mMonsters[i];

			if (not monster.IsActive())
			{
				mIsHeroMonsterColliding[i] = false;
				mIsMonsterInBoundaryColliding[i] = false;
				mIsMonsterBulletColliding[i] = false;

				continue;
			}

			// 플레이어 - 몬스터가 충돌하면 몬스터는 삭제된다.
			if (Collision::IsCollidedSqureWithSqure(GetRectangleFromSprite(mHero), GetRectangleFromSprite(monster)))
			{
				mIsHeroMonsterColliding[i] = true;
			}
			else
			{
				mIsHeroMonsterColliding[i] = false;
			}

			// 내부 원과 충돌하면 몬스터는 삭제된다.
			if (Collision::IsCollidedCircleWithPoint({}, IN_BOUNDARY_RADIUS, monster.GetPosition()))
			{
				mIsMonsterInBoundaryColliding[i] = true;
			}
			else
			{
				mIsMonsterInBoundaryColliding[i] = false;
			}

			// 몬스터 - 총알에 충돌하면 몬스터는 삭제된다.
			Line line =
			{
				.Point0 = mPrevBulletPosition,
				.Point1 = mBullet.GetPosition()
			};

			if (Collision::IsCollidedSqureWithLine(GetRectangleFromSprite(monster), line))
			{
				mIsMonsterBulletColliding[i] = true;
			}
			else
			{
				mIsMonsterBulletColliding[i] = false;
			}
		}

		Line line =
		{
			.Point0 = mLine.Point0,
			.Point1 = mLine.Point1
		};

		if (Collision::IsCollidedSqureWithLine(GetRectangleFromSprite(mHero), line))
		{
			//DEBUG_LOG("dd");
		}
		else
		{
			//DEBUG_LOG("ss");
		}
	}

	// 충돌을 적용한다.
	{
		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			// 플레이어 - 몬스터
			if (mIsHeroMonsterColliding[i])
			{
				mMonsters[i].SetActive(false);
				mIsMonsterSpwan[i] = true;

				mHeroHpValue -= mMonsterAttackValue;
			}

			// 몬스터 - 가운데 원
			if (mIsMonsterInBoundaryColliding[i])
			{
				mMonsters[i].SetActive(false);
				mIsMonsterSpwan[i] = true;

				mHeroHpValue -= mMonsterAttackValue;
			}

			// 총알 - 몬스터
			if (mBullet.IsActive() and mIsMonsterBulletColliding[i])
			{
				mMonsters[i].SetActive(false);
				mBullet.SetActive(false);
				mIsMonsterSpwan[i] = true;

				mIsMonsterBulletColliding[i] = false;
			}
		}
	}

	return true;
}

void MainScene::PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
	ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();

	// 라인을 그린다.
	//{
	//	D2D1_ELLIPSE CIRCLE{ .radiusX = 5.0f, .radiusY = 5.0f };

	//	mLine.Point0 = { .x = -200.0f, .y = 200.0f };
	//	mLine.Point1 = { .x = 150.0f, .y = 100.0f };

	//	Matrix3x2F point0WorldView = Transformation::getWorldMatrix(mLine.Point0) * view;
	//	Matrix3x2F point1WorldView = Transformation::getWorldMatrix(mLine.Point1) * view;

	//	// 두 점을 그린다.
	//	{
	//		renderTarget->SetTransform(point0WorldView);
	//		renderTarget->DrawEllipse(CIRCLE, mDefaultBrush);

	//		renderTarget->SetTransform(point1WorldView);
	//		renderTarget->DrawEllipse(CIRCLE, mDefaultBrush);
	//	}

	//	// 라인을 그린다.
	//	{
	//		D2D1_POINT_2F point0 = D2D1_POINT_2F{ .x = 0.0f, .y = 0.0f } *point0WorldView;
	//		D2D1_POINT_2F point1 = D2D1_POINT_2F{ .x = 0.0f, .y = 0.0f } *point1WorldView;
	//		renderTarget->SetTransform(Matrix3x2F::Identity());
	//		renderTarget->DrawLine(point0, point1, mDefaultBrush);
	//	}
	//}

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

			if (monster.IsActive())
			{
				const Matrix3x2F worldView = Transformation::getWorldMatrix({ .x = GetRectangleFromSprite(monster).left, .y = GetRectangleFromSprite(monster).top}) * view;
				renderTarget->SetTransform(worldView);

				ID2D1SolidColorBrush* cyanBrush = nullptr;
				renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cyan), &cyanBrush);

				const D2D1_SIZE_F scale = monster.GetScale();

				const D2D1_RECT_F colliderSize =
				{ 
					.left = 0.0f,
					.top = 0.0f, 
					.right = scale.width * mRectangleTexture.GetWidth(), 
					.bottom = scale.width * mRectangleTexture.GetWidth()
				};

				renderTarget->DrawRectangle(colliderSize, cyanBrush);
				RELEASE_D2D1(cyanBrush);
			}
		}
	}

	// 총알 충돌박스를 그린다.
	{
		if (mBullet.IsActive())
		{
			const Matrix3x2F worldView = Transformation::getWorldMatrix(GetCircleFromSprite(mBullet).point) * view;
			renderTarget->SetTransform(worldView);

			ID2D1SolidColorBrush* yellowBrush = nullptr;
			renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &yellowBrush);

			const D2D1_SIZE_F scale = mBullet.GetScale();

			const D2D1_ELLIPSE circleSize =
			{
				.radiusX = scale.width * mCircleTexture.GetWidth() * 0.5f,
				.radiusY = scale.height * mCircleTexture.GetHeight() * 0.5f
			};

			renderTarget->DrawEllipse(circleSize, yellowBrush);
			RELEASE_D2D1(yellowBrush);
		}
	}
}

void MainScene::Finalize()
{
	RELEASE_D2D1(mDefaultBrush);

	mRectangleTexture.Finalize();
	mRedRectangleTexture.Finalize();

	mCircleTexture.Finalize();
	mRedCircleTexture.Finalize();
}

D2D1_RECT_F MainScene::GetRectangleFromSprite(const Sprite& sprite)
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

D2D1_ELLIPSE MainScene::GetCircleFromSprite(const Sprite& sprite)
{
	const D2D1_RECT_F rect = GetRectangleFromSprite(sprite);
	const D2D1_POINT_2F position = sprite.GetPosition();

	const D2D1_ELLIPSE circle =
	{
		.point = position,
		.radiusX = fabs(rect.right - position.x),
		.radiusY = fabs(rect.top - position.y)
	};

	return circle;
}