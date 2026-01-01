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
	bool Update(const float deltaTime) override;
	void Finalize() override;

private:
	Texture mRectangleTexture{};
	Texture mRedRectangleTexture{};

	Camera mMainCamera{};

	std::array<std::vector<Sprite*>, uint32_t(Layer::Count)> mSpriteLayers{};
	Sprite mHero{};
	Sprite mMonster{};
	Sprite mZoom{};
	
	std::array<Sprite, 4> mBars{};
};