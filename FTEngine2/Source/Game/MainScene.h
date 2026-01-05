#pragma once

#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/Sprite.h"
#include "Core/Texture.h"

struct GizmoLine
{
	D2D1_POINT_2F Point0;
	D2D1_POINT_2F Point1;
};

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
	Texture mRectangleTexture{};
	Texture mRedRectangleTexture{};

	Texture mCircleTexture{};
	Texture mRedCircleTexture{};

	Camera mMainCamera{};

	std::array<std::vector<Sprite*>, uint32_t(Layer::Count)> mSpriteLayers{};
	Sprite mHero{};
	constexpr static uint32_t MONSTER_COUNT = 20;
	Sprite mMonsters[MONSTER_COUNT]{};
	Sprite mZoom{};
	GizmoLine mLine{};

	ID2D1SolidColorBrush* mDefaultBrush = nullptr;
	bool mIsCursorConfined = false;

	D2D1_POINT_2F mMonsterSpawnPositions[MONSTER_COUNT]{};
};