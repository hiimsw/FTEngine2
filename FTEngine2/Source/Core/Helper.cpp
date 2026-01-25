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

FMOD::System* Helper::GetSoundSystem() const
{
	return mSoundSystem;
}

void Helper::_Initialize(IWICImagingFactory* wicImagingFactory, IDWriteFactory* dWriteFactory, ID2D1HwndRenderTarget* renderTarget, FMOD::System* soundSystem)
{
	ASSERT(wicImagingFactory != nullptr 
		and dWriteFactory != nullptr
		and renderTarget != nullptr
		and soundSystem != nullptr);

	mWICImagingFactory = wicImagingFactory;
	mDWriteFactory = dWriteFactory;
	mRenderTarget = renderTarget;
	mSoundSystem = soundSystem;
}
