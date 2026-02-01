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

private:
	ID2D1Factory* mFactory = nullptr;
	IWICImagingFactory* mWICImagingFactory = nullptr;
	IDWriteFactory* mDwriteFactory = nullptr;
	ID2D1HwndRenderTarget* mRenderTarget = nullptr;
	FMOD::System* mSoundSystem = nullptr;
	ID2D1SolidColorBrush* mBrush = nullptr;

	Helper mHelper{};
	Scene* mScene = nullptr;
};