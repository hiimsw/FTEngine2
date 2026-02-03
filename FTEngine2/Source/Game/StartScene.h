#pragma once
#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"

enum class eColor_Type
{
	Red,
	Orange,
	Yellow,
	Green,
	Blue,
	Purple
};

struct Star
{
	Sprite sprite;
	eColor_Type type;

	bool isVisible;
	float speed;
};

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
	D2D1_POINT_2F getMouseWorldPosition() const;
	D2D1_RECT_F getRectangleFromSprite(const Sprite& sprite, const Texture texture);
	float getRandom(const float min, const float max);
	void updateFadeEffect(Star* star, const float deltaTime);

private:
	bool mIsUpdate = true;
	static constexpr float TITLE_RECT_OFFSET = 50.0f;

	std::array<std::vector<Sprite*>, uint32_t(Layer::End)> mSpriteLayers{};
	Camera mMainCamera{};

	Sprite mStartButton{};
	Texture mStartIdleButtonTexture{};
	Texture mStartContactButtonTexture{};

	Sprite mExitButton{};
	Texture mExitIdleButtonTexture{};
	Texture mExitContactButtonTexture{};

	static constexpr uint32_t COUNT = 3;
	Star mRedStars[COUNT]{};
	Texture mRedStarTexture{};

	Star mOrangeStars[COUNT]{};
	Texture mOrangeStarTexture{};

	Star mYellowStars[COUNT]{};
	Texture mYellowStarTexture{};

	Star mGreenStars[COUNT]{};
	Texture mGreenStarTexture{};

	Star mBlueStars[COUNT]{};
	Texture mBlueStarTexture{};

	Star mPurpleStars[COUNT]{};
	Texture mPurpleStarTexture{};
};