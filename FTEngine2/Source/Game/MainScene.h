#pragma once

#include "Core/Scene.h"

class MainScene final : public Scene
{
public:
	MainScene() = default;
	MainScene(const MainScene&) = delete;
	MainScene& operator=(const MainScene&) = delete;

	void Initialize() override;
	bool Update(const float deltaTime) override;
	void Finalize() override;
};