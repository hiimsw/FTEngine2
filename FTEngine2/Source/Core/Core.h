#pragma once

#include "Helper.h"
#include "Scene.h"

class Core final
{
public:
	Core() = default;
	Core(const Core&) = delete;
	Core& operator=(const Core&) = delete;

	void Initialize(HWND hWnd, Scene* scene);
	bool Update(const float deltaTime);
	void Finalize();

	void ChangeScene(Scene* scene);
	void SetSceneType(const Scene::SCENE_TYPE type);

private:
	ID2D1Factory* mFactory = nullptr;
	IWICImagingFactory* mWICImagingFactory = nullptr;
	IDWriteFactory* mDwriteFactory = nullptr;
	ID2D1HwndRenderTarget* mRenderTarget = nullptr;

	Helper mHelper{};
	Scene* mScene = nullptr;

	Scene::SCENE_TYPE mSceneType{};
};