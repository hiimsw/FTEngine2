#pragma once

class Helper;

class Sound final
{
public:
	Sound() = default;
	Sound& operator=(const Sound&) = delete;

public:
	void Initialize(Helper* helper, const std::string& filename, const bool bLoop);
	void Finalize();
	void Play();
	void Replay();
	void Pause();

	float GetVolume() const;
	void SetVolume(float volume);
	unsigned int GetLength() const;
	float GetElapsedTime();

private:
	FMOD::Channel* mChannel = nullptr;
	unsigned int mLength = 0;
};