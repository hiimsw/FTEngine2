#include "pch.h"
#include "Constant.h"

Constant& Constant::Get()
{
	static Constant constant{};
	return constant;
}

int32_t Constant::GetWidth() const
{
	return 1280;
}

int32_t Constant::GetHeight() const
{
	return 720;
}