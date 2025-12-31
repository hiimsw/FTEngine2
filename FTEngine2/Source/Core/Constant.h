#pragma once

class Constant
{
public:
	[[nodiscard]] static Constant& Get();

	[[nodiscard]] int32_t GetWidth() const;
	[[nodiscard]] int32_t GetHeight() const;

private:
	Constant() = default;
	Constant(const Constant&) = delete;
	Constant& operator=(const Constant&) = delete;
	~Constant() = default;
};