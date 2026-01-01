#include "pch.h"
#include "MainScene.h"

#include "Core/Constant.h"
#include "Core/Input.h"

constexpr float SCALE = 0.5f;

void MainScene::Initialize()
{
	{
		mSprites.reserve(128);
		SetSprites(&mSprites);

		SetCamera(&mMainCamera);
		
		Input::Get().SetCursorVisible(false);
		Input::Get().SetCursorLockState(Input::eCursorLockState::Confined);
	}	

	mRectangleTexture.Initialize(GetHelper(), L"Resource/Rectangle.png");

	mHero.SetTexture(&mRectangleTexture);
	mSprites.push_back(&mHero);

	// 몬스터를 초기화한다.
	{
		mMonster.SetTexture(&mRectangleTexture);

		constexpr D2D1_POINT_2F POSITION{ .x = 100.0f, .y = 0.0f };
		mMonster.SetPosition(POSITION);

		constexpr float MONSTER_SCALE = 0.5f;
		mMonster.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });

		mSprites.push_back(&mMonster);
	}

	// 줌을 초기화 한다.
	mZoom.SetAngle(45.0f);
	mZoom.SetUI(true);
	mZoom.SetTexture(&mRectangleTexture);
	mSprites.push_back(&mZoom);

	// 카메라를 초기화한다.
	{
		D2D1_POINT_2F heroPosition = mHero.GetPosition();
		D2D1_POINT_2F cameraPosition{ .x = heroPosition.x * 0.5f, .y = heroPosition.y * 0.5f };

		mMainCamera.SetPosition(cameraPosition);
	}

	// TODO(이수원): 디버깅 용도로 사용되며, 추후 삭제 예정이다.
	{
		for (Sprite* sprite : mSprites)
		{
			D2D1_SIZE_F scale = sprite->GetScale();
			scale.width *= SCALE;
			scale.height *= SCALE;
			sprite->SetScale(scale);
		}
	}
}

bool MainScene::Update(const float deltaTime)
{
	// 게임을 종료한다.
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		return false;
	}

	// 플레이어를 업데이트한다.
	{
		// 누르면 1, 안 누르면 0
		int32_t moveX = Input::Get().GetKey('D') - Input::Get().GetKey('A');
		int32_t moveY = Input::Get().GetKey('W') - Input::Get().GetKey('S');

		if (moveX != 0 or moveY != 0)
		{
			float spped = 180.0f * deltaTime;
			D2D1_POINT_2F direction = Math::GetNormalizeVector({ .x = float(moveX), .y = float(moveY) });
			D2D1_POINT_2F velocity{ direction.x * spped, direction.y * spped };

			D2D1_POINT_2F position = Math::AddVector(mHero.GetPosition(), velocity);
			mHero.SetPosition(position);
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
		mZoom.SetPosition(Math::MinusVector(mousePos, centerOffset));
		
		DEBUG_LOG("%f, %f", mousePos.x, mousePos.y);
	}

	// 카메라를 업데이트한다.
	{
		D2D1_POINT_2F position = mMainCamera.GetPosition();
		position = mHero.GetPosition();

		//std::lerp()

		mMainCamera.SetPosition(position);
	}

	return true;
}

void MainScene::Finalize()
{
	mRectangleTexture.Finalize();
}
