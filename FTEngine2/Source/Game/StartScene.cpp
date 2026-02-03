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

	srand(unsigned int(time(nullptr)));

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

	// 배경을 초기화한다.
	{
		mRedStarTexture.Initialize(GetHelper(), L"Resource/RedStar.png");

		const D2D1_SIZE_F screenScale = { .width = float(Constant::Get().GetWidth()) * 0.5f, .height = float(Constant::Get().GetHeight()) * 0.5f };
		constexpr float TITLE_RECT_OFFSET = 50.0f;

		for (Star& red : mRedStars)
		{
			red.isVisible = false;
			red.speed = getRandom(0.09f, 1.0f);

			const D2D1_POINT_2F screenPoint =
			{
				.x = getRandom(-screenScale.width + TITLE_RECT_OFFSET, screenScale.width - TITLE_RECT_OFFSET),
				.y = getRandom(-screenScale.height + TITLE_RECT_OFFSET, screenScale.height - TITLE_RECT_OFFSET)
			};

			Sprite& sprite = red.sprite;
			sprite.SetPosition(screenPoint);
			sprite.SetOpacity(0.0f);
			sprite.SetTexture(&mRedStarTexture);
			mSpriteLayers[uint32_t(Layer::Background)].push_back(&sprite);
		}
	}
}

void StartScene::PreDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
}

bool StartScene::Update(const float deltaTime)
{
	// 키를 업데이트한다.
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		PostQuitMessage(0);
	}

	// 배경 투명도를 업데이트한다.
	{
		float speed[RED_COUNT];
		for (Star& red : mRedStars)
		{
			float opacity = red.sprite.GetOpacity();

			if (red.isVisible)
			{
				opacity -= red.speed * deltaTime;

				if (opacity <= 0.0f)
				{
					opacity = 0.0f;
					red.isVisible = false;
				}
			}
			else
			{
				opacity += red.speed * deltaTime;

				if (opacity >= 1.0f)
				{
					opacity = 1.0f;
					red.isVisible = true;
				}
			}

			red.sprite.SetOpacity(opacity);
		}
	}

	// 충돌을 업데이트한다.
	{
		// Start Button
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

		// Exit Button
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

	mRedStarTexture.Finalize();
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

float StartScene::getRandom(const float min, const float max)
{
	const float result = float(rand()) / RAND_MAX * (max - min) + min;
	return result;
}
