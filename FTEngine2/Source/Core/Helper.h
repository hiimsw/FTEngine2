#pragma once

struct IWICImagingFactory;
struct IDWriteFactory;
struct ID2D1HwndRenderTarget;

class Helper final
{
public:
	Helper() = default;
	Helper(const Helper&) = delete;
	Helper& operator=(const Helper&) = delete;

	[[nodiscard]] IWICImagingFactory* GetWICImagingFactory() const;
	[[nodiscard]] IDWriteFactory* GetDWriteFactory() const;
	[[nodiscard]] ID2D1HwndRenderTarget* GetRenderTarget() const;

public:
	void _Initialize(IWICImagingFactory* wicImagingFactory, IDWriteFactory* dWriteFactory, ID2D1HwndRenderTarget* renderTarget);

private:
	IWICImagingFactory* mWICImagingFactory = nullptr;
	IDWriteFactory* mDWriteFactory = nullptr;
	ID2D1HwndRenderTarget* mRenderTarget = nullptr;
};