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

		Input::Get().SetCursorVisible(false);
		Input::Get().SetCursorLockState(Input::eCursorLockState::Confined);

		ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();
		HR(renderTarget->CreateSolidColorBrush(ColorF(1.0f, 1.0f, 1.0f), &mDefaultBrush));

		mIsCursorConfined = (Input::Get().GetCursorLockState() == Input::eCursorLockState::Confined);
	}

	mRectangleTexture.Initialize(GetHelper(), L"Resource/Rectangle.png");
	mRedRectangleTexture.Initialize(GetHelper(), L"Resource/RedRectangle.png");

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

	// 몬스터를 초기화한다.
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int32_t> dist(20, 999);

		static uint32_t cnt;
		static D2D1_POINT_2F spawnDirection[MONSTER_COUNT]{};
		static D2D1_POINT_2F spawnPositionInRect[MONSTER_COUNT]{};

		// 랜덤 좌표를 생성한다.
		while (cnt != MONSTER_COUNT)
		{
			D2D1_SIZE_F UNIT = 
			{ 
				.width = mRectangleTexture.GetWidth() * 0.5f * mMonsters[0].GetScale().width, 
				.height = mRectangleTexture.GetHeight() * 0.5f * mMonsters[0].GetScale().height
			};

			
			mMonsterSpawnPositions[cnt] =
			{
				.x = float(dist(gen)),
				.y = float(dist(gen))
			};

			if (rand() % 2 == 0)
			{
				mMonsterSpawnPositions[cnt].x *= -1.0f;
			}

			if (rand() % 2 == 0)
			{
				mMonsterSpawnPositions[cnt].y *= -1.0f;
			}

			spawnDirection[cnt] = Math::GetNormalizeVector(mMonsterSpawnPositions[cnt]);

			constexpr float OUTLINE_OFFSET = 350.0f;
			spawnPositionInRect[cnt] = Math::ScaleVector(spawnDirection[cnt], OUTLINE_OFFSET);

			cnt++;
		}

		constexpr float MONSTER_SCALE = 0.5f;

		for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		{
			mMonsters[i].SetTexture(&mRectangleTexture);
			mMonsters[i].SetPosition(spawnPositionInRect[i]);
			mMonsters[i].SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });
			mMonsters[i].SetActive(false);

			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&mMonsters[i]);
		}
	}

	// 줌을 초기화한다.
	{
		mZoom.SetAngle(45.0f);
		mZoom.SetUI(true);
		mZoom.SetTexture(&mRedRectangleTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mZoom);
	}

	// 카메라를 초기화한다.
	{
		D2D1_POINT_2F heroPosition = mHero.GetPosition();
		D2D1_POINT_2F cameraPosition{ .x = heroPosition.x * 0.5f, .y = heroPosition.y * 0.5f };

		mMainCamera.SetPosition(cameraPosition);
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
		Matrix3x2F worldView = Transformation::getWorldMatrix() * view;
		renderTarget->SetTransform(worldView);

		D2D1_ELLIPSE ellipse{ .radiusX = 400.0f, .radiusY = 400.0f };
		renderTarget->DrawEllipse(ellipse, mDefaultBrush, 2.0f);
	}
}

bool MainScene::Update(const float deltaTime)
{
	// 게임을 종료한다.
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		return false;
	}

	if (Input::Get().GetKeyDown(VK_CONTROL))
	{
		mIsCursorConfined = !mIsCursorConfined;
		Input::Get().SetCursorLockState(mIsCursorConfined ? Input::eCursorLockState::Confined : Input::eCursorLockState::None);
		Input::Get().SetCursorVisible(not mIsCursorConfined);
	}

	// 플레이어를 업데이트한다.
	{
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
				D2D1_POINT_2F direction = Math::GetNormalizeVector(velocity);
				D2D1_POINT_2F adjustVelocity = Math::ScaleVector(direction, speed);
				adjustVelocity = Math::ScaleVector(adjustVelocity, deltaTime);

				D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), adjustVelocity);
				mHero.SetPosition(position);
			}
		}
	}

	// 줌을 업데이트한다.
	{
		D2D1_POINT_2F centerOffset =
		{
			.x = (Constant::Get().GetWidth() - 1.0f) * 0.5f,
			.y = (Constant::Get().GetHeight() - 1.0f) * 0.5f
		};

		D2D1_POINT_2F mousePos = Input::Get().GetMousePosition();
		mZoom.SetPosition(Math::SubtractVector(mousePos, centerOffset));
	}

	// 카메라를 업데이트한다.
	{
		D2D1_POINT_2F position = mMainCamera.GetPosition();
		position = mHero.GetPosition();

		//std::lerp()

		mMainCamera.SetPosition(position);
	}

	// 몬스터를 업데이트한다.
	{
		static float spawnTimer;

		spawnTimer += deltaTime;

		static uint32_t spawnIndex;
		if (spawnTimer >= 0.5f and spawnIndex < MONSTER_COUNT)
		{
			if (spawnTimer >= 0.1f)
			{
				mMonsters[spawnIndex].SetActive(true);

				spawnIndex++;
				spawnTimer = 0.0f;
			}
		}
	}

	// 충돌	처리를 업데이트한다.
	{
		D2D1_POINT_2F zoomPos = mZoom.GetPosition();

		if (Collision::IsCollidedSqureWithPoint(GetRectagnleFromSprite(mHero), zoomPos))
		{
			mHero.SetTexture(&mRedRectangleTexture);
		}
		else
		{
			mHero.SetTexture(&mRectangleTexture);
		}

		//if (Collision::IsCollidedCircleWithPoint(heroPos, float(rect.right - heroPos.x), zoomPos))
		//{
		//	mHero.SetTexture(&mRedCircleTexture);
		//	DEBUG_LOG("ㅇㅇ");
		//}
		//else
		//{
		//	mHero.SetTexture(&mCircleTexture);
		//	DEBUG_LOG("ㄴㄴ");
		//}

		if (Collision::IsCollidedSqureWithSqure(GetRectagnleFromSprite(mHero), GetRectagnleFromSprite(mZoom)))
		{
			DEBUG_LOG("dd");
		}
		else
		{
			DEBUG_LOG("ss");
		}

		Line line =
		{
			.Point0 = mLine.Point0,
			.Point1 = mLine.Point1
		};

		if (Collision::IsCollidedSqureWithLine(GetRectagnleFromSprite(mHero), line))
		{
			DEBUG_LOG("dd");
		}
		else
		{
			DEBUG_LOG("ss");
		}
	}

	return true;
}

void MainScene::PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
	ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();

	// 라인을 그린다.
	{
		D2D1_ELLIPSE CIRCLE{ .radiusX = 5.0f, .radiusY = 5.0f };

		mLine.Point0 = { .x = -200.0f, .y = 200.0f };
		mLine.Point1 = { .x = 150.0f, .y = 100.0f };

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
			D2D1_POINT_2F point0 = D2D1_POINT_2F{ .x = 0.0f, .y = 0.0f } *point0WorldView;
			D2D1_POINT_2F point1 = D2D1_POINT_2F{ .x = 0.0f, .y = 0.0f } *point1WorldView;
			renderTarget->SetTransform(Matrix3x2F::Identity());
			renderTarget->DrawLine(point0, point1, mDefaultBrush);
		}
	}

	// 몬스터를 그린다.
	{
		//D2D1_POINT_2F spawnDirection[MONSTER_COUNT]{};
		//D2D1_POINT_2F spawnPositionInCircle[MONSTER_COUNT]{};

		//for (uint32_t i = 0; i < MONSTER_COUNT; ++i)
		//{
		//	spawnDirection[i] = Math::GetNormalizeVector(mMonsterSpawnPositions[i]);
		//	spawnPositionInCircle[i] = Math::ScaleVector(spawnDirection[i], 350.0f);

		//	Matrix3x2F worldView = Transformation::getWorldMatrix(spawnPositionInCircle[i]) * view;
		//	renderTarget->SetTransform(worldView);

		//	D2D1_ELLIPSE CIRCLE{ .radiusX = 30.0f, .radiusY = 30.0f };
		//	renderTarget->DrawEllipse(CIRCLE, mDefaultBrush);
		//}
	}
}

void MainScene::Finalize()
{
	RELEASE_D2D1(mDefaultBrush);

	mRectangleTexture.Finalize();
	mRedRectangleTexture.Finalize();
}

D2D1_RECT_F MainScene::GetRectagnleFromSprite(const Sprite& sprite)
{		
	D2D1_POINT_2F position = sprite.GetPosition();
	D2D1_SIZE_F scale = sprite.GetScale();

	D2D1_SIZE_F offset =
	{
		.width = scale.width * mRectangleTexture.GetWidth() * 0.5f,
		.height = scale.height * mRectangleTexture.GetHeight() * 0.5f
	};

	D2D1_RECT_F rect =
	{
		.left = position.x - offset.width,
		.top = position.y + offset.height,
		.right = position.x + offset.width,
		.bottom = position.y - offset.height
	};

	return rect;
}
