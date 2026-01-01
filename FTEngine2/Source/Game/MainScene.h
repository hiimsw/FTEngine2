#pragma once

#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"

class MainScene final : public Scene
{
public:
	MainScene() = default;
	MainScene(const MainScene&) = delete;
	MainScene& operator=(const MainScene&) = delete;

	void Initialize() override;
	bool Update(const float deltaTime) override;
	void Finalize() override;

private:
	Texture mRectangleTexture{};

	Camera mMainCamera{};

	std::vector<Sprite*> mSpriteLayer{};
	Sprite mHero{};
	Sprite mMonster{};
	Sprite mZoom{};
	
	std::array<Sprite, 4> mBars{};
};