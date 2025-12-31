#pragma once

class Helper;

class Texture final
{
public:
	Texture() = default;
	Texture(const Texture& other);
	Texture& operator=(const Texture&) = delete;

	void Initialize(Helper* helper, const std::wstring& filename);
	void Finalize();

	[[nodiscard]] uint32_t GetWidth() const;
	[[nodiscard]] uint32_t GetHeight() const;

public:
	[[nodiscard]] ID2D1Bitmap* _GetBitmap() const;

private:
	ID2D1Bitmap* mBitmap = nullptr;
};