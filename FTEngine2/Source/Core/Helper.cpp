#include "pch.h"
#include "Helper.h"

IWICImagingFactory* Helper::GetWICImagingFactory() const
{
	return mWICImagingFactory;
}

IDWriteFactory* Helper::GetDWriteFactory() const
{
	return mDWriteFactory;
}

ID2D1HwndRenderTarget* Helper::GetRenderTarget() const
{
	return mRenderTarget;
}

void Helper::_Initialize(IWICImagingFactory* wicImagingFactory, IDWriteFactory* dWriteFactory, ID2D1HwndRenderTarget* renderTarget)
{
	ASSERT(wicImagingFactory != nullptr 
		and dWriteFactory != nullptr
		and renderTarget != nullptr);

	mWICImagingFactory = wicImagingFactory;
	mDWriteFactory = dWriteFactory;
	mRenderTarget = renderTarget;
}
