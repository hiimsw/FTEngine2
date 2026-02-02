#include "pch.h"
#include "Core/Constant.h"
#include "Core/Helper.h"
#include "Core/Input.h"
#include "Core/Transformation.h"

#include "StartScene.h"

void StartScene::Initialize()
{
	for (std::vector<Sprite*>& layer : mSpriteLayers)
	{
		mSpriteLayers[uint32_t(Layer::Background)].reserve(32);
	}

	SetSpriteLayers(mSpriteLayers.data(), uint32_t(mSpriteLayers.size()));
	SetCamera(&mMainCamera);

	mStartButtonTexture.Initialize(GetHelper(), L"Resource/start_idle_button.png");

	mStartButton.SetPosition({ .x = 0.0f, .y = -150.0f });
	mStartButton.SetTexture(&mStartButtonTexture);
	mSpriteLayers[uint32_t(Layer::Player)].push_back(&mStartButton);
}

void StartScene::PreDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
}

bool StartScene::Update(const float deltaTime)
{
	if (Input::Get().GetKeyDown('Q'))
	{
		mIsUpdate = false;
	}

	return mIsUpdate;
}

void StartScene::PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
}

void StartScene::Finalize()
{
	mStartButtonTexture.Finalize();
}
