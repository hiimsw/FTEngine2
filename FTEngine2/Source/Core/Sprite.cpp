#include "pch.h"
#include "Sprite.h"

#include "Texture.h"

const Texture* Sprite::GetTextureOrNull() const
{
	return mTexture;
}

void Sprite::SetTexture(Texture* texture)
{
	ASSERT(texture != nullptr);

	mTexture = texture;
}

bool Sprite::IsActive() const
{
	return mbActive;
}

void Sprite::SetActive(bool bActive)
{
	mbActive = bActive;
}

D2D1_SIZE_F Sprite::GetScale() const
{
	return mScale;
}

void Sprite::SetScale(const D2D1_SIZE_F& scale)
{
	mScale = scale;
}

D2D1_POINT_2F Sprite::GetPosition() const
{
	return mPosition;
}

void Sprite::SetPosition(const D2D1_POINT_2F& position)
{
	mPosition = position;
}

D2D1_POINT_2F Sprite::GetCenter() const
{
	return mCenter;
}

void Sprite::SetCenter(const D2D1_POINT_2F& center)
{
	mCenter = center;
}

float Sprite::GetAngle() const
{
	return mAngle;
}

void Sprite::SetAngle(const float angle)
{
	mAngle = angle;
}

float Sprite::GetOpacity() const
{
	return mOpacity;
}

void Sprite::SetOpacity(const float opacity)
{
	mOpacity = opacity;
}

bool Sprite::IsUI() const
{
	return mbUI;
}

void Sprite::SetUI(const bool bUI)
{
	mbUI = bUI;
}

Texture* Sprite::_GetTextureOrNull() const
{
	return mTexture;
}
