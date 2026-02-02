#pragma once
#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"

class StartScene final : public Scene
{
public:
	StartScene() = default;
	StartScene(const StartScene&) = delete;
	StartScene& operator=(const StartScene&) = delete;

	void Initialize() override;
	void PreDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI) override;
	bool Update(const float deltaTime) override;
	void PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI) override;
	void Finalize() override;

private:
	bool mIsUpdate = true;

	std::array<std::vector<Sprite*>, uint32_t(Layer::End)> mSpriteLayers{};
	Camera mMainCamera{};

	Sprite mStartButton{};
	Texture mStartButtonTexture{};
};