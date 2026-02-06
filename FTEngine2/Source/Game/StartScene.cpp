#include "pch.h"
#include "Core/Collision.h"
#include "Core/Constant.h"
#include "Core/Helper.h"
#include "Core/Input.h"
#include "Core/Transformation.h"

#include "StartScene.h"

void StartScene::Initialize()
{
	// 기본 데이터를 초기화한다.
	
	mSpriteLayers[uint32_t(Layer::Background)].reserve(32);

	SetSpriteLayers(mSpriteLayers.data(), uint32_t(mSpriteLayers.size()));
	SetCamera(&mMainCamera);

	srand(unsigned int(time(nullptr)));

	mBackgroundSound.Initialize(GetHelper(), "Resource/Sound/DST-RailJet-LongSeamlessLoop.mp3", true);
	mBackgroundSound.SetVolume(0.3f);
	mBackgroundSound.Play();

	mButtonSound.Initialize(GetHelper(), "Resource/Sound/button_sound.wav", false);
	mButtonSound.SetVolume(0.2f);

	// Star를 초기화한다.
	{
		mRedStarTexture.Initialize(GetHelper(), L"Resource/RedStar.png");
		mOrangeStarTexture.Initialize(GetHelper(), L"Resource/OrangeStar.png");
		mYellowStarTexture.Initialize(GetHelper(), L"Resource/YellowStar.png");
		mGreenStarTexture.Initialize(GetHelper(), L"Resource/GreenStar.png");
		mBlueStarTexture.Initialize(GetHelper(), L"Resource/BlueStar.png");
		mPurpleStarTexture.Initialize(GetHelper(), L"Resource/PurpleStar.png");

		Texture* textures[6] =
		{
			&mRedStarTexture,
			&mOrangeStarTexture,
			&mYellowStarTexture,
			&mGreenStarTexture,
			&mBlueStarTexture,
			&mPurpleStarTexture
		};

		// 파일 읽기
		std::wifstream file(L"Resource/StarPosition.txt");
		ASSERT(file);

		D2D1_POINT_2F position{};
		uint32_t starCount{};

		while (file >> position.x >> position.y)
		{
			if (starCount >= STAR_COUNT)
			{
				break;
			}

			Star& star = mStars[starCount];
			star.isOpacityVisible = false;
			star.speed = getRandom(0.09f, 1.0f);

			Sprite& sprite = star.sprite;
			sprite.SetPosition(position);
			sprite.SetOpacity(0.0f);
			sprite.SetTexture(textures[starCount % 6]);
			mSpriteLayers[uint32_t(Layer::Background)].push_back(&sprite);

			++starCount;
		}
	}

	// 타이틀을 초기화한다.
	{
		mTitleTexture.Initialize(GetHelper(), L"Resource/Title.png");

		mTitle.SetPosition({ .x = 0.0f, .y = 100.0f });
		mTitle.SetScale({ .width = 2.0f, .height = 2.0f });
		mTitle.SetTexture(&mTitleTexture);
		mSpriteLayers[uint32_t(Layer::Background)].push_back(&mTitle);
	}

	// 시작버튼을 초기화한다. 
	{
		mStartIdleButtonTexture.Initialize(GetHelper(), L"Resource/start_idle_button.png");
		mStartContactButtonTexture.Initialize(GetHelper(), L"Resource/start_contact_button.png");

		mStartButton.SetPosition({ .x = 0.0f, .y = -100.0f });
		mStartButton.SetScale({ .width = 0.7f, .height = 0.7f });
		mStartButton.SetTexture(&mStartIdleButtonTexture);
		mSpriteLayers[uint32_t(Layer::Background)].push_back(&mStartButton);
	}

	// 종료버튼을 초기화한다. 
	{
		mExitIdleButtonTexture.Initialize(GetHelper(), L"Resource/exit_idle_button.png");
		mExitContactButtonTexture.Initialize(GetHelper(), L"Resource/exit_contact_button.png");

		mExitButton.SetPosition({ .x = 0.0f, .y = -220.0f });
		mExitButton.SetScale({ .width = 0.7f, .height = 0.7f });
		mExitButton.SetTexture(&mExitIdleButtonTexture);
		mSpriteLayers[uint32_t(Layer::Background)].push_back(&mExitButton);
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

	// 투명도를 업데이트한다.
	{
		for (Star& star : mStars)
		{
			float opacity = star.sprite.GetOpacity();

			if (star.isOpacityVisible)
			{
				opacity -= star.speed * deltaTime;

				if (opacity <= 0.0f)
				{
					opacity = 0.0f;
					star.isOpacityVisible = false;
				}
			}
			else
			{
				opacity += star.speed * deltaTime;

				if (opacity >= 1.0f)
				{
					opacity = 1.0f;
					star.isOpacityVisible = true;
				}
			}

			star.sprite.SetOpacity(opacity);
		}
	}

	// 버튼을 업데이트한다.
	{
		updateButtonState
		(
			{
				.sprite = &mStartButton,
				.isColliding = mIsStartButtonColliding,
				.originalTexture = &mStartIdleButtonTexture,
				.effectTexture = &mStartContactButtonTexture,
				.isSoundPlay = &mIsStartButtonSoundPlay
			}
		);

		updateButtonState
		(
			{
				.sprite = &mExitButton,
				.isColliding = mIsExitButtonColliding,
				.originalTexture = &mExitIdleButtonTexture,
				.effectTexture = &mExitContactButtonTexture,
				.isSoundPlay = &mIsExitButtonSoundPlay
			}
		);

		if (mIsStartButtonColliding)
		{
			if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
			{
				mIsUpdate = false;
			}
		}

		if (mIsExitButtonColliding)
		{
			if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
			{
				PostQuitMessage(0);
			}
		}
	}

	// 충돌을 업데이트한다.
	{
		// Start Button
		if (Collision::IsCollidedSqureWithPoint(getRectangleFromSprite(mStartButton, mStartIdleButtonTexture), getMouseWorldPosition()))
		{
			mIsStartButtonColliding = true;
		}
		else
		{
			mIsStartButtonColliding = false;
		}

		// Exit Button
		if (Collision::IsCollidedSqureWithPoint(getRectangleFromSprite(mExitButton, mExitIdleButtonTexture), getMouseWorldPosition()))
		{
			mIsExitButtonColliding = true;
		}
		else
		{
			mIsExitButtonColliding = false;
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
	mOrangeStarTexture.Finalize();
	mYellowStarTexture.Finalize();
	mGreenStarTexture.Finalize();
	mBlueStarTexture.Finalize();
	mPurpleStarTexture.Finalize();

	mBackgroundSound.Finalize();
	mButtonSound.Finalize();
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
	const float	result = float(rand()) / RAND_MAX * (max - min) + min;
	return result;
}

void StartScene::initializeStar(const StarDesc& desc)
{
}

void StartScene::updateFadeEffect(Star* star, const float deltaTime)
{
	ASSERT(star != nullptr);
	
	Sprite& sprite = star->sprite;
	bool& isVisible = star->isOpacityVisible;
	const float speed = star->speed;

}

void StartScene::updateButtonState(const StartSceneButtonDesc& desc)
{
	Sprite* sprite = desc.sprite;
	const bool isColliding = desc.isColliding;
	Texture* originalTexture = desc.originalTexture;
	Texture* effectTexture = desc.effectTexture;
	bool* isSoundPlay = desc.isSoundPlay;

	if (isColliding)
	{
		sprite->SetTexture(effectTexture);

		if (not *isSoundPlay)
		{
			mButtonSound.Replay();
			*isSoundPlay = true;
		}
	}
	else
	{
		*isSoundPlay = false;
		sprite->SetTexture(originalTexture);
	}
}
