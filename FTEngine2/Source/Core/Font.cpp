#include "pch.h"
#include "Font.h"

#include "Constant.h"
#include "Helper.h"

void Font::Initialize(Helper* helper, const std::wstring& filename, const float fontSize)
{
	ASSERT(helper != nullptr);

	mHelper = helper;

	std::wstring filenameByWStr = {};
	filenameByWStr.assign(filename.begin(), filename.end());

	IDWriteFactory* dWriteFactory = mHelper->GetDWriteFactory();
	HR(dWriteFactory->CreateTextFormat(filenameByWStr.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"", &mTextFormat));
}

void Font::Finalize()
{
	RELEASE_D2D1(mTextFormat);
}

D2D1_SIZE_F Font::_CalculateTextSize(const std::wstring& text) const
{
	IDWriteFactory* dWriteFactory = mHelper->GetDWriteFactory();
	IDWriteTextLayout* layout = nullptr;
	HR(dWriteFactory->CreateTextLayout(text.c_str(), UINT32(text.size()), mTextFormat, float(Constant::Get().GetWidth()), float(Constant::Get().GetHeight()), &layout));

	DWRITE_TEXT_METRICS metrics{};
	HR(layout->GetMetrics(&metrics));
	D2D1_SIZE_F size{ .width = metrics.width, .height = metrics.height };

	return size;
}

IDWriteTextFormat* Font::_GetTextFormat() const
{
	return mTextFormat;
}
