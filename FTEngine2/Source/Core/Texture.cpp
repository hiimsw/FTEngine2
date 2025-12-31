#include "pch.h"
#include "Texture.h"

#include "Helper.h"

Texture::Texture(const Texture& other)
{
	memcpy(this, &other, sizeof(Texture));
}

void Texture::Initialize(Helper* helper, const std::wstring& filename)
{
	ASSERT(helper != nullptr);

	ID2D1HwndRenderTarget* renderTarget = helper->GetRenderTarget();
	IWICImagingFactory* wicImagingFactory = helper->GetWICImagingFactory();

	IWICBitmapDecoder* decoder = nullptr;
	HR(wicImagingFactory->CreateDecoderFromFilename(filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder));

	IWICBitmapFrameDecode* frame = nullptr;
	HR(decoder->GetFrame(0, &frame));

	IWICFormatConverter* converter = nullptr;
	HR(wicImagingFactory->CreateFormatConverter(&converter));
	HR(converter->Initialize(frame, GUID_WICPixelFormat32bppPRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom));

	HR(renderTarget->CreateBitmapFromWicBitmap(converter, nullptr, &mBitmap));

	RELEASE_D2D1(converter);
	RELEASE_D2D1(frame);
	RELEASE_D2D1(decoder);
}

void Texture::Finalize()
{
	RELEASE_D2D1(mBitmap);
}

uint32_t Texture::GetWidth() const
{
	uint32_t width = mBitmap->GetPixelSize().width;
	return width;
}

uint32_t Texture::GetHeight() const
{
	uint32_t height = mBitmap->GetPixelSize().height;
	return height;
}

ID2D1Bitmap* Texture::_GetBitmap() const
{
	return mBitmap;
}
