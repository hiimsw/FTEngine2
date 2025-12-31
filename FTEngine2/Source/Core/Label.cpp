#include "pch.h"
#include "Label.h"

#include "Font.h"

const Font* Label::GetFontOrNull() const
{
	return mFont;
}

void Label::SetFont(Font* font)
{
	ASSERT(font != nullptr);

	mFont = font;
	mTextSize = mFont->_CalculateTextSize(mText);
}

const std::wstring& Label::GetText() const
{
	return mText;
}

const std::wstring& Label::GetTextByWStr() const
{
	return mText;
}

void Label::SetText(const std::wstring& text)
{
	mText = text;

	if (mFont != nullptr)
	{
		mTextSize = mFont->_CalculateTextSize(mText);
	}
}

const D2D1_SIZE_F Label::GetTextSize() const
{
	return mTextSize;
}

bool Label::IsActive() const
{
	return mbActive;
}

void Label::SetActive(const bool bActive)
{
	mbActive = bActive;
}

D2D1_SIZE_F Label::GetScale() const
{
	return mScale;
}

void Label::SetScale(const D2D1_SIZE_F scale)
{
	mScale = scale;
}

D2D1_POINT_2F Label::GetPosition() const
{
	return mPosition;
}

void Label::SetPosition(const D2D1_POINT_2F position)
{
	mPosition = position;
}

D2D1_POINT_2F Label::GetCenter() const
{
	return mCenter;
}

void Label::SetCenter(const D2D1_POINT_2F center)
{
	mCenter = center;
}

float Label::GetAngle() const
{
	return mAngle;
}

void Label::SetAngle(const float angle)
{
	mAngle = angle;
}

float Label::GetOpacity() const
{
	return mOpacity;
}

void Label::SetOpacity(const float opacity)
{
	mOpacity = opacity;
}

bool Label::IsUI() const
{
	return mbUI;
}

void Label::SetUI(const bool bUI)
{
	mbUI = bUI;
}

Font* Label::_GetFontOrNull() const
{
	return mFont;
}
