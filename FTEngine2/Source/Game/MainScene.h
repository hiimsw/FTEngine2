#pragma once

#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"

class MainScene final : public Scene
{
	enum class Layer
	{
		Background,
		Monster,
		Player,
		UI,
		Count
	};

public:
	MainScene() = default;
	MainScene(const MainScene&) = delete;
	MainScene& operator=(const MainScene&) = delete;

	void Initialize() override;
	void PreDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI) override;
	bool Update(const float deltaTime) override;
	void PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI) override;
	void Finalize() override;

private:


private:
	Texture mRectangleTexture{};
	Texture mRedRectangleTexture{};

	Texture mCircleTexture{};
	Texture mRedCircleTexture{};

	Camera mMainCamera{};

	std::array<std::vector<Sprite*>, uint32_t(Layer::Count)> mSpriteLayers{};
	Sprite mHero{};
	Sprite mMonster{};
	Sprite mZoom{};

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;
	bool mIsCursorConfined = false;
};