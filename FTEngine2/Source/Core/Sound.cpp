#include "pch.h"
#include "Sound.h"

#include "Helper.h"

void Sound::Initialize(Helper* helper, const std::string& filename, const bool bLoop)
{
	ASSERT(helper != nullptr);

	FMOD::System* system = helper->GetSoundSystem();

	FMOD::Sound* fmodSound = nullptr;
	FC(system->createSound(filename.c_str(), not bLoop ? FMOD_DEFAULT : FMOD_LOOP_NORMAL, nullptr, &fmodSound));
	MASSERT(fmodSound != nullptr, "사운드 파일을 찾을 수 없습니다.");

	FC(system->playSound(fmodSound, nullptr, true, &mChannel));
	FC(fmodSound->getLength(&mLength, FMOD_TIMEUNIT_MS));
}

void Sound::Finalize()
{
	FC(mChannel->stop());
}

void Sound::Play()
{
	FC(mChannel->setPaused(false));
}

void Sound::Replay()
{
	FC(mChannel->setPosition(0, FMOD_TIMEUNIT_MS));
	FC(mChannel->setPaused(false));
}

void Sound::Pause()
{
	FC(mChannel->setPaused(true));
}

float Sound::GetVolume() const
{
	float volume = 0.0;
	FC(mChannel->getVolume(&volume));

	return volume;
}

void Sound::SetVolume(float volume)
{
	FC(mChannel->setVolume(volume));
}

unsigned int Sound::GetLength() const
{
	return mLength;
}

float Sound::GetElapsedTime()
{
	unsigned int pos;
	FC(mChannel->getPosition(&pos, FMOD_TIMEUNIT_MS));

	return pos * 0.001f;
}
