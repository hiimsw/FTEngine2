#pragma once

class Helper;

class Font final
{
public:
	Font() = default;
	Font(const Font&) = delete;
	Font& operator=(const Font&) = delete;

	void Initialize(Helper* helper, const std::wstring& filename, const float fontSize);
	void Finalize();

public:
	[[nodiscard]] D2D1_SIZE_F _CalculateTextSize(const std::wstring& text) const;
	[[nodiscard]] IDWriteTextFormat* _GetTextFormat() const;

private:
	Helper* mHelper = nullptr;
	IDWriteTextFormat* mTextFormat = nullptr;
};