#include "pch.h"
#include "MainScene.h"

#include "Core/Collision.h"
#include "Core/Constant.h"
#include "Core/Helper.h"
#include "Core/Input.h"
#include "Core/Transformation.h"

using namespace D2D1;

constexpr float SCALE = 0.5f;

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
		mHero.SetScale({ .width = 5.0f, .height = 5.0f });
		mHero.SetTexture(&mRectangleTexture);
		mSpriteLayers[uint32_t(Layer::Player)].push_back(&mHero);

		// 동그라미
		//mHero.SetScale({ .width = 5.0f, .height = 5.0f });
		//mHero.SetTexture(&mCircleTexture);
		//mSpriteLayers[uint32_t(Layer::Player)].push_back(&mHero);
	}

	// 몬스터를 초기화한다.
	{
		mMonster.SetTexture(&mRectangleTexture);

		constexpr D2D1_POINT_2F POSITION{ .x = 100.0f, .y = 0.0f };
		mMonster.SetPosition(POSITION);

		constexpr float MONSTER_SCALE = 0.5f;
		mMonster.SetScale({ .width = MONSTER_SCALE, .height = MONSTER_SCALE });

		mSpriteLayers[uint32_t(Layer::Monster)].push_back(&mMonster);
	}

	// 줌을 초기화한다.
	{
		mZoom.SetAngle(0.0f);
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
		// 누르면 1, 안 누르면 0
		int32_t moveX = Input::Get().GetKey('D') - Input::Get().GetKey('A');
		int32_t moveY = Input::Get().GetKey('W') - Input::Get().GetKey('S');

		if (moveX != 0 or moveY != 0)
		{
			float spped = 180.0f * deltaTime;
			D2D1_POINT_2F direction = Math::GetNormalizeVector({ .x = float(moveX), .y = float(moveY) });
			D2D1_POINT_2F velocity = Math::ScaleVector(direction, spped);

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
		mZoom.SetPosition(Math::SubtractVector(mousePos, centerOffset));
	}

	// 카메라를 업데이트한다.
	{
		D2D1_POINT_2F position = mMainCamera.GetPosition();
		position = mHero.GetPosition();

		//std::lerp()

		mMainCamera.SetPosition(position);
	}

	// 충돌	처리를 업데이트한다.
	{
		//D2D1_POINT_2F heroPos = mHero.GetPosition();
		//D2D1_SIZE_F heroScale = mHero.GetScale();
		//D2D1_SIZE_F offset =
		//{
		//	.width = heroScale.width * mRectangleTexture.GetWidth() * 0.5f,
		//	.height = heroScale.height * mRectangleTexture.GetHeight() * 0.5f
		//};

		//RECT rect =
		//{
		//	.left = LONG(heroPos.x - offset.width),
		//	.top = LONG(heroPos.y + offset.height),
		//	.right = LONG(heroPos.x + offset.width),
		//	.bottom = LONG(heroPos.y - offset.height)
		//};

		//D2D1_POINT_2F zoomPos = mZoom.GetPosition();

		//if (Collision::IsCollidedSqureWithPoint(rect, zoomPos))
		//{
		//	mHero.SetTexture(&mRedRectangleTexture);
		//}
		//else
		//{
		//	mHero.SetTexture(&mRectangleTexture);
		//}

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

		D2D1_POINT_2F heroPos = mHero.GetPosition();
		D2D1_SIZE_F heroScale = mHero.GetScale();
		D2D1_SIZE_F half = { .width = mRectangleTexture.GetWidth() * 0.5f, .height = mRectangleTexture.GetHeight() * 0.5f };

		D2D1_RECT_F heroRect =
		{
			.left = heroPos.x - heroScale.width * half.width,
			.top = heroPos.y + heroScale.height * half.height,
			.right = heroPos.x + heroScale.width * half.width,
			.bottom = heroPos.y - heroScale.height * half.height
		};

		D2D1_POINT_2F zoomPos = mZoom.GetPosition();
		D2D1_SIZE_F zoomScale = mZoom.GetScale();

		D2D1_RECT_F zoomRect =
		{
			.left = zoomPos.x - zoomScale.width * half.width,
			.top = zoomPos.y + zoomScale.height * half.height,
			.right = zoomPos.x + zoomScale.width * half.width,
			.bottom = zoomPos.y - zoomScale.height * half.height
		};

		if (Collision::IsCollidedSqureWithSqure(heroRect, zoomRect))
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
}

void MainScene::Finalize()
{
	RELEASE_D2D1(mDefaultBrush);

	mRectangleTexture.Finalize();
	mRedRectangleTexture.Finalize();
}