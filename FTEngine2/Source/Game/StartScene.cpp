#include "pch.h"
#include "Core/Collision.h"
#include "Core/Constant.h"
#include "Core/Helper.h"
#include "Core/Input.h"
#include "Core/Transformation.h"

#include "StartScene.h"

void StartScene::Initialize()
{
	mSpriteLayers[uint32_t(Layer::Background)].reserve(32);

	SetSpriteLayers(mSpriteLayers.data(), uint32_t(mSpriteLayers.size()));
	SetCamera(&mMainCamera);

	// 시작버튼을 초기화한다. 
	{
		mStartIdleButtonTexture.Initialize(GetHelper(), L"Resource/start_idle_button.png");
		mStartContactButtonTexture.Initialize(GetHelper(), L"Resource/start_contact_button.png");

		mStartButton.SetPosition({ .x = 0.0f, .y = -100.0f });
		mStartButton.SetTexture(&mStartIdleButtonTexture);
		mSpriteLayers[uint32_t(Layer::Background)].push_back(&mStartButton);
	}

	// 종료버튼을 초기화한다. 
	{
		mExitIdleButtonTexture.Initialize(GetHelper(), L"Resource/exit_idle_button.png");
		mExitContactButtonTexture.Initialize(GetHelper(), L"Resource/exit_contact_button.png");

		mExitButton.SetPosition({ .x = 0.0f, .y = -220.0f });
		mExitButton.SetTexture(&mExitIdleButtonTexture);
		mSpriteLayers[uint32_t(Layer::Background)].push_back(&mExitButton);
	}

	{

	}
}

void StartScene::PreDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
}

bool StartScene::Update(const float deltaTime)
{
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		PostQuitMessage(0);
	}

	if (Collision::IsCollidedSqureWithPoint(getRectangleFromSprite(mStartButton, mStartIdleButtonTexture), getMouseWorldPosition()))
	{
		mStartButton.SetTexture(&mStartContactButtonTexture);	

		if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
		{
			mIsUpdate = false;
		}
	}
	else
	{
		mStartButton.SetTexture(&mStartIdleButtonTexture);
	}

	if (Collision::IsCollidedSqureWithPoint(getRectangleFromSprite(mExitButton, mExitIdleButtonTexture), getMouseWorldPosition()))
	{
		mExitButton.SetTexture(&mExitContactButtonTexture);

		if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
		{
			PostQuitMessage(0);
		}
	}
	else
	{
		mExitButton.SetTexture(&mExitIdleButtonTexture);
	}

	return mIsUpdate;
}

void StartScene::PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
}

void StartScene::Finalize()
{
	mStartIdleButtonTexture.Finalize();
	mStartContactButtonTexture.Finalize();

	mExitIdleButtonTexture.Finalize();
	mExitContactButtonTexture.Finalize();
}

D2D1_POINT_2F StartScene::getMouseWorldPosition() const
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

	return screenPosition;
}

D2D1_RECT_F StartScene::getRectangleFromSprite(const Sprite& sprite, const Texture texture)
{
	const D2D1_SIZE_F scale = sprite.GetScale();

	const D2D1_SIZE_F offset =
	{
		.width = scale.width * texture.GetWidth() * 0.5f,
		.height = scale.height * texture.GetHeight() * 0.5f
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