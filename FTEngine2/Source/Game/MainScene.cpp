#include "pch.h"
#include "MainScene.h"

#include "Core/Collision.h"
#include "Core/Constant.h"
#include "Core/Helper.h"
#include "Core/Input.h"
#include "Core/Transformation.h"

using namespace D2D1;

void MainScene::Initialize()
{
	// 기본 데이터를 초기화한다.
	{
		for (std::vector<Sprite*>& layer : mSpriteLayers)
		{
			layer.reserve(128);
		}

		SetSpriteLayers(mSpriteLayers.data(), uint32_t(mSpriteLayers.size()));

		SetCamera(&mMainCamera);

		mLabels.reserve(16);
		SetLabels(&mLabels);

		mTimerFont.Initialize(GetHelper(), L"Arial", 40.0f);
		mDefaultFont.Initialize(GetHelper(), L"Arial", 20.0f);
		mBulletFont.Initialize(GetHelper(), L"Arial", 30.0f);

		Input::Get().SetCursorVisible(false);
		Input::Get().SetCursorLockState(Input::eCursorLockState::Confined);

		ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();
		HR(renderTarget->CreateSolidColorBrush(ColorF(1.0f, 1.0f, 1.0f), &mDefaultBrush));
		HR(renderTarget->CreateSolidColorBrush(ColorF(ColorF::Yellow), &mYellowBrush));
		HR(renderTarget->CreateSolidColorBrush(ColorF(ColorF::Orange), &mOrange));
		HR(renderTarget->CreateSolidColorBrush(ColorF(ColorF::Cyan), &mCyanBrush));
		HR(renderTarget->CreateSolidColorBrush(ColorF(ColorF::DarkGreen), &mDarkGreen));

		mIsCursorConfined = (Input::Get().GetCursorLockState() == Input::eCursorLockState::Confined);

		srand(unsigned int(time(nullptr)));
	}

	// 사용되는 이미지를 초기화한다.
	{
		mRectangleTexture.Initialize(GetHelper(), L"Resource/Rectangle.png");
		mRedRectangleTexture.Initialize(GetHelper(), L"Resource/RedRectangle.png");
		mYellowRectangleTexture.Initialize(GetHelper(), L"Resource/YellowRectangle.png");
		mSkyBlueRectangleTexture.Initialize(GetHelper(), L"Resource/SkyBlueRectangle.png");
		mBlueRectangleTexture.Initialize(GetHelper(), L"Resource/BlueRectangle.png");
		mPinkRectangleTexture.Initialize(GetHelper(), L"Resource/PinkRectangle.png");
		mPurpleRectangleTexture.Initialize(GetHelper(), L"Resource/PurpleRectangle.png");
		mBlackRectangleTexture.Initialize(GetHelper(), L"Resource/BlackRectangle.png");

		mCircleTexture.Initialize(GetHelper(), L"Resource/Circle.png");
		mRedCircleTexture.Initialize(GetHelper(), L"Resource/RedCircle.png");

		mWhiteBarTexture.Initialize(GetHelper(), L"Resource/WhiteBar.png");
		mRedBarTexture.Initialize(GetHelper(), L"Resource/RedBar.png");
		mYellowBarTexture.Initialize(GetHelper(), L"Resource/YellowBar.png");
		mBlueBarTexture.Initialize(GetHelper(), L"Resource/BlueBar.png");
	}

	// 사운드를 초기화한다.
	{
		mBackgroundSound.Initialize(GetHelper(), "Resource/Sound/DST-TowerDefenseTheme.mp3", true);
		mBackgroundSound.SetVolume(0.3f);
		mBackgroundSound.Play();

		mBulletSound.Initialize(GetHelper(), "Resource/Sound/shoot_sound.wav", false);

		mReloadSound.Initialize(GetHelper(), "Resource/Sound/reload.mp3", false);
		mReloadSound.SetVolume(0.5f);

		mHeroHitSound.Initialize(GetHelper(), "Resource/Sound/hit.mp3", false);
		mHeroHitSound.SetVolume(1.0f);

		mDashSound.Initialize(GetHelper(), "Resource/Sound/dash.mp3", false);
		mDashSound.SetVolume(0.3f);

		mShieldSound.Initialize(GetHelper(), "Resource/Sound/E_Skill.mp3", false);
		mShieldSound.SetVolume(0.2f);

		mOrbitSound.Initialize(GetHelper(), "Resource/Sound/Q_Skill.mp3", false);
		mOrbitSound.SetVolume(0.2f);

		mBigMonsterDeadSound.Initialize(GetHelper(), "Resource/Sound/bone_break.mp3", false);
		mBigMonsterDeadSound.SetVolume(0.5f);

		mRunMonsterDeadSound.Initialize(GetHelper(), "Resource/Sound/bone_break2.mp3", false);
		mRunMonsterDeadSound.SetVolume(0.5f);

		mSlowMonsterDeadSound.Initialize(GetHelper(), "Resource/Sound/bone_break3.mp3", false);
		mSlowMonsterDeadSound.SetVolume(0.5f);

		mGameOverSound.Initialize(GetHelper(), "Resource/Sound/game_over.mp3", false);
		mGameOverSound.SetVolume(0.3f);

		mButtonSound.Initialize(GetHelper(), "Resource/Sound/button_sound.wav", false);
		mButtonSound.SetVolume(0.2f);
	}

	// 플레이어를 초기화한다.
	{
		mHero.hp = HERO_MAX_HP;
		mHero.prevHp = mHero.hp;

		Sprite& hero = mHero.sprite;
		hero.SetPosition({ .x = -200.0f, .y = 0.0f });
		hero.SetScale({ .width = 0.75f, .height = 0.75f });
		hero.SetTexture(&mPinkRectangleTexture);
		mSpriteLayers[uint32_t(Layer::Player)].push_back(&hero);

		for (Sprite& shadow : mDash.shadow)
		{
			shadow.SetScale(hero.GetScale());
			shadow.SetTexture(&mPinkRectangleTexture);
			shadow.SetActive(false);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&shadow);
		}

		mDash.count = DASH_MAX_COUNT;
	}

	// 총알을 초기화한다.
	{
		for (Bullet& bullet : mBullets)
		{
			Sprite& sprite = bullet.sprite;
			sprite.SetPosition(mHero.sprite.GetPosition());
			sprite.SetCenter({ .x = -0.5f, .y = 0.0f });
			sprite.SetScale({ 2.5f, 0.1f });
			sprite.SetActive(false);
			sprite.SetTexture(&mRedRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Player)].push_back(&sprite);
		}
	}

	// 탄피를 초기화한다.
	{
		for (Casing& casing : mCasings)
		{
			Sprite& sprite = casing.sprite;
			sprite.SetScale({ .width = 0.2f, .height = 0.2f });
			sprite.SetCenter({ -0.5f, 0.0f });
			sprite.SetOpacity(0.3f);
			sprite.SetActive(false);
			sprite.SetTexture(&mRedRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Effect)].push_back(&sprite);
		}
	}

	// 쉴드 스킬을 초기화한다.
	mShield.state = eShield_State::End;

	// 공전 스킬을 초기화한다.
	{
		mOrbit.ellipse =
		{
			.point = {.x = 0.0f, .y = 160.0f },
			.radiusX = 20.0f,
			.radiusY = 20.0f,
		};

		mOrbit.state = eOrbit_State::End;
	}

	// 몬스터를 초기화한다.
	{
		for (Monster& monster : mBigMonsters)
		{
			initializeMonster
			(
				{
					.monster = &monster,
					.maxHp = BIG_MONSTER_MAX_HP,
					.monsterScale = { BIG_MONSTER_SCALE, BIG_MONSTER_SCALE },
					.hpBackgroundScale = { BIG_MONSTER_HP_BAR_WIDTH, 0.7f },
					.hpScale = { BIG_MONSTER_HP_BAR_WIDTH, 0.7f }
				}
			);
		}
	}

	// 돌진 몬스터를 초기화한다.
	{
		for (RunMonster& run : mRunMonsters)
		{
			initializeMonster
			(
				{
					.monster = &run.monster,
					.maxHp = RUN_MONSTER_MAX_HP,
					.monsterScale = { RUN_MONSTER_SCALE, RUN_MONSTER_SCALE },
					.hpBackgroundScale = { RUN_MONSTER_HP_BAR_WIDTH, 0.5f },
					.hpScale = { RUN_MONSTER_HP_BAR_WIDTH, 0.5f }
				}
			);

			Sprite& startBar = run.startBar;
			startBar.SetScale({ .width = 0.0f, .height = 0.1f });
			startBar.SetCenter({ .x = -0.5f, .y = 0.0f });
			startBar.SetActive(false);
			startBar.SetTexture(&mRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Monster)].push_back(&startBar);
		}
	}

	// 느린 몬스터를 초기화한다.
	{		
		for (SlowMonster& slow : mSlowMonsters)
		{
			// 그림자
			for (Sprite& shadow : slow.shadow)
			{
				shadow.SetScale({ .width = SLOW_MONSTER_SCALE, .height = SLOW_MONSTER_SCALE });
				shadow.SetOpacity(1.0f);
				shadow.SetActive(false);
				shadow.SetTexture(&mRectangleTexture);
				mSpriteLayers[uint32_t(Layer::Monster)].push_back(&shadow);
			}

			initializeMonster
			(
				{
					.monster = &slow.monster,
					.maxHp = SLOW_MONSTER_MAX_HP,
					.monsterScale = { SLOW_MONSTER_SCALE, SLOW_MONSTER_SCALE },
					.hpBackgroundScale = { SLOW_MONSTER_HP_BAR_WIDTH, 0.5f },
					.hpScale = { SLOW_MONSTER_HP_BAR_WIDTH, 0.5f }
				}
			);

			slow.moveState = eSlow_Monster_State::End;
		}
	}

	// 이펙트를 초기화한다.
	{
		for (Sprite& effect : mLongEffect)
		{
			effect.SetScale({ LONG_EFFECT_SCALE.width, LONG_EFFECT_SCALE.height });
			effect.SetActive(false);
			effect.SetTexture(&mSkyBlueRectangleTexture);
			mSpriteLayers[uint32_t(Layer::Effect)].push_back(&effect);
		}
	}

	// 파티클을 초기화한다.
	{
		mRedStarTexture.Initialize(GetHelper(), L"Resource/RedStar.png");
		mOrangeStarTexture.Initialize(GetHelper(), L"Resource/OrangeStar.png");
		mYellowStarTexture.Initialize(GetHelper(), L"Resource/YellowStar.png");
		mGreenStarTexture.Initialize(GetHelper(), L"Resource/GreenStar.png");
		mBlueStarTexture.Initialize(GetHelper(), L"Resource/BlueStar.png");
		mPurpleStarTexture.Initialize(GetHelper(), L"Resource/PurpleStar.png");

		constexpr uint32_t COLOR_COUNT = 6;
		Texture* starParticleTextures[COLOR_COUNT] =
		{
			&mRedStarTexture,
			&mOrangeStarTexture,
			&mYellowStarTexture,
			&mGreenStarTexture,
			&mBlueStarTexture,
			&mPurpleStarTexture
		};

		Texture* rectParticleTextures[COLOR_COUNT] =
		{
			&mRectangleTexture,
			&mRedRectangleTexture,
			&mYellowRectangleTexture,
			&mPinkRectangleTexture,
			&mBlueRectangleTexture,
			&mPurpleStarTexture
		};

		// Star
		for (uint32_t i = 0; i < mStarParticles.size(); ++i)
		{
			Particle& particle = mStarParticles[i];
			particle.direction = {};
			particle.speed = getRandom(100.0f, 300.0f);

			Sprite& sprite = particle.sprite;
			sprite.SetScale({ .width = 0.5f, .height = 0.5f });
			sprite.SetActive(false);
			sprite.SetTexture(starParticleTextures[i % COLOR_COUNT]);
			mSpriteLayers[uint32_t(Layer::Effect)].push_back(&sprite);
		}

		// Rect
		for (uint32_t i = 0; i < mRectParticles.size(); ++i)
		{
			Particle& particle = mRectParticles[i];

			particle.direction = {};
			particle.speed = getRandom(100.0f, 300.0f);

			Sprite& sprite = particle.sprite;
			sprite.SetScale({ .width = 0.7f, .height = 0.7f });
			sprite.SetAngle(45.0f);
			sprite.SetActive(false);
			sprite.SetTexture(rectParticleTextures[i % COLOR_COUNT]);
			mSpriteLayers[uint32_t(Layer::Effect)].push_back(&sprite);
		}
	}

	// 줌을 초기화한다.
	{
		mZoom.SetScale({ .width = 0.7f, .height = 0.7f });
		mZoom.SetAngle(45.0f);
		mZoom.SetUI(true);
		mZoom.SetTexture(&mRedRectangleTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mZoom);
	}

	// 카메라를 초기화한다.
	{
		const D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
		const D2D1_POINT_2F cameraPosition{ .x = heroPosition.x * 0.5f, .y = heroPosition.y * 0.5f };

		mMainCamera.SetPosition(cameraPosition);
	}

	// Dash 게이지를 초기화한다.
	{
		mUiBackgroundDashBar.SetPosition({ .x = -300.0f, .y = -UI_CENTER_POSITION_Y });
		mUiBackgroundDashBar.SetScale({ .width = UI_DASH_SCALE_WIDTH, .height = 1.0f });
		mUiBackgroundDashBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiBackgroundDashBar.SetUI(true);
		mUiBackgroundDashBar.SetTexture(&mWhiteBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiBackgroundDashBar);

		D2D1_POINT_2F offset = { .x = mUiBackgroundDashBar.GetPosition().x + 2.5f, .y = mUiBackgroundDashBar.GetPosition().y };
		mUiDashBar.SetPosition(offset);

		mUiDashBar.SetScale(mUiBackgroundDashBar.GetScale());
		mUiDashBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiDashBar.SetUI(true);
		mUiDashBar.SetTexture(&mYellowBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiDashBar);
	}

	// HP바를 초기화한다.
	{
		constexpr float OFFSET_Y = 50.0f;
		mUiBackgroundHpBar.SetPosition({ .x = mUiBackgroundDashBar.GetPosition().x, .y = mUiBackgroundDashBar.GetPosition().y + OFFSET_Y });

		mUiBackgroundHpBar.SetScale({ .width = UI_HP_SCALE_WIDTH, .height = 1.0f });
		mUiBackgroundHpBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiBackgroundHpBar.SetUI(true);
		mUiBackgroundHpBar.SetTexture(&mWhiteBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiBackgroundHpBar);

		D2D1_POINT_2F offset = { .x = mUiBackgroundHpBar.GetPosition().x + 2.5f, .y = mUiBackgroundHpBar.GetPosition().y };
		mUiHpBar.SetPosition(offset);

		mUiHpBar.SetScale(mUiBackgroundHpBar.GetScale());
		mUiHpBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiHpBar.SetUI(true);
		mUiHpBar.SetTexture(&mRedBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiHpBar);
	}

	// All Kill Bar를 초기화한다.
	{
		mUiKillBackgroundBar.SetPosition({ .x = -float(Constant::Get().GetWidth()) * 0.5f + 100.0f, .y = mUiBackgroundHpBar.GetPosition().y - 20.0f });
		mUiKillBackgroundBar.SetScale({ .width = UI_KILL_SCALE_WIDTH, .height = 1.0f });
		mUiKillBackgroundBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiKillBackgroundBar.SetUI(true);
		mUiKillBackgroundBar.SetTexture(&mWhiteBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiKillBackgroundBar);
		
		mUiKillCountBar.SetPosition(mUiKillBackgroundBar.GetPosition());

		float targetWidth = UI_KILL_SCALE_WIDTH * (float(mKillMonsterCount) / float(KILL_ALL_MONSTER_COUNT));
		mUiKillCountBar.SetScale({ targetWidth, mUiKillBackgroundBar.GetScale().height});

		mUiKillCountBar.SetCenter({ .x = -0.5f, .y = 0.0f });
		mUiKillCountBar.SetUI(true);
		mUiKillCountBar.SetTexture(&mBlueBarTexture);
		mSpriteLayers[uint32_t(Layer::UI)].push_back(&mUiKillCountBar);
	}

	// 라벨을 초기화한다.
	{
		// 타이머
		{
			mTimerLabel.SetFont(&mTimerFont);
			mTimerLabel.SetUI(true);
			mTimerLabel.SetPosition({ .x = 0.0f, .y = UI_CENTER_POSITION_Y });
			mLabels.push_back(&mTimerLabel);
		}

		// 현재 대쉬 개수
		{
			mDashValueLabel.SetFont(&mDefaultFont);
			mDashValueLabel.SetUI(true);

			const D2D1_POINT_2F dashBarPosition = mUiBackgroundDashBar.GetPosition();
			constexpr float OFFSET_X = 10.0f;
			D2D1_POINT_2F offset = { .x = dashBarPosition.x - OFFSET_X, .y = dashBarPosition.y };
			mDashValueLabel.SetPosition(offset);

			mDashValueLabel.SetCenter({ .x = 0.5f, .y = 0.0f });
			mDashValueLabel.SetText(L"Dash: " + std::to_wstring(mDash.count) + L" / " + std::to_wstring(DASH_MAX_COUNT));
			mLabels.push_back(&mDashValueLabel);
		}

		// 현재 체력
		{
			mHpValueLabel.SetFont(&mDefaultFont);
			mHpValueLabel.SetUI(true);

			const D2D1_POINT_2F hpBarPosition = mUiHpBar.GetPosition();
			constexpr float OFFSET_X = 10.0f;
			D2D1_POINT_2F offset = { .x = hpBarPosition.x - OFFSET_X, .y = hpBarPosition.y };
			mHpValueLabel.SetPosition(offset);

			mHpValueLabel.SetCenter({ .x = 0.5f, .y = 0.0f });
			mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHero.hp) + L" / " + std::to_wstring(HERO_MAX_HP));
			mLabels.push_back(&mHpValueLabel);
		}

		// 총알 개수
		{
			mBulletLabel.SetFont(&mBulletFont);
			mBulletLabel.SetUI(true);

			const D2D1_POINT_2F position = mUiBackgroundHpBar.GetPosition();
			float barOffsetX = mUiBackgroundHpBar.GetScale().width * mWhiteBarTexture.GetWidth();
			const D2D1_POINT_2F offset = { .x = position.x + barOffsetX + 55.0f, .y = position.y };
			mBulletLabel.SetPosition(offset);

			mBulletLabel.SetText(std::to_wstring(mBulletValue) + L"/" + std::to_wstring(BULLET_COUNT));
			mBulletLabel.SetCenter({ .x = -0.5f, .y = 0.0f });
			mLabels.push_back(&mBulletLabel);
		}

		// 쉴드 쿨 타이머
		{
			mShieldLabel.SetFont(&mDefaultFont);
			mShieldLabel.SetUI(true);

			const D2D1_POINT_2F position = mUiBackgroundDashBar.GetPosition();
			float barOffsetX = mUiBackgroundDashBar.GetScale().width * mWhiteBarTexture.GetWidth();
			const D2D1_POINT_2F offset = { .x = position.x + barOffsetX + 65.0f, .y = position.y };
			mShieldLabel.SetPosition(offset);

			mShieldLabel.SetCenter({ .x = -0.5f, .y = 0.0f });
			mLabels.push_back(&mShieldLabel);
		}

		// 공전 쿨 타이머
		{
			mOrbitLabel.SetFont(&mDefaultFont);
			mOrbitLabel.SetUI(true);

			const D2D1_POINT_2F position = mShieldLabel.GetPosition();
			const D2D1_POINT_2F offset = { .x = position.x + 70.0f, .y = position.y };
			mOrbitLabel.SetPosition(offset);

			mOrbitLabel.SetCenter({ .x = -0.5f, .y = 0.0f });
			mLabels.push_back(&mOrbitLabel);
		}

		// 쉴드 키 설명
		{
			mShieldKeyLabel.SetFont(&mDefaultFont);
			mShieldKeyLabel.SetUI(true);

			const D2D1_POINT_2F position = mUiBackgroundDashBar.GetPosition();
			float barOffsetX = mUiBackgroundDashBar.GetScale().width * mWhiteBarTexture.GetWidth();
			const D2D1_POINT_2F offset = { .x = position.x + barOffsetX + 65.0f, .y = position.y };
			mShieldKeyLabel.SetPosition(offset);

			mShieldKeyLabel.SetText(L"E");
			mShieldKeyLabel.SetCenter({ .x = -0.5f, .y = 0.0f });
			mLabels.push_back(&mShieldKeyLabel);
		}

		// 공전 키 설명
		{
			mOrbitKeyLabel.SetFont(&mDefaultFont);
			mOrbitKeyLabel.SetUI(true);

			const D2D1_POINT_2F position = mShieldKeyLabel.GetPosition();
			const D2D1_POINT_2F offset = { .x = position.x + 68.0f, .y = position.y + 3.0f };
			mOrbitKeyLabel.SetPosition(offset);

			mOrbitKeyLabel.SetText(L"Q");
			mOrbitKeyLabel.SetCenter({ .x = -0.5f, .y = 0.0f });
			mLabels.push_back(&mOrbitKeyLabel);
		}

		// 모든 몬스터 삭제 라벨
		{
			mKillAllMonsterLabel.SetFont(&mDefaultFont);
			mKillAllMonsterLabel.SetUI(true);

			D2D1_POINT_2F offset = { .x = mUiKillBackgroundBar.GetPosition().x - 5.0f, .y = mUiKillBackgroundBar.GetPosition().y };
			mKillAllMonsterLabel.SetPosition(offset);
			mKillAllMonsterLabel.SetText(L"F: " + std::to_wstring(mKillMonsterCount) + L" / " + std::to_wstring(KILL_ALL_MONSTER_COUNT));
			mKillAllMonsterLabel.SetCenter({ .x = 0.5f, .y = 0.0f });
			mLabels.push_back(&mKillAllMonsterLabel);
		}

		// 엔딩
		{
			mGameOverBackground.SetPosition({ .x = 0.0f, .y = -60.0f });
			mGameOverBackground.SetScale({ 10.0f, 6.0f });
			mGameOverBackground.SetUI(true);
			mGameOverBackground.SetActive(false);
			mGameOverBackground.SetTexture(&mBlackRectangleTexture);
			mSpriteLayers[uint32_t(Layer::UI)].push_back(&mGameOverBackground);

			mGameOverTexture.Initialize(GetHelper(), L"Resource/GameOver.png");
			mGameOver.SetScale({ .width = 1.5f, .height = 1.5f });
			mGameOver.SetUI(true);
			mGameOver.SetActive(false);
			mGameOver.SetTexture(&mGameOverTexture);
			mSpriteLayers[uint32_t(Layer::UI)].push_back(&mGameOver);
		}

		// 다시 시작 버튼
		{
			mResumeIdleButtonTexture.Initialize(GetHelper(), L"Resource/resume_idle_button.png");
			mResumeContactButtonTexture.Initialize(GetHelper(), L"Resource/resume_contact_button.png");

			mResumeButton.SetPosition({ .x = 0.0f, .y = 0.0f });
			mResumeButton.SetScale({ .width = 0.7f, .height = 0.7f });
			mResumeButton.SetUI(true);
			mResumeButton.SetActive(false);
			mResumeButton.SetTexture(&mResumeIdleButtonTexture);
			mSpriteLayers[uint32_t(Layer::UI)].push_back(&mResumeButton);
		}

		// 종료 버튼
		{
			mExitIdleButtonTexture.Initialize(GetHelper(), L"Resource/exit_idle_button.png");
			mExitContactButtonTexture.Initialize(GetHelper(), L"Resource/exit_contact_button.png");

			mExitButton.SetPosition({ .x = 0.0f, .y = -120.0f });
			mExitButton.SetScale({ .width = 0.7f, .height = 0.7f });
			mExitButton.SetUI(true);
			mExitButton.SetActive(false);
			mExitButton.SetTexture(&mExitIdleButtonTexture);
			mSpriteLayers[uint32_t(Layer::UI)].push_back(&mExitButton);
		}
	}
}

void MainScene::PreDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
	ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();

	// 바운더리를 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix() * view;
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse{ .radiusX = BOUNDARY_RADIUS, .radiusY = BOUNDARY_RADIUS };
		renderTarget->DrawEllipse(ellipse, mDefaultBrush, 2.0f);
	}

	// 내부 바운더리를 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix() * view;
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse{ .radiusX = IN_BOUNDARY_RADIUS, .radiusY = IN_BOUNDARY_RADIUS };
		renderTarget->DrawEllipse(ellipse, mDefaultBrush, 2.0f);
	}
}

bool MainScene::Update(const float deltaTime)
{
	// 키를 업데이트한다.
	{
		// 게임을 종료한다.
		if (Input::Get().GetKeyDown(VK_ESCAPE))
		{
			PostQuitMessage(0);
		}

		// 마우스 커서를 설정한다.
		if (Input::Get().GetKeyDown(VK_CONTROL))
		{
			mIsCursorConfined = !mIsCursorConfined;
			Input::Get().SetCursorLockState(mIsCursorConfined ? Input::eCursorLockState::Confined : Input::eCursorLockState::None);
			Input::Get().SetCursorVisible(not mIsCursorConfined);
		}

#ifdef _DEBUG
		if (Input::Get().GetKeyDown('T'))
		{
			mIsColliderKeyDown = !mIsColliderKeyDown;
		}
#endif
	}

	// 줌을 업데이트한다.
	{
		// 세팅되어 있는 좌표(0, 0)으로 맞춰준다.
		const D2D1_POINT_2F centerOffset =
		{
			.x = (Constant::Get().GetWidth() - 1.0f) * 0.5f,
			.y = (Constant::Get().GetHeight() - 1.0f) * 0.5f
		};

		const D2D1_POINT_2F mousePosition = Input::Get().GetMousePosition();

		// 마우스 좌표를 좌표계에 맞춘다.
		const D2D1_POINT_2F screenPosition = Math::SubtractVector(mousePosition, centerOffset);
		mZoom.SetPosition(screenPosition);

		const D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
		const D2D1_POINT_2F zoomPosition = getMouseWorldPosition();

		D2D1_POINT_2F toTarget = Math::SubtractVector(zoomPosition, heroPosition);
		const float length = Math::GetVectorLength(toTarget);

		constexpr float MAX_SCALE = 0.7f;
		constexpr float MIN_SCALE = 0.5f;
		constexpr float MAX_DISTANCE = 300.0f;
		float zoomScale = std::clamp(length / MAX_DISTANCE, MIN_SCALE, MAX_SCALE);

		mZoom.SetScale({ .width = zoomScale, .height = zoomScale });
	}

	// 플레이어를 업데이트한다.
	{
		// 이동을 업데이트한다.
		{
			constexpr float MAX_SPEED = 400.0f;
			constexpr float ACC = 50.0f; // 가속도

			const int32_t moveX = Input::Get().GetKey('D') - Input::Get().GetKey('A');
			const int32_t moveY = Input::Get().GetKey('W') - Input::Get().GetKey('S');

			static int32_t previousMoveX;
			static int32_t previousMoveY;
			D2D1_POINT_2F& velocity = mHero.velocity;

			if (moveX != 0)
			{
				velocity.x = std::clamp(velocity.x + ACC * moveX, -MAX_SPEED, MAX_SPEED);
				previousMoveX = moveX;
			}
			else
			{
				if (previousMoveX > 0)
				{
					velocity.x = max(velocity.x - ACC, 0.0f);
				}
				else
				{
					velocity.x = min(velocity.x + ACC, 0.0f);
				}
			}

			if (moveY != 0)
			{
				velocity.y = std::clamp(velocity.y + ACC * moveY, -MAX_SPEED, MAX_SPEED);
				previousMoveY = moveY;
			}
			else
			{
				if (previousMoveY > 0)
				{
					velocity.y = max(velocity.y - ACC, 0.0f);
				}
				else
				{
					velocity.y = min(velocity.y + ACC, 0.0f);
				}
			}

			constexpr float MAX_DASH_SPEED = 600.0f;
			constexpr float DASH_ACC = 30.0f;
			static D2D1_POINT_2F dashDirection;

			if (Math::GetVectorLength(velocity) != 0.0f)
			{
				float speed = min(Math::GetVectorLength(velocity), MAX_SPEED);
				const D2D1_POINT_2F direction = Math::NormalizeVector(velocity);
				D2D1_POINT_2F adjustVelocity = Math::ScaleVector(direction, speed);
				adjustVelocity = Math::ScaleVector(adjustVelocity, deltaTime);

				if (not mDash.isActive and Input::Get().GetKeyDown(VK_SPACE))
				{
					dashDirection = direction;

					if (mDash.count != 0)
					{
						mDash.count--;
						mDash.isActive = true;
						mDashSound.Replay();
						mDash.shadowCoolTimer = 0.0f;
					}
				}

				const D2D1_POINT_2F position = Math::AddVector(mHero.sprite.GetPosition(), adjustVelocity);
				mHero.sprite.SetPosition(position);
			}

			if (mDash.isActive)
			{
				mDash.moveSpeed = min(mDash.moveSpeed + DASH_ACC, MAX_DASH_SPEED);
				const D2D1_POINT_2F velocity = Math::ScaleVector(dashDirection, mDash.moveSpeed * deltaTime);

				if (mDash.moveSpeed >= MAX_DASH_SPEED)
				{
					mDash.moveSpeed = 0.0f;
					mDash.isActive = false;
				}

				const D2D1_POINT_2F position = Math::AddVector(mHero.sprite.GetPosition(), velocity);
				mHero.sprite.SetPosition(position);

				mDash.shadowCoolTimer -= deltaTime;
				if (mDash.shadowCoolTimer <= 0.0f)
				{
					for (Sprite& shadow : mDash.shadow)
					{
						if (shadow.IsActive())
						{
							continue;
						}

						shadow.SetOpacity(1.0f);
						shadow.SetPosition(position);
						shadow.SetActive(true);
						break;
					}

					mDash.shadowCoolTimer = 0.05f;
				}
			}

			for (Sprite& shadow : mDash.shadow)
			{
				if (not shadow.IsActive())
				{
					continue;
				}

				float opacity = shadow.GetOpacity();
				opacity -= 5.0f * deltaTime;
				shadow.SetOpacity(opacity);

				if (opacity <= 0.0f)
				{
					shadow.SetActive(false);
				}
			}

			if (mDash.count < DASH_MAX_COUNT)
			{
				mDash.timer += deltaTime;

				if (mDash.timer >= 2.0f)
				{
					mDash.count++;
					mDash.timer = 0.0f;
				}
			}
		}

		// 총알을 업데이트한다.
		{
			constexpr float LENGTH = 100.0f;

			mBulletShootingCoolTimer = max(mBulletShootingCoolTimer - deltaTime, 0.0f);

			// 총알을 스폰한다.
			if (Input::Get().GetMouseButton(Input::eMouseButton::Left)
				and mBulletShootingCoolTimer <= 0.001f
				and mBulletValue != 0
				and not misKeyDownReload)
			{
				mBulletSound.Replay();

				for (uint32_t i = 0; i < BULLET_COUNT; ++i)
				{
					Bullet& bullet = mBullets[i];
					Sprite& bulletSprite = bullet.sprite;

					if (bulletSprite.IsActive())
					{
						continue;
					}

					const D2D1_POINT_2F spawnPosition = mHero.sprite.GetPosition();
					bulletSprite.SetPosition(spawnPosition);
					bullet.prevPosition = spawnPosition;

					// 이동 방향을 구한다.
					bullet.direction = Math::SubtractVector(getMouseWorldPosition(), spawnPosition);

					// 거리에 따라 반동효과가 다르다.
					const float length = Math::GetVectorLength(bullet.direction);
					bullet.direction = (length >= 200.0f) ?
						Math::RotateVector(bullet.direction, getRandom(-5.0f, 5.0f))
						: Math::RotateVector(bullet.direction, getRandom(0.0f, 0.0f));

					bullet.direction = Math::NormalizeVector(bullet.direction);

					float angle = Math::ConvertRadianToDegree(atan2f(bullet.direction.y, bullet.direction.x));
					bulletSprite.SetAngle(-angle);

					bulletSprite.SetActive(true);

					bulletSprite.SetOpacity(0.0f);

					mBulletValue--;

					// 탄피를 생성한다.
					{
						for (uint32_t j = 0; j < CASING_COUNT; ++j)
						{
							Casing& casing = mCasings[j];
							Sprite& casingSprite = casing.sprite;

							if (casingSprite.IsActive())
							{
								continue;
							}

							casingSprite.SetOpacity(1.0f);
							casingSprite.SetActive(true);

							D2D1_POINT_2F& casingDirection = casing.casingDirection;
							casingDirection = Math::NormalizeVector(bullet.direction);
							casingDirection = Math::RotateVector(casingDirection, getRandom(-30.0f, 30.0f));
							casingDirection = Math::ScaleVector(casingDirection, -1.0f);	// 뒤로 가도록 조정한다.

							D2D1_POINT_2F spawnPosition = mHero.sprite.GetPosition();
							constexpr float OFFSET = 50.0f;
							spawnPosition = Math::AddVector(spawnPosition, Math::ScaleVector(casingDirection, OFFSET));
							casingSprite.SetPosition((spawnPosition));

							// 탄피의 이동 좌표를 생성한다.
							casing.startPosition = casingSprite.GetPosition();
							casing.endPosition = Math::AddVector(casing.startPosition, Math::ScaleVector(casingDirection, LENGTH));

							break;
						}
					}

					// 카메라 흔들기를 시작한다.
					{
						const float amplitude = Constant::Get().GetHeight() * getRandom(0.008f, 0.012f);
						const float duration = getRandom(0.05f, 0.08f);
						const float frequency = getRandom(50.0f, 60.0f);
						initializeCameraShake(amplitude, duration, frequency);
					}

					break;
				}

				mBulletShootingCoolTimer = 0.12f;
			}

			// 총알을 이동시킨다.
			{
				constexpr float MOVE_SPEED = 1500.0f;

				for (uint32_t i = 0; i < BULLET_COUNT; ++i)
				{
					Bullet& bullet = mBullets[i];
					Sprite& bulletSprite = bullet.sprite;

					if (not bulletSprite.IsActive())
					{
						continue;
					}

					const D2D1_POINT_2F velocity = Math::ScaleVector(bullet.direction, MOVE_SPEED * deltaTime);
					const D2D1_POINT_2F position = Math::AddVector(bulletSprite.GetPosition(), velocity);

					bullet.prevPosition = bulletSprite.GetPosition();
					bulletSprite.SetPosition(position);

					float opacity = bulletSprite.GetOpacity();

					D2D1_POINT_2F lerp = { .x = opacity, .y = opacity };
					lerp = Math::LerpVector(lerp, { .x = 1.0f, .y = 1.0f }, 10.0f * deltaTime);
					bulletSprite.SetOpacity(lerp.x);
				}
			}

			// 탄피를 이동시킨다.
			{
				constexpr float SPEED = 400.0f;
				constexpr float MOVE_TIME = 1.0f;

				for (uint32_t i = 0; i < CASING_COUNT; ++i)
				{
					Casing& casing = mCasings[i];
					Sprite& casingSprite = casing.sprite;

					if (not casingSprite.IsActive())
					{
						continue;
					}

					casing.casingTimer += deltaTime;

					float t = casing.casingTimer / MOVE_TIME;
					t = std::clamp(t, 0.0f, 1.0f);

					D2D1_POINT_2F position = Math::LerpVector(casing.startPosition, casing.endPosition, t);
					casingSprite.SetPosition(position);

					if (t >= 1.0f)
					{
						casingSprite.SetActive(false);
						casing.casingTimer = 0.0f;
					}

					float opacity = casingSprite.GetOpacity();
					opacity -= 0.8f * deltaTime;
					casingSprite.SetOpacity(opacity);
				}
			}

			// 재장전을 한다.
			{
				constexpr float RELOAD_TIME = 1.2f;

				if (mBulletValue <= 0)
				{
					if (mreloadCoolTimer == 0.0f) 
					{
						mReloadSound.Replay();
					}

					mreloadCoolTimer += deltaTime;

					if (mreloadCoolTimer >= RELOAD_TIME)
					{
						mBulletValue = BULLET_COUNT;
						mreloadCoolTimer = 0.0f;
					}
				}

				// 키를 누르면, 재장전이 된다.
				if (Input::Get().GetKeyDown('R'))
				{
					if (mBulletValue != BULLET_COUNT)
					{
						mReloadSound.Replay();

						misKeyDownReload = true;
					}
				}

				if (misKeyDownReload)
				{
					mreloadKeyDownCoolTimer += deltaTime;

					if (mreloadKeyDownCoolTimer >= RELOAD_TIME)
					{
						mBulletValue = BULLET_COUNT;
						mreloadKeyDownCoolTimer = 0.0f;
						misKeyDownReload = false;
					}
				}
			}
		}

		// 쉴드 키를 업데이트한다.
		{
			if (mShield.state == eShield_State::End
				and Input::Get().GetKeyDown('E'))
			{
				mShieldSound.Replay();

				mShieldLabel.SetActive(true);
				mShield.state = eShield_State::Growing;
			}

			constexpr float SHIELD_SKILL_DURATION = 3.0f;
			constexpr float COOL_TIME = 8.0f;

			// 쿨타임을 표시한다.
			{
				mShield.labelCoolTimer += deltaTime;
				const uint32_t seconds = uint32_t(mShield.labelCoolTimer) % 60;

				if (mShield.state != eShield_State::End)
				{
					mShieldKeyLabel.SetActive(false);
					mShieldLabel.SetText(std::to_wstring(uint32_t(2.0f + SHIELD_SKILL_DURATION + COOL_TIME) - seconds));

				}
				else
				{
					mShieldKeyLabel.SetActive(true);
					mShieldLabel.SetActive(false);
				}
			}

			switch (mShield.state)
			{
			case eShield_State::Growing:
			{
				mShieldKeyLabel.SetActive(false);

				mShield.speed = 100.0f;

				mShield.scale.width += mShield.speed * deltaTime;
				mShield.scale.height += mShield.speed * deltaTime;

				if (mShield.scale.width >= SHELD_MAX_RADIUS)
				{
					mShield.state = eShield_State::Waiting;
				}

				break;
			}

			case eShield_State::Waiting:
			{
				mShield.speed = 0.0f;

				mShield.waitingTimer += deltaTime;

				// 1초 남았을 때 깜빡거린다.
				if ((SHIELD_SKILL_DURATION - mShield.waitingTimer) <= 1.0f)
				{
					mShield.blinkTimer += deltaTime;

					if (mShield.blinkTimer >= 0.1f)
					{
						mShield.isBlinkOn = !mShield.isBlinkOn;
						mShield.blinkTimer = 0.0f;
					}
				}
				else
				{
					mShield.isBlinkOn = true;
				}

				if (mShield.waitingTimer >= SHIELD_SKILL_DURATION)
				{
					mShield.scale.width = SHELD_MIN_RADIUS;
					mShield.scale.height = SHELD_MIN_RADIUS;

					mShield.waitingTimer = 0.0f;

					mShield.blinkTimer = 0.0f;
					mShield.isBlinkOn = true;

					mShield.state = eShield_State::CoolTime;
				}

				break;
			}

			case eShield_State::CoolTime:
			{
				mShieldSound.Pause();

				mShield.coolTimer += deltaTime;

				if (mShield.coolTimer >= COOL_TIME)
				{
					mShield.coolTimer = 0.0f;
					mShield.state = eShield_State::End;
				}

				break;
			}

			case eShield_State::End:
			{
				mShield.labelCoolTimer = 0.0f;
				break;
			}

			default:
				break;
			}
		}

		// 플레이어 주변을 공전하는 스킬을 업데이트한다.
		{
			if (Input::Get().GetKeyDown('Q')
				and mOrbit.state == eOrbit_State::End)
			{
				mOrbitSound.Replay();

				mOrbitLabel.SetActive(true);
				mOrbit.state = eOrbit_State::Rotating;
			}

			constexpr float SPEED = 400.0f;
			constexpr float ROTATE_TIME = 4.0f;
			constexpr float COOL_TIME = 6.0f;

			mOrbit.labelCoolTimer += deltaTime;
			const uint32_t seconds = uint32_t(mOrbit.labelCoolTimer) % 60;

			// 쿨타임을 표시한다.
			if (mOrbit.state != eOrbit_State::End)
			{
				mOrbitKeyLabel.SetActive(false);
				mOrbitLabel.SetText(std::to_wstring(uint32_t(ROTATE_TIME + COOL_TIME) - seconds));
			}
			else
			{
				mOrbitKeyLabel.SetActive(true);
				mOrbitLabel.SetActive(false);
			}

			switch (mOrbit.state)
			{
			case eOrbit_State::Rotating:
			{
				mOrbit.rotatingTimer += deltaTime;
				mOrbit.angle += SPEED * deltaTime;

				constexpr float OFFSET = 160.0f;
				mOrbit.ellipse.point = { .x = 0.0f, .y = OFFSET };
				mOrbit.ellipse.point = Math::RotateVector(mOrbit.ellipse.point, -mOrbit.angle);

				// 1초 남았을 때 깜빡거린다. (총 4초 유지)
				if ((ROTATE_TIME - mOrbit.rotatingTimer) <= 1.0f)
				{
					mOrbit.blinkTimer += deltaTime;

					if (mOrbit.blinkTimer >= 0.1f)
					{
						mOrbit.isBlinkOn = !mOrbit.isBlinkOn;
						mOrbit.blinkTimer = 0.0f;
					}
				}
				else
				{
					mOrbit.isBlinkOn = true;
				}

				if (mOrbit.rotatingTimer >= ROTATE_TIME)
				{
					mOrbit.rotatingTimer = 0.0f;
					mOrbit.state = eOrbit_State::CoolTime;
				}

				break;
			}
			case eOrbit_State::CoolTime:
			{
				mOrbitSound.Pause();

				mOrbit.coolTimer += deltaTime;

				if (mOrbit.coolTimer >= COOL_TIME)
				{
					mOrbit.coolTimer = 0.0f;
					mOrbit.state = eOrbit_State::End;
				}
				break;
			}
			case eOrbit_State::End:
			{
				mOrbit.labelCoolTimer = 0.0f;
				break;
			}
			default:
				break;
			}
		}

		// 모든 몬스터를 죽일 수 있는 스킬을 업데이트한다.
		{			
			if (Input::Get().GetKeyDown('F'))
			{
				if (mKillMonsterCount >= KILL_ALL_MONSTER_COUNT)
				{
					mIsKillAllMonster = true;
				}
			}

			if (mIsKillAllMonster)
			{
				for (Monster& monster : mBigMonsters)
				{
					if (not monster.sprite.IsActive())
					{
						continue;
					}

					monster.hp = 0;
					spawnParticle(mRectParticles.data(), mRectParticles.size(), &monster, PARTICLE_PER);
				}

				for (RunMonster& run : mRunMonsters)
				{
					if (not run.monster.sprite.IsActive())
					{
						continue;
					}

					run.monster.hp = 0;
					spawnParticle(mRectParticles.data(), mRectParticles.size(), &run.monster, PARTICLE_PER);
				}

				for (SlowMonster& slow : mSlowMonsters)
				{
					if (not slow.monster.sprite.IsActive())
					{
						continue;
					}

					slow.monster.hp = 0;
					spawnParticle(mRectParticles.data(), mRectParticles.size(), &slow.monster, PARTICLE_PER);
				}

				mKillMonsterCount = 0;
				mIsKillAllMonster = false;
			}
		}
	}

	// 몬스터를 일정 시간마다 스폰한다.
	{
		// 큰 몬스터
		mBigMonsterSpawnTimer += deltaTime;
		if (mBigMonsterSpawnTimer >= 0.5f)
		{
			for (Monster& monster : mBigMonsters)
			{
				if (monster.sprite.IsActive())
				{
					continue;
				}

				spawnMonster(
					{
						.monster = &monster,
						.scale {.width = BIG_MONSTER_SCALE , .height = BIG_MONSTER_SCALE },
						.maxHp = BIG_MONSTER_MAX_HP,
						.hpBarOffset = { .x = 3.5f, .y = -10.0f }
					}
				);

				monster.moveSpeed = getRandom(10.0f, 80.0f);
				mBigMonsterSpawnTimer = 0.0f;
				break;
			}
		}

		// 돌진 몬스터
		mRunMonsterSpawnTimer += deltaTime;
		if (mRunMonsterSpawnTimer >= 2.0f)
		{
			for (RunMonster& run : mRunMonsters)
			{
				Sprite& sprite = run.monster.sprite;
				if (sprite.IsActive())
				{
					continue;
				}

				spawnMonster(
					{
						.monster = &run.monster,
						.scale {.width = RUN_MONSTER_SCALE , .height = RUN_MONSTER_SCALE },
						.maxHp = RUN_MONSTER_MAX_HP,
						.hpBarOffset = {.x = 0.0f, .y = -10.0f }
					}
				);

				// 출발바를 생성한다.
				run.isMoveable = false;
				run.startBar.SetPosition({ .x = sprite.GetPosition().x - 10.0f, .y = sprite.GetPosition().y - 20.0f });
				run.startBar.SetScale({ .width = 0.0f, .height = 0.1f });
				run.startBar.SetActive(false);

				mRunMonsterSpawnTimer = 0.0f;
				break;
			}
		}

		// 느린 몬스터
		mSlowMonsterSpawnTimer += deltaTime;
		if (mSlowMonsterSpawnTimer >= 1.0f)
		{
			for (SlowMonster& slow : mSlowMonsters)
			{
				Monster& monster = slow.monster;
				Sprite& sprite = monster.sprite;

				if (sprite.IsActive())
				{
					continue;
				}

				spawnMonster(
					{
						.monster = &monster,
						.scale {.width = SLOW_MONSTER_SCALE , .height = SLOW_MONSTER_SCALE },
						.maxHp = SLOW_MONSTER_MAX_HP,
						.hpBarOffset = {.x = 2.0f, .y = -10.0f }
					}
				);

				mSlowMonsterSpawnTimer = 0.0f;
				break;
			}
		}
	}

	// 몬스터 스폰 이펙트를 업데이트한다.
	{
		// 큰 몬스터
		for (Monster& monster : mBigMonsters)
		{
			if (monster.state != eMonster_State::Spawn)
			{
				continue;
			}

			spawnMonsterEffect(
				{
					.monster = &monster,
					.originalScale { BIG_MONSTER_SCALE, BIG_MONSTER_SCALE },
					.effectScale{ 4.0f, 4.0f },
					.time = 0.3f,
					.deltaTime = deltaTime
				}
			);
		}

		// 돌진 몬스터
		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;

			if (monster.state != eMonster_State::Spawn)
			{
				continue;
			}

			spawnMonsterEffect(
				{
					.monster = &monster,
					.originalScale { RUN_MONSTER_SCALE, RUN_MONSTER_SCALE },
					.effectScale{ 3.3f, 3.3f },
					.time = 0.5f,
					.deltaTime = deltaTime
				}
			);
		}

		// 느린 몬스터 
		{
			for (SlowMonster& slow : mSlowMonsters)
			{
				Monster& monster = slow.monster;
				if (monster.state != eMonster_State::Spawn)
				{
					continue;
				}

				spawnMonsterEffect(
					{
						.monster = &monster,
						.originalScale { SLOW_MONSTER_SCALE, SLOW_MONSTER_SCALE },
						.effectScale{ 2.0f, 2.0f },
						.time = 0.5f,
						.deltaTime = deltaTime
					}
				);
			}
		}
	}

	// 몬스터의 HpBar Active를 업데이트한다.
	{
		// 큰 몬스터
		for (Monster& monster : mBigMonsters)
		{
			if (not monster.sprite.IsActive())
			{
				continue;
			}

			if (monster.state == eMonster_State::Life
				and not monster.isHpBarActivated)
			{
				monster.backgroundHpBar.SetActive(true);
				monster.hpBar.SetActive(true);

				monster.isHpBarActivated = true;
			}
		}

		// 돌진 몬스터
		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;
			if (not monster.sprite.IsActive())
			{
				continue;
			}

			if (monster.state == eMonster_State::Life)
			{
				if (not run.isMoveable)
				{
					D2D1_SIZE_F scale = run.startBar.GetScale();
					if (scale.width < RUN_MONSTER_START_BAR_WIDTH)
					{
						run.startBar.SetActive(true);
					}
					else
					{
						run.startBar.SetActive(false);

						monster.backgroundHpBar.SetActive(true);
						monster.hpBar.SetActive(true);
					}
				}
			}
		}

		// 느린 몬스터
		for (SlowMonster& slow : mSlowMonsters)
		{
			Monster& monster = slow.monster;
			if (not monster.sprite.IsActive())
			{
				continue;
			}

			if (monster.state == eMonster_State::Life
				and not monster.isHpBarActivated)
			{
				slow.moveState = eSlow_Monster_State::Stop;
				monster.backgroundHpBar.SetActive(true);
				monster.hpBar.SetActive(true);

				monster.isHpBarActivated = true;
			}
		}
	}

	// 몬스터 이동을 업데이트한다.
	{
		// 큰 몬스터
		for (Monster& monster : mBigMonsters)
		{
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			Sprite& sprite = monster.sprite;
			D2D1_POINT_2F position = sprite.GetPosition();

			D2D1_POINT_2F direction = Math::SubtractVector({}, position);
			direction = Math::NormalizeVector(direction);
			const D2D1_POINT_2F velocity = Math::ScaleVector(direction, monster.moveSpeed * deltaTime);

			position = Math::AddVector(position, velocity);
			sprite.SetPosition(position);

			// hp를 좌표를 업데이트한다.
			{
				const D2D1_POINT_2F position = getHpBarOffset(sprite, { .x = 3.5f, .y = -10.0f });
				monster.backgroundHpBar.SetPosition(position);
				monster.hpBar.SetPosition(position);
			}
		}

		// 돌진 몬스터
		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			Sprite& sprite = monster.sprite;

			if (not run.isMoveable)
			{
				// 출발바가 꽉 차면 돌진 몬스터는 이동한다.
				constexpr float START_COOL_TIME = 2.0f;
				float barSpeed = RUN_MONSTER_START_BAR_WIDTH / START_COOL_TIME;

				D2D1_SIZE_F scale = run.startBar.GetScale();
				if (scale.width < RUN_MONSTER_START_BAR_WIDTH)
				{
					scale.width += barSpeed * deltaTime;
					run.startBar.SetScale(scale);
					continue;
				}
				else
				{
					scale.width = RUN_MONSTER_START_BAR_WIDTH;
					run.startBar.SetScale(scale);

					const D2D1_POINT_2F monsterPosition = sprite.GetPosition();
					const D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();

					monster.moveSpeed = 0.0f;

					run.direction = Math::SubtractVector(heroPosition, monsterPosition);
					run.direction = Math::NormalizeVector(run.direction);

					run.isMoveable = true;
				}
			}

			constexpr float MOVE_ACC = 5.0f;
			monster.moveSpeed = min(monster.moveSpeed + MOVE_ACC, 400.0f);
			D2D1_POINT_2F velocity = Math::ScaleVector(run.direction, monster.moveSpeed * deltaTime);

			D2D1_POINT_2F position = sprite.GetPosition();
			position = Math::AddVector(position, velocity);
			sprite.SetPosition(position);

			// hp를 좌표를 업데이트한다.
			{
				const D2D1_POINT_2F position = getHpBarOffset(sprite, { .x = 0.0f, .y = -10.0f });
				monster.backgroundHpBar.SetPosition(position);
				monster.hpBar.SetPosition(position);
			}
		}

		// 느린 몬스터
		{
			constexpr float LENGTH = 100.0f;
			constexpr float MOVE_TIME = 1.5f;
			constexpr float STOP_TIME = 1.0f;

			for (SlowMonster& slow : mSlowMonsters)
			{
				Monster& monster = slow.monster;
				Sprite& sprite = monster.sprite;

				if (monster.state != eMonster_State::Life)
				{
					continue;
				}

				// 느린 몬스터의 이동
				switch (slow.moveState)
				{
				case eSlow_Monster_State::Moving:
				{
					slow.movingTimer += deltaTime;

					float t = slow.movingTimer / MOVE_TIME;
					t = std::clamp(t, 0.0f, 1.0f);

					// 갈수록 느려지는 효과이다.
					float easeOutT = 1.0f - (1.0f - t) * (1.0f - t);

					D2D1_POINT_2F position = Math::LerpVector(slow.startPosition, slow.endPosition, easeOutT);
					sprite.SetPosition(position);

					if (easeOutT >= 1.0f)
					{
						slow.moveState = eSlow_Monster_State::Stop;
						slow.stopTimer = 0.0f;
					}

					// hp를 좌표를 업데이트한다.
					{
						const D2D1_POINT_2F position = getHpBarOffset(sprite, { .x = 2.0f, .y = -10.0f });
						monster.backgroundHpBar.SetPosition(position);
						monster.hpBar.SetPosition(position);
					}

					break;
				}

				case eSlow_Monster_State::Stop:
				{
					slow.stopTimer += deltaTime;

					if (slow.stopTimer >= STOP_TIME)
					{
						slow.movingTimer = 0.0f;
						slow.shadowCoolTimer = 0.0f;

						slow.startPosition = sprite.GetPosition();
						D2D1_POINT_2F direction = Math::SubtractVector({}, slow.startPosition);
						direction = Math::NormalizeVector(direction);
						slow.endPosition = Math::AddVector(slow.startPosition, Math::ScaleVector(direction, LENGTH));

						slow.moveState = eSlow_Monster_State::Moving;
					}

					break;
				}
				}

				slow.shadowCoolTimer -= deltaTime;

				if (slow.shadowCoolTimer <= 0.0f)
				{
					if (not monster.sprite.IsActive())
					{
						continue;
					}

					if (slow.moveState != eSlow_Monster_State::Moving)
					{
						continue;
					}

					for (Sprite& shadow : slow.shadow)
					{
						if (shadow.IsActive())
						{
							continue;
						}

						shadow.SetOpacity(1.0f);
						shadow.SetPosition(monster.sprite.GetPosition());
						shadow.SetActive(true);
						break;
					}

					slow.shadowCoolTimer = 0.02f;
				}
			}

			// 그림자 이펙트가 업데이트한다.
			for (SlowMonster& slow : mSlowMonsters)
			{
				for (Sprite& shadow : slow.shadow)
				{
					if (not shadow.IsActive())
					{
						continue;
					}

					float opacity = shadow.GetOpacity();
					opacity -= 5.0f * deltaTime;
					shadow.SetOpacity(opacity);

					if (opacity <= 0.0f)
					{
						shadow.SetActive(false);
					}
				}
			}
		}
	}

	// 총알 - 몬스터 충돌 시, 파티클을 스폰한다.
	{
		// 큰 몬스터
		for (Monster& monster : mBigMonsters)
		{
			if (not monster.isBulletColliding)
			{
				continue;
			}

			spawnParticle(mStarParticles.data(), mStarParticles.size(), &monster, PARTICLE_PER);

			if (monster.hp <= 0)
			{
				++mKillMonsterCount;
			}

			monster.isBulletColliding = false;
		}

		// 돌진 몬스터
		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;
			if (not monster.isBulletColliding)
			{
				continue;
			}

			spawnParticle(mStarParticles.data(), mStarParticles.size(), &monster, PARTICLE_PER);
			++mKillMonsterCount;
			monster.isBulletColliding = false;
		}

		// 느린 몬스터
		for (SlowMonster& slow : mSlowMonsters)
		{
			Monster& monster = slow.monster;
			if (not monster.isBulletColliding)
			{
				continue;
			}

			spawnParticle(mStarParticles.data(), mStarParticles.size(), &monster, PARTICLE_PER);
			++mKillMonsterCount;
			monster.isBulletColliding = false;
		}
	}

	// 쉴드 스킬 - 큰 몬스터 충돌 시, Effect를 스폰한다.
	{
		// 큰 몬스터
		for (Monster& monster : mBigMonsters)
		{
			if (not monster.isShieldColliding)
			{
				continue;
			}

			spawnLongEffect(mLongEffect.data(), mLongEffect.size(), &mSkyBlueRectangleTexture, monster);
			++mKillMonsterCount;
			monster.isShieldColliding = false;
		}

		// 돌진 몬스터
		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;
			if (not monster.isShieldColliding)
			{
				continue;
			}

			spawnDiamondEffect(mCyanEffect.data(), mCyanEffect.size(), monster);
			++mKillMonsterCount;
			monster.isShieldColliding = false;
		}

		// 느린 몬스터
		for (SlowMonster& slow : mSlowMonsters)
		{
			Monster& monster = slow.monster;
			if (not monster.isShieldColliding)
			{
				continue;
			}

			spawnDiamondEffect(mGreenEffect.data(), mGreenEffect.size(), monster);
			++mKillMonsterCount;
			monster.isShieldColliding = false;
		}
	}

	// 공전 스킬 - 큰 몬스터 충돌 시, Effect를 스폰한다.
	{
		// 큰 몬스터
		for (Monster& monster : mBigMonsters)
		{
			if (not monster.isOrbitColliding)
			{
				continue;
			}

			spawnLongEffect(mLongEffect.data(), mLongEffect.size() , & mBlueRectangleTexture, monster);
			++mKillMonsterCount;
			monster.isOrbitColliding = false;
		}

		// 돌진 몬스터
		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;
			if (not monster.isOrbitColliding)
			{
				continue;
			}

			spawnDiamondEffect(mCyanEffect.data(), mCyanEffect.size(), monster);
			++mKillMonsterCount;
			monster.isOrbitColliding = false;
		}

		// 느린 몬스터
		for (SlowMonster& slow : mSlowMonsters)
		{
			Monster& monster = slow.monster;
			if (not monster.isOrbitColliding)
			{
				continue;
			}

			spawnDiamondEffect(mGreenEffect.data(), mGreenEffect.size(), monster);
			++mKillMonsterCount;
			monster.isOrbitColliding = false;
		}
	}

	// 체력바와 수명을 업데이트한다.
	{
		// 큰 몬스터
		for (Monster& monster : mBigMonsters)
		{
			// 체력바를 업데이트한다.
			updateMonsterHp(&monster, BIG_MONSTER_HP_BAR_WIDTH, BIG_MONSTER_MAX_HP, deltaTime);

			// 큰 몬스터가 죽으면 이펙트가 생성된다.
			if (monster.hp <= 0
				and monster.state == eMonster_State::Life)
			{
				monster.state = eMonster_State::Dead;
				mBigMonsterDeadSound.Replay();
			}

			deadMonsterEffect(
				{
					.monster = &monster,
					.originalScale = {.width = BIG_MONSTER_SCALE, .height = BIG_MONSTER_SCALE },
					.effectScale = {.width = 0.1f, .height = 0.1f },
					.time = 0.5f,
					.deltaTime = deltaTime
				}
			);
		}

		// 돌진 몬스터
		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;

			// 체력바를 업데이트한다.
			updateMonsterHp(&monster, RUN_MONSTER_HP_BAR_WIDTH, RUN_MONSTER_MAX_HP, deltaTime);

			// 몬스터가 죽으면 이펙트가 생성된다.
			if (monster.hp <= 0
				and monster.state == eMonster_State::Life)
			{
				monster.state = eMonster_State::Dead;
				mRunMonsterDeadSound.Replay();

				run.startBar.SetActive(false);
			}

			deadMonsterEffect(
				{
					.monster = &monster,
					.originalScale = {.width = RUN_MONSTER_SCALE, .height = RUN_MONSTER_SCALE },
					.effectScale = {.width = 0.1f, .height = 0.1f },
					.time = 0.4f,
					.deltaTime = deltaTime
				}
			);
		}

		// 느린 몬스터
		for (SlowMonster& slow : mSlowMonsters)
		{
			Monster& monster = slow.monster;

			// 체력바를 업데이트한다.
			updateMonsterHp(&monster, SLOW_MONSTER_HP_BAR_WIDTH, SLOW_MONSTER_MAX_HP, deltaTime);

			// 몬스터가 죽으면 이펙트가 생성된다.
			if (monster.hp <= 0
				and monster.state == eMonster_State::Life)
			{
				monster.state = eMonster_State::Dead;
				mSlowMonsterDeadSound.Replay();

				for (Sprite& shadow : slow.shadow)
				{
					shadow.SetActive(false);
				}
			}

			deadMonsterEffect(
				{
					.monster = &monster,
					.originalScale = {.width = SLOW_MONSTER_SCALE, .height = SLOW_MONSTER_SCALE },
					.effectScale = {.width = 0.1f, .height = 0.1f },
					.time = 0.7f,
					.deltaTime = deltaTime
				}
			);
		}
	}

	// 파티클을 업데이트한다.
	updateParticle(mStarParticles.data(), STAR_PARTICLE_COUNT, deltaTime);
	updateParticle(mRectParticles.data(), RECT_PARTICLE_COUNT, deltaTime);

	// 이펙트를 업데이트한다.
	{
		// Update Long Effect
		updateLongEffect
		(
			{
				.sprite = mLongEffect.data(),
				.size = LONG_EFFECT_COUNT,
				.timer = mLongEffectTimer,
				.time = 0.5f,
				.scale = { LONG_EFFECT_SCALE.width, LONG_EFFECT_SCALE.height},
				.deltaTime = deltaTime
			}
		);
		
		// Update Cyan Effect
		updateDiamondEffect
		(
			{
				.effect = mCyanEffect.data(),
				.size = CYAN_EFFECT_COUNT,
				.scale = { .width = 80.0f, .height = 80.0f },
				.speed = 3.0f,
				.time = 0.4f,
				.thick = { .x = 50.0f, .y = 50.0f },
				.deltaTime = deltaTime
			}
		);

		// Update Green Effect
		updateDiamondEffect
		(
			{
				.effect = mGreenEffect.data(),
				.size = GREEN_EFFECT_COUNT,
				.scale = {.width = 70.0f, .height = 70.0f },
				.speed = 3.0f,
				.time = 0.7f,
				.thick = {.x = 50.0f, .y = 50.0f },
				.deltaTime = deltaTime
			}
		);
	}

	// 플레이어 체력에 관련된 부분을 업데이트한다.
	{
		// 플레이어가 죽었을 때 종료된다.
		if (mHero.hp <= 0)
		{
			mBackgroundSound.Pause();
			mDashSound.Pause();
			mBulletSound.Pause();
			mReloadSound.Pause();
			mBigMonsterDeadSound.Pause();
			mRunMonsterDeadSound.Pause();
			mSlowMonsterDeadSound.Pause();
			mShieldSound.Pause();
			mOrbitSound.Pause();

			mHero.hp = 0;
			mHero.velocity = {};
			mHero.sprite.SetPosition({});
			mHero.sprite.SetActive(false);

			mDash.isActive = false;
			mDash.moveSpeed = 0.0f;
			mDash.count = 0;

			misKeyDownReload = false;
			mBulletValue = 0.0f;

			mShield.state = eShield_State::End;
			mOrbit.state = eOrbit_State::End;

			mGameOverSound.Play();
		}

		// UI 버튼 관련
		{
			// 플레이어가 죽었을 때 딱 한 번 실행된다.
			if (not mIsGameOver
				and mHero.hp <= 0)
			{
				mGameOver.SetActive(true);

				mGameOverTimer += deltaTime;

				if (mGameOverTimer >= 2.0f)
				{
					mIsGameOver = true;
					mGameOver.SetActive(false);

					Input::Get().SetCursorVisible(true);
					mZoom.SetActive(false);

					mGameOverBackground.SetActive(true);
					mResumeButton.SetActive(true);
					mExitButton.SetActive(true);

					mGameOverTimer = 0.0f;
				}
			}

			// UI 버튼을 업데이트한다.
			if (mIsGameOver)
			{
				updateButtonState
				(
					{
						.sprite = &mResumeButton,
						.isColliding = mIsResumeButtonColliding,
						.originalTexture = &mResumeIdleButtonTexture,
						.effectTexture = &mResumeContactButtonTexture,
						.isSoundPlay = &mIsResumeButtonSoundPlay
					}
				);

				updateButtonState
				(
					{
						.sprite = &mExitButton,
						.isColliding = mIsExitButtonColliding,
						.originalTexture = &mExitIdleButtonTexture,
						.effectTexture = &mExitContactButtonTexture,
						.isSoundPlay = &mIsExitButtonSoundPlay
					}
				);

				if (mIsResumeButtonColliding)
				{
					if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
					{
						mIsUpdate = false;
					}
				}

				if (mIsExitButtonColliding)
				{
					if (Input::Get().GetMouseButtonDown(Input::eMouseButton::Left))
					{
						PostQuitMessage(0);
					}
				}
			}
		}

		// 플레이어 체력 라벨을 업데이트 한다.
		if (mHero.prevHp != mHero.hp)
		{
			mHeroHitSound.Replay();
			mHpValueLabel.SetText(L"Hp: " + std::to_wstring(mHero.hp) + L" / " + std::to_wstring(HERO_MAX_HP));
			mHero.isHitEffect = true;

			mHero.prevHp = mHero.hp;
		}

		// 플레이어의 Hit Effect를 업데이트한다.
		{
			float opacity = mHero.sprite.GetOpacity();
			if (mHero.isHitEffect)
			{
				opacity -= 3.0f * deltaTime;

				if (opacity <= 0.0f)
				{
					opacity = 0.0f;
					mHero.isHitEffect = false;
				}
			}
			else
			{
				opacity += 3.0f * deltaTime;

				if (opacity >= 1.0f)
				{
					opacity = 1.0f;
				}
			}

			mHero.sprite.SetOpacity(opacity);

			// 외부, 내부 원과 충돌되면 플레이어는 색상이 잠시 바뀐다.
			if (mHero.isHitBoundry)
			{
				mHero.sprite.SetTexture(&mRedRectangleTexture);

				if (opacity <= 0.0f)
				{
					mHero.isHitBoundry = false;
				}
			}
			else
			{
				mHero.sprite.SetTexture(&mPinkRectangleTexture);
			}
		}

		// 플레이어 체력바를 업데이트한다.
		D2D1_POINT_2F scale = { mUiHpBar.GetScale().width, mUiHpBar.GetScale().height };
		float targetWidth = UI_HP_SCALE_WIDTH * (float(mHero.hp) / float(HERO_MAX_HP));
		scale = Math::LerpVector(scale, { .x = targetWidth, .y = scale.y }, 10.0f * deltaTime);
		mUiHpBar.SetScale({ scale.x, scale.y });
	}

	// F 스킬바를 업데이트한다.
	{
		DEBUG_LOG("%f", mUiKillCountBar.GetScale().width);
		D2D1_POINT_2F scale = { mUiKillCountBar.GetScale().width, mUiKillCountBar.GetScale().height };
		float targetWidth = UI_KILL_SCALE_WIDTH * (float(mKillMonsterCount) / float(KILL_ALL_MONSTER_COUNT));
		scale = Math::LerpVector(scale, { .x = targetWidth, .y = scale.y }, 20.0f * deltaTime);
		mUiKillCountBar.SetScale({ scale.x, scale.y });

		mKillAllMonsterLabel.SetText(L"F: " + std::to_wstring(mKillMonsterCount) + L" / " + std::to_wstring(KILL_ALL_MONSTER_COUNT));

		if (mKillMonsterCount >= KILL_ALL_MONSTER_COUNT)
		{
			mKillMonsterCount = 10;
		}
	}

	// 플레이어 대쉬바를 업데이트한다.
	{
		static int32_t prevDashCount = mDash.count;

		if (prevDashCount != mDash.count)
		{
			mDashValueLabel.SetText(L"Dash: " + std::to_wstring(mDash.count) + L" / " + std::to_wstring(DASH_MAX_COUNT));

			prevDashCount = mDash.count;
		}

		D2D1_POINT_2F scale = { mUiDashBar.GetScale().width, mUiDashBar.GetScale().height };
		scale = Math::LerpVector(scale, { UI_DASH_SCALE_WIDTH * (float(mDash.count) / float(DASH_MAX_COUNT)),  scale.y }, 8.0f * deltaTime);
		mUiDashBar.SetScale({ scale.x, scale.y });
	}

	// 총알 라벨을 업데이트한다.
	{
		static int32_t prevBulletCount = mBulletValue;

		if (prevBulletCount != mBulletValue)
		{
			mBulletLabel.SetText(std::to_wstring(mBulletValue) + L"/" + std::to_wstring(BULLET_COUNT));

			prevBulletCount = mBulletValue;
		}
	}

	// 타이머 라벨을 업데이트한다.
	{
		if (mHero.hp > 0)
		{
			mGameTimer += deltaTime;
		}

		const uint32_t seconds = uint32_t(mGameTimer) % 60;
		const uint32_t minutes = uint32_t(mGameTimer) / 60;

		std::wstring name = L"Timer: " + std::to_wstring(minutes) + L":" + std::to_wstring(seconds);
		mTimerLabel.SetText(name);
	}

	// 충돌 처리를 업데이트한다.
	{		
		// Resume Button
		if (Collision::IsCollidedSqureWithPoint(getRectangleFromSprite(mResumeButton, mResumeIdleButtonTexture), getMouseWorldPosition()))
		{
			mIsResumeButtonColliding = true;
		}
		else
		{
			mIsResumeButtonColliding = false;
		}

		// Exit Button
		if (Collision::IsCollidedSqureWithPoint(getRectangleFromSprite(mExitButton, mExitIdleButtonTexture), getMouseWorldPosition()))
		{
			mIsExitButtonColliding = true;
		}
		else
		{
			mIsExitButtonColliding = false;
		}

		// 플레이어 - 외부 원
		{
			if (not Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mHero.sprite), {}, BOUNDARY_RADIUS))
			{
				mHero.velocity = {};

				D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
				const D2D1_POINT_2F direction = Math::NormalizeVector(heroPosition);

				heroPosition = Math::AddVector(heroPosition, Math::ScaleVector(direction, -2.0f));
				mHero.sprite.SetPosition(heroPosition);
			}
		}

		// 플레이어 - 내부 원
		{
			const float offset = IN_BOUNDARY_RADIUS + mHero.sprite.GetScale().height * mPinkRectangleTexture.GetHeight();
			if (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(mHero.sprite), {}, offset))
			{
				mHero.velocity = {};

				D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
				const D2D1_POINT_2F direction = Math::NormalizeVector(heroPosition);

				heroPosition = Math::AddVector(heroPosition, Math::ScaleVector(direction, 2.0f));
				mHero.sprite.SetPosition(heroPosition);
			}
		}

		// 총알과 원이 충돌한다.
		for (Bullet& bullet : mBullets)
		{
			Sprite& sprite = bullet.sprite;

			const float halfLength = sprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f;
			const D2D1_POINT_2F endPosition =
			{
				.x = sprite.GetPosition().x + bullet.direction.x * halfLength,
				.y = sprite.GetPosition().y + bullet.direction.y * halfLength
			};

			Line line =
			{
				.Point0 = bullet.prevPosition,
				.Point1 = endPosition
			};

			bool isBoundryToBullet = Collision::IsCollidedCircleWithLine({}, BOUNDARY_RADIUS, line);
			if (not isBoundryToBullet)
			{
				sprite.SetActive(false);
			}

			const float offset = IN_BOUNDARY_RADIUS + sprite.GetScale().height * mRedRectangleTexture.GetHeight();
			bool isInBoundryToBullet = Collision::IsCollidedCircleWithLine({}, offset, line);
			if (isInBoundryToBullet)
			{
				sprite.SetActive(false);
			}
		}

		for (Monster& monster : mBigMonsters)
		{
			Sprite& sprite = monster.sprite;
			if (not sprite.IsActive())
			{
				continue;
			}

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			// 몬스터 - 내부 바운더리
			{
				const float offset = IN_BOUNDARY_RADIUS + monster.sprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f;
				const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), {}, offset);

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= BIG_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
					mHero.isHitBoundry = true;
				}
			}

			// 몬스터 - 플레이어
			{
				const bool isCollision = Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero.sprite), getRectangleFromSprite(monster.sprite));
				if (isCollision and monster.hp > 0)
				{
					monster.hp -= BIG_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}
		}

		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;
			Sprite& sprite = monster.sprite;
			if (not sprite.IsActive())
			{
				continue;
			}

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			// 돌진 몬스터 - 내부 바운더리
			{
				const float offset = IN_BOUNDARY_RADIUS + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
				const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), {}, offset);

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= RUN_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
					mHero.isHitBoundry = true;
				}
			}

			// 돌진 몬스터 - 외부 바운더리
			{
				const bool isCollision = not (Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), {}, BOUNDARY_RADIUS));

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= RUN_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
					mHero.isHitBoundry = true;
				}
			}

			// 몬스터 - 플레이어
			{
				const bool isCollision = Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero.sprite), getRectangleFromSprite(monster.sprite));

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= RUN_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}
		}

		for (SlowMonster& slow : mSlowMonsters)
		{
			Monster& monster = slow.monster;
			Sprite& sprite = monster.sprite;

			if (not sprite.IsActive())
			{
				continue;
			}

			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			// 느린 몬스터 - 내부 바운더리
			{
				const float offset = IN_BOUNDARY_RADIUS + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
				const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), {}, offset);

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= SLOW_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
					mHero.isHitBoundry = true;
				}
			}

			// 느린 몬스터 - 플레이어
			{
				const bool isCollision = Collision::IsCollidedSqureWithSqure(getRectangleFromSprite(mHero.sprite), getRectangleFromSprite(monster.sprite));

				if (isCollision and monster.hp > 0)
				{
					monster.hp -= SLOW_MONSTER_MAX_HP;
					mHero.hp -= MONSTER_ATTACK_VALUE;
				}
			}
		}

		// 총알과 모든 몬스터 충돌을 검사한다.
		for (Bullet& bullet : mBullets)
		{
			Sprite& bulletSprite = bullet.sprite;
			if (not bulletSprite.IsActive())
			{
				continue;
			}

			// 이전 좌표와 현재 좌표의 직선을 그려서 충돌체크를 한다.
			const float halfLength = bulletSprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f;
			const D2D1_POINT_2F endPosition =
			{
				.x = bulletSprite.GetPosition().x + bullet.direction.x * halfLength,
				.y = bulletSprite.GetPosition().y + bullet.direction.y * halfLength
			};

			const Line line =
			{
				.Point0 = bullet.prevPosition,
				.Point1 = endPosition
			};

			// 총알과 기본 몬스터 충돌을 검사한다.
			Sprite* targetMonster = nullptr;
			float targetMonsterDistance = 999.9f;

			for (Monster& monster : mBigMonsters)
			{
				if (monster.state != eMonster_State::Life)
				{
					continue;
				}

				Sprite& sprite = monster.sprite;
				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(sprite), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(bullet.prevPosition, sprite.GetPosition()));
				if (distance < targetMonsterDistance)
				{
					monster.hp -= BULLET_ATTACK_VALUE;
					monster.isBulletColliding = true;

					targetMonster = &sprite;
					targetMonsterDistance = distance;
				}
			}

			// 돌진 몬스터를 계산한다.
			Sprite* targetRunMonster = nullptr;
			float targetRunMonsterDistance = 999.9f;

			for (RunMonster& run : mRunMonsters)
			{
				Monster& monster = run.monster;
				Sprite& sprite = monster.sprite;

				if (not sprite.IsActive())
				{
					continue;
				}

				if (monster.state != eMonster_State::Life)
				{
					continue;
				}

				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(sprite), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(bullet.prevPosition, sprite.GetPosition()));
				if (distance < targetRunMonsterDistance)
				{
					monster.hp -= BULLET_ATTACK_VALUE;
					monster.isBulletColliding = true;

					targetRunMonster = &sprite;
					targetRunMonsterDistance = distance;
				}
			}

			// 느린 몬스터를 계산한다
			Sprite* targetSlowMonster = nullptr;
			float targetSlowMonsterDistance = 999.9f;

			for (SlowMonster& slow : mSlowMonsters)
			{
				Monster& monster = slow.monster;
				Sprite& sprite = monster.sprite;

				if (not sprite.IsActive())
				{
					continue;
				}

				if (monster.state != eMonster_State::Life)
				{
					continue;
				}

				if (not Collision::IsCollidedSqureWithLine(getRectangleFromSprite(sprite), line))
				{
					continue;
				}

				const float distance = Math::GetVectorLength(Math::SubtractVector(bullet.prevPosition, sprite.GetPosition()));
				if (distance < targetMonsterDistance)
				{
					monster.hp -= BULLET_ATTACK_VALUE;
					monster.isBulletColliding = true;

					targetSlowMonster = &sprite;
					targetSlowMonsterDistance = distance;
				}
			}

			if (targetMonster != nullptr
				or targetRunMonster != nullptr
				or targetSlowMonster != nullptr)
			{
				bullet.sprite.SetActive(false);
			}
		}

		// 플레이어 쉴드와 몬스터가 충돌하면 몬스터는 삭제된다.
		for (Monster& monster : mBigMonsters)
		{
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			if (mShield.state != eShield_State::Growing
				and mShield.state != eShield_State::Waiting)
			{
				continue;
			}

			const float offset = mShield.scale.width * 0.5f + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), mHero.sprite.GetPosition(), offset);
			
			if (isCollision and monster.hp > 0)
			{
				monster.isShieldColliding = true;
				monster.hp -= BIG_MONSTER_MAX_HP;
			}
		}

		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			if (mShield.state != eShield_State::Growing
				and mShield.state != eShield_State::Waiting)
			{
				continue;
			}

			const float offset = mShield.scale.width * 0.5f + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), mHero.sprite.GetPosition(), offset);
			
			if (isCollision and monster.hp > 0)
			{
				monster.isShieldColliding = true;
				monster.hp -= RUN_MONSTER_MAX_HP;
			}
		}

		for (SlowMonster& slow : mSlowMonsters)
		{
			Monster& monster = slow.monster;
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			if (mShield.state != eShield_State::Growing
				and mShield.state != eShield_State::Waiting)
			{
				continue;
			}

			const float offset = mShield.scale.width * 0.5f + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), mHero.sprite.GetPosition(), offset);
			
			if (isCollision and monster.hp > 0)
			{
				monster.isShieldColliding = true;
				monster.hp -= SLOW_MONSTER_MAX_HP;
			}
		}

		// 플레이어 주변을 공전하는 원과 몬스터가 충돌하면 몬스터는 삭제된다.
		for (Monster& monster : mBigMonsters)
		{
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			if (mOrbit.state != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::SubtractVector(mHero.sprite.GetPosition(), mOrbit.ellipse.point);
			const float radius = mOrbit.ellipse.radiusX + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), center, radius);

			if (isCollision and monster.hp > 0)
			{
				monster.isOrbitColliding = true;
				monster.hp -= BIG_MONSTER_MAX_HP;
			}
		}

		for (RunMonster& run : mRunMonsters)
		{
			Monster& monster = run.monster;
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			if (mOrbit.state != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::SubtractVector(mHero.sprite.GetPosition(), mOrbit.ellipse.point);
			const float radius = mOrbit.ellipse.radiusX + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), center, radius);
			
			if (isCollision and monster.hp > 0)
			{
				monster.isOrbitColliding = true;
				monster.hp -= RUN_MONSTER_MAX_HP;
			}
		}

		for (SlowMonster& slow : mSlowMonsters)
		{
			Monster& monster = slow.monster;
			if (monster.state != eMonster_State::Life)
			{
				continue;
			}

			if (mOrbit.state != eOrbit_State::Rotating)
			{
				continue;
			}

			const D2D1_POINT_2F center = Math::SubtractVector(mHero.sprite.GetPosition(), mOrbit.ellipse.point);
			const float radius = mOrbit.ellipse.radiusX + monster.sprite.GetScale().height * mRectangleTexture.GetHeight();
			const bool isCollision = Collision::IsCollidedSqureWithCircle(getRectangleFromSprite(monster.sprite), center, radius);

			if (isCollision and monster.hp > 0)
			{
				monster.isOrbitColliding = true;
				monster.hp -= SLOW_MONSTER_MAX_HP;
			}
		}
	}

	// 카메라를 업데이트한다.
	{
		D2D1_POINT_2F position = mMainCamera.GetPosition();
		const D2D1_POINT_2F heroPosition = mHero.sprite.GetPosition();
		position = Math::LerpVector(position, heroPosition, 8.0f * deltaTime);

		if (mCameraShakeAmplitude > 0.0f)
		{
			const D2D1_POINT_2F offset = updateCameraShake(deltaTime);
			position = Math::AddVector(position, offset);
		}

		mMainCamera.SetPosition(position);
	}

	return mIsUpdate;
}

void MainScene::PostDraw(const D2D1::Matrix3x2F& view, const D2D1::Matrix3x2F& viewForUI)
{
	ID2D1HwndRenderTarget* renderTarget = GetHelper()->GetRenderTarget();

	// Hero 쉴드 스킬을 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix(mHero.sprite.GetPosition()) * view;
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse =
		{
			.radiusX = mShield.scale.width * 0.5f,
			.radiusY = mShield.scale.height * 0.5f
		};

		if (mShield.state == eShield_State::Growing or mShield.state == eShield_State::Waiting
			and mShield.isBlinkOn)
		{
			renderTarget->DrawEllipse(ellipse, mYellowBrush, 10.0f);
		}
	}

	// Hero 공전 스킬을 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix(mHero.sprite.GetPosition()) * view;
		renderTarget->SetTransform(worldView);

		if (mOrbit.state == eOrbit_State::Rotating
			and mOrbit.isBlinkOn)
		{
			renderTarget->DrawEllipse(mOrbit.ellipse, mOrange, 5.0f);
		}
	}

	// CYAN 이펙트를 그린다.
	{
		for (const DiamondEffect& effect : mCyanEffect)
		{
			if (not effect.isActive)
			{
				continue;
			}

			drawDiamondEffect
			(
				{
					.effect = effect, 
					.positionOffset = {.x = 0.0f, .y = 40.0f }, 
					.angle = 45.0f, 
					.renderTarget = renderTarget, 
					.brush = mCyanBrush, 
					.view = view
				}
			);
		}
	}

	// Green 이펙트를 그린다.
	{
		for (const DiamondEffect& effect : mGreenEffect)
		{
			if (not effect.isActive)
			{
				continue;
			}

			drawDiamondEffect
			(
				{
					.effect = effect,
					.positionOffset = {.x = 0.0f, .y = 50.0f },
					.angle = 45.0f,
					.renderTarget = renderTarget,
					.brush = mDarkGreen,
					.view = view
				}
			);
		}
	}
#ifdef _DEBUG
	// 몬스터 충돌박스를 그린다.
	{
		for (Monster& monster : mBigMonsters)
		{
			Sprite& sprite = monster.sprite;
			if (mIsColliderKeyDown and sprite.IsActive())
			{
				const Matrix3x2F worldView = Transformation::getWorldMatrix(
					{
						.x = getRectangleFromSprite(sprite).left,
						.y = getRectangleFromSprite(sprite).top
					}) * view;
				renderTarget->SetTransform(worldView);

				const D2D1_SIZE_F scale = sprite.GetScale();

				const D2D1_RECT_F colliderSize =
				{
					.left = 0.0f,
					.top = 0.0f,
					.right = scale.width * mRectangleTexture.GetWidth(),
					.bottom = scale.width * mRectangleTexture.GetWidth()
				};

				renderTarget->DrawRectangle(colliderSize, mCyanBrush);
			}

			for (RunMonster& run : mRunMonsters)
			{
				Sprite& sprite = run.monster.sprite;
				if (mIsColliderKeyDown and sprite.IsActive())
				{
					const Matrix3x2F worldView = Transformation::getWorldMatrix(
						{
							.x = getRectangleFromSprite(sprite).left,
							.y = getRectangleFromSprite(sprite).top
						}) * view;
					renderTarget->SetTransform(worldView);

					const D2D1_SIZE_F scale = sprite.GetScale();

					const D2D1_RECT_F colliderSize =
					{
						.left = 0.0f,
						.top = 0.0f,
						.right = scale.width * mRectangleTexture.GetWidth(),
						.bottom = scale.width * mRectangleTexture.GetWidth()
					};

					renderTarget->DrawRectangle(colliderSize, mCyanBrush);
				}
			}

			for (SlowMonster& slow : mSlowMonsters)
			{
				Sprite& sprite = slow.monster.sprite;
				if (mIsColliderKeyDown and sprite.IsActive())
				{
					const Matrix3x2F worldView = Transformation::getWorldMatrix(
						{
							.x = getRectangleFromSprite(sprite).left,
							.y = getRectangleFromSprite(sprite).top
						}) * view;
					renderTarget->SetTransform(worldView);

					const D2D1_SIZE_F scale = sprite.GetScale();

					const D2D1_RECT_F colliderSize =
					{
						.left = 0.0f,
						.top = 0.0f,
						.right = scale.width * mRectangleTexture.GetWidth(),
						.bottom = scale.width * mRectangleTexture.GetWidth()
					};

					renderTarget->DrawRectangle(colliderSize, mCyanBrush);
				}
			}
		}

		// 총알 충돌박스를 그린다.
		{
			for (Bullet& bullet : mBullets)
			{
				Sprite& sprite = bullet.sprite;
				if (mIsColliderKeyDown and sprite.IsActive())
				{
					const Matrix3x2F worldView = Transformation::getWorldMatrix(getCircleFromSprite(sprite).point) * view;
					renderTarget->SetTransform(worldView);

					const D2D1_SIZE_F scale = sprite.GetScale();

					const D2D1_ELLIPSE circleSize =
					{
						.radiusX = scale.width * mCircleTexture.GetWidth() * 0.5f,
						.radiusY = scale.height * mCircleTexture.GetHeight() * 0.5f
					};

					renderTarget->DrawEllipse(circleSize, mYellowBrush);
				}
			}
		}
	}
#endif

	// 쉴드 스킬 UI를 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix
		(
			{
				.x = float(Constant::Get().GetWidth()) * 0.5f + 520.0f,
				.y = 60.0f
			}
		);
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse{ .radiusX = 25.0f, .radiusY = 25.0f };
		renderTarget->DrawEllipse(ellipse, mYellowBrush, 5.0f);
	}

	// 공전 스킬 UI를 그린다.
	{
		const Matrix3x2F worldView = Transformation::getWorldMatrix
		(
			{
				.x = float(Constant::Get().GetWidth()) * 0.5f + 590.0f,
				.y = 62.0f
			}
		);
		renderTarget->SetTransform(worldView);

		const D2D1_ELLIPSE ellipse{ .radiusX = 25.0f, .radiusY = 25.0f };
		renderTarget->DrawEllipse(ellipse, mOrange, 5.0f);
	}
}

void MainScene::Finalize()
{
	RELEASE_D2D1(mCyanBrush);
	RELEASE_D2D1(mYellowBrush);
	RELEASE_D2D1(mCyanBrush);
	RELEASE_D2D1(mDarkGreen);
	RELEASE_D2D1(mDefaultBrush);

	mRectangleTexture.Finalize();
	mRedRectangleTexture.Finalize();
	mYellowRectangleTexture.Finalize();
	mSkyBlueRectangleTexture.Finalize();
	mBlueRectangleTexture.Finalize();
	mPinkRectangleTexture.Finalize();
	mPurpleRectangleTexture.Finalize();
	mBlackRectangleTexture.Finalize();

	mCircleTexture.Finalize();
	mRedCircleTexture.Finalize();

	mWhiteBarTexture.Finalize();
	mRedBarTexture.Finalize();
	mYellowBarTexture.Finalize();

	mRedStarTexture.Finalize();
	mOrangeStarTexture.Finalize();
	mYellowStarTexture.Finalize();
	mGreenStarTexture.Finalize();
	mBlueStarTexture.Finalize();
	mPurpleStarTexture.Finalize();

	mExitIdleButtonTexture.Finalize();
	mExitContactButtonTexture.Finalize();

	// Sound
	mBackgroundSound.Finalize();

	mHeroHitSound.Finalize();
	mDashSound.Finalize();

	mBulletSound.Finalize();
	mReloadSound.Finalize();

	mShieldSound.Finalize();
	mOrbitSound.Finalize();

	mBigMonsterDeadSound.Finalize();
	mRunMonsterDeadSound.Finalize();
	mSlowMonsterDeadSound.Finalize();

	mButtonSound.Finalize();
	mGameOverSound.Finalize();
}

D2D1_RECT_F MainScene::getRectangleFromSprite(const Sprite& sprite)
{
	const D2D1_SIZE_F scale = sprite.GetScale();

	const D2D1_SIZE_F offset =
	{
		.width = scale.width * mRectangleTexture.GetWidth() * 0.5f,
		.height = scale.height * mRectangleTexture.GetHeight() * 0.5f
	};

	const D2D1_POINT_2F position = sprite.GetPosition();

	const D2D1_RECT_F rect =
	{
		.left = position.x - offset.width,
		.top = position.y + offset.height,
		.right = position.x + offset.width,
		.bottom = position.y - offset.height
	};

	return rect;
}

D2D1_RECT_F MainScene::getRectangleFromSprite(const Sprite& sprite, Texture& texture)
{

	const D2D1_SIZE_F scale = sprite.GetScale();

	const D2D1_SIZE_F offset =
	{
		.width = scale.width * texture.GetWidth() * 0.5f,
		.height = scale.height * texture.GetHeight() * 0.5f
	};

	const D2D1_POINT_2F position = sprite.GetPosition();

	const D2D1_RECT_F rect =
	{
		.left = position.x - offset.width,
		.top = position.y + offset.height,
		.right = position.x + offset.width,
		.bottom = position.y - offset.height
	};

	return rect;
}

D2D1_ELLIPSE MainScene::getCircleFromSprite(const Sprite& sprite)
{
	const D2D1_RECT_F rect = getRectangleFromSprite(sprite);
	const D2D1_POINT_2F position = sprite.GetPosition();

	const D2D1_ELLIPSE circle =
	{
		.point = position,
		.radiusX = (rect.right - position.x),
		.radiusY = rect.top - position.y
	};

	return circle;
}

float MainScene::getRandom(const float min, const float max)
{
	const float result = float(rand()) / RAND_MAX * (max - min) + min;
	return result;
}

uint32_t MainScene::getRandom(const uint32_t min, const uint32_t max)
{
	const uint32_t result = rand() % (max - min + 1) + min;
	return result;
}

D2D1_POINT_2F MainScene::getMouseWorldPosition() const
{
	const D2D1_POINT_2F zoomPosition = mZoom.GetPosition();
	const D2D1_POINT_2F cameraPosition = mMainCamera.GetPosition();

	const D2D1_POINT_2F result = Math::AddVector(zoomPosition, cameraPosition);

	return result;
}

void MainScene::initializeCameraShake(const float amplitude, const float duration, const float frequency)
{
	mCameraShakeTime = 0.0f;
	mCameraShakeTimer = 0.0f;

	mCameraShakeAmplitude = amplitude;	
	mCameraShakeDuration = duration;	
	mCameraShakeFrequency = frequency;
}

D2D1_POINT_2F MainScene::updateCameraShake(const float deltaTime)
{
	mCameraShakeTime += deltaTime;
	if (mCameraShakeTime >= mCameraShakeDuration)
	{
		initializeCameraShake(0.0f, 0.0f, 0.0f);

		return D2D1_POINT_2F{};
	}

	mCameraShakeTimer += deltaTime;

	if (const float period = (1.0f / mCameraShakeFrequency);
		mCameraShakeTimer >= period)
	{
		float progress = mCameraShakeTime / mCameraShakeDuration;
		float strength = std::pow(1.0f - progress, 2.0f) * mCameraShakeAmplitude;

		const float radian = getRandom(0.0f, 2.0f * Math::PI);
		D2D1_POINT_2F direction = { .x = cos(radian) * 1.2f, .y = sin(radian) * 0.8f };

		mCameraShakeTimer = 0.0f;

		D2D1_POINT_2F offset = Math::ScaleVector(direction, strength);
		return offset;
	}

	return D2D1_POINT_2F{};
}

void MainScene::spawnMonster(const MonsterSpawnDesc& desc)
{
	Monster* monster = desc.monster;
	const D2D1_SIZE_F scale = desc.scale;
	const int32_t maxHp = desc.maxHp;
	const D2D1_POINT_2F hpBarOffset = desc.hpBarOffset;

	// 초기 정보를 세팅한다.
	{
		monster->state = eMonster_State::Spawn;
		monster->spawnState = eSpawnEffect_State::None;
		monster->hp = maxHp;
		monster->isHpBarActivated = false;
	}

	// 초기 Sprite를 세팅한다.
	{
		constexpr float MIN_ANGLE = 0.0f;
		constexpr float MAX_ANGLE = 2.0f * Math::PI;
		const float angle = getRandom(MIN_ANGLE, MAX_ANGLE);

		const D2D1_POINT_2F spawnDirection =
		{
			.x = cos(angle),
			.y = sin(angle)
		};

		const float SPAWN_DISTANCE = BOUNDARY_RADIUS - 30.0f;
		const D2D1_POINT_2F spawnPosition = Math::ScaleVector(spawnDirection, SPAWN_DISTANCE);

		Sprite& sprite = monster->sprite;
		sprite.SetPosition(spawnPosition);
		sprite.SetScale(scale);
		sprite.SetActive(true);
	}

	// 초기 HpBar를 세팅한다.
	{
		const D2D1_POINT_2F offset = getHpBarOffset(monster->sprite, hpBarOffset);
		monster->backgroundHpBar.SetPosition(offset);
		monster->hpBar.SetPosition(offset);
		monster->backgroundHpBar.SetActive(false);
		monster->hpBar.SetActive(false);
	}
}

void MainScene::spawnMonsterEffect(const MonsterSpawnEffectDesc& desc)
{
	Monster* monster = desc.monster;
	const D2D1_SIZE_F originalScale = desc.originalScale;
	const D2D1_SIZE_F effectScale = desc.effectScale;
	const float time = desc.time;
	const float deltaTime = desc.deltaTime;

	switch (monster->spawnState)
	{
		case eSpawnEffect_State::None:
		{
			monster->spawnState = eSpawnEffect_State::Bigger;
			break;
		}
		case eSpawnEffect_State::Bigger:
		{
			monster->spawnStartEffectTimer += desc.deltaTime;
			float biggerT = monster->spawnStartEffectTimer / time;
			biggerT = std::clamp(biggerT, 0.0f, 1.0f);
			D2D1_POINT_2F scale = Math::LerpVector({ .x = originalScale.width, .y = originalScale.height }, { .x = effectScale.width, .y = effectScale.height }, biggerT);

			if (biggerT >= 1.0f)
			{
				monster->spawnStartEffectTimer = 0.0f;
				monster->spawnState = eSpawnEffect_State::Smaller;
			}

			monster->sprite.SetScale({ .width = scale.x, .height = scale.y });
			break;
		}
		case eSpawnEffect_State::Smaller:
		{
			monster->spawnEndEffectTimer += desc.deltaTime;
			float smallerT = monster->spawnEndEffectTimer / time;
			smallerT = std::clamp(smallerT, 0.0f, 1.0f);
			D2D1_POINT_2F scale = Math::LerpVector({ .x = effectScale.width, .y = effectScale.height }, { .x = originalScale.width, .y = originalScale.height }, smallerT);

			if (smallerT >= 1.0f)
			{
				monster->spawnState = eSpawnEffect_State::End;
				monster->spawnEndEffectTimer = 0.0f;
			}

			monster->sprite.SetScale({ .width = scale.x, .height = scale.y });
			break;
		}
		case eSpawnEffect_State::End:
		{
			monster->state = eMonster_State::Life;
			break;
		}
	}
}

void MainScene::updateMonsterHp(Monster* monster, const float maxWidthBar, const uint32_t maxHp, const float deltaTime)
{
	ASSERT(monster != nullptr);

	Sprite& hpBar = monster->hpBar;
	D2D1_POINT_2F scale = { .x = hpBar.GetScale().width, .y = hpBar.GetScale().height };
	scale = Math::LerpVector(scale, { maxWidthBar * (float(monster->hp) / float(maxHp)), hpBar.GetScale().height }, 10.0f * deltaTime);
	hpBar.SetScale({ scale.x, scale.y });
}

D2D1_POINT_2F MainScene::getHpBarOffset(const Sprite& sprite, const D2D1_POINT_2F offset)
{
	const D2D1_SIZE_F scaleOffset =
	{
		.width = sprite.GetScale().width * mRectangleTexture.GetWidth() * 0.5f,
			.height = sprite.GetScale().height * mRectangleTexture.GetHeight() * 0.5f
	};

	const D2D1_POINT_2F result =
	{
		.x = sprite.GetPosition().x - scaleOffset.width + offset.x,
		.y = sprite.GetPosition().y - scaleOffset.height + offset.y
	};

	return result;
}

void MainScene::deadMonsterEffect(const MonsterDeadSoundDesc& desc)
{
	Monster* monster = desc.monster;
	const D2D1_SIZE_F originalScale = desc.originalScale;
	const D2D1_SIZE_F effectScale = desc.effectScale;
	const float time = desc.time;
	const float deltaTime = desc.deltaTime;

	if (monster->hp <= 0)
	{
		// 카메라 흔들기를 시작한다.
		const float amplitude = Constant::Get().GetHeight() * getRandom(0.008f, 0.012f);
		const float duration = getRandom(0.5f, 0.8f);
		const float frequency = getRandom(50.0f, 60.0f);
		initializeCameraShake(amplitude, duration, frequency);

		monster->hp = 0;
		monster->isHpBarActivated = false;
		monster->backgroundHpBar.SetActive(false);
		monster->hpBar.SetActive(false);

		monster->deadEffectTimer += deltaTime;

		float t = monster->deadEffectTimer / time;
		t = std::clamp(t, 0.0f, 1.0f);

		D2D1_POINT_2F scale = Math::LerpVector({ .x = originalScale.width, .y = originalScale.height }, { .x = effectScale.width, .y = effectScale.height }, t);
		monster->sprite.SetScale({ scale.x , scale.y });

		if (t >= 1.0f)
		{
			monster->sprite.SetActive(false);
			monster->deadEffectTimer = 0.0f;
		}
	}
}

void MainScene::spawnParticle(Particle* particles, uint32_t size, Monster* monster, uint32_t spawnCount)
{
	ASSERT(monster != nullptr);

	for (uint32_t i = 0; i < size; ++i)
	{
		Particle& particle = particles[i];

		if (particle.sprite.IsActive())
		{
			continue;
		}

		// 좌표를 스폰한다.
		{
			D2D1_POINT_2F& direction = particle.direction;
			D2D1_POINT_2F spawnPosition = monster->sprite.GetPosition();

			direction = Math::SubtractVector(spawnPosition, mHero.sprite.GetPosition());
			direction = Math::NormalizeVector(direction);
			direction = Math::RotateVector(direction, getRandom(-60.0f, 60.0));

			Sprite& sprite = particle.sprite;
			sprite.SetPosition(spawnPosition);
			sprite.SetActive(true);
			sprite.SetOpacity(1.0f);
		}

		--spawnCount;

		if (spawnCount <= 0)
		{
			break;
		}
	}
}

void MainScene::updateParticle(Particle* particles, const uint32_t size, const float deltaTime)
{
	for (uint32_t i = 0; i < size; ++i)
	{
		Particle& particle = particles[i];
		Sprite& sprite = particle.sprite;

		if (not sprite.IsActive())
		{
			continue;
		}

		D2D1_POINT_2F poisition = sprite.GetPosition();
		poisition = Math::AddVector(poisition,
			Math::ScaleVector(particle.direction, particle.speed * deltaTime));
		sprite.SetPosition(poisition);

		float opacity = sprite.GetOpacity();
		opacity -= 1.0f * deltaTime;
		sprite.SetOpacity(opacity);

		if (opacity <= 0.0f)
		{
			sprite.SetActive(false);
		}
	}
}

void MainScene::drawDiamondEffect(const DrawDiamondEffectDesc& desc)
{
	const DiamondEffect& effect = desc.effect;
	const D2D1_POINT_2F positionOffset = desc.positionOffset;
	const float angle = desc.angle;
	ID2D1HwndRenderTarget* renderTarget = desc.renderTarget;
	ID2D1SolidColorBrush* brush = desc.brush;
	const D2D1::Matrix3x2F& view = desc.view;

	const D2D1_POINT_2F position = effect.position;
	const D2D1_SIZE_F scale = effect.scale;
	const D2D1_POINT_2F thick = effect.thick;

	const Matrix3x2F worldView =
		Transformation::getWorldMatrix(
			{
				.x = position.x + positionOffset.x,
				.y = position.y + positionOffset.y
			}, angle) * view;
	renderTarget->SetTransform(worldView);


	const D2D1_RECT_F colliderSize =
	{
		.left = 0.0f,
		.top = 0.0f,
		.right = scale.width,
		.bottom = scale.height
	};

	renderTarget->DrawRectangle(colliderSize, brush, thick.x);
}

void MainScene::initializeMonster(const MonsterInitDesc& desc)
{			
	Monster* monster = desc.monster;
	const float maxHp = desc.maxHp;
	const D2D1_SIZE_F monsterScale = desc.monsterScale;
	const D2D1_SIZE_F hpBackgroundScale = desc.hpBackgroundScale;
	const D2D1_SIZE_F hpScale = desc.hpScale;


	// 기본 정보를 초기화한다.
	monster->state = eMonster_State::Spawn;
	monster->isBulletColliding = false;
	monster->isShieldColliding = false;
	monster->isOrbitColliding = false;
	monster->spawnState = eSpawnEffect_State::None;
	monster->spawnStartEffectTimer = {};
	monster->spawnEndEffectTimer = {};
	monster->deadEffectTimer = {};
	monster->moveSpeed = {};
	monster->hp = maxHp;
	monster->isHpBarActivated = false;

	// 몬스터 Sprite의 기본 정보를 초기화한다.
	Sprite& sprite = monster->sprite;
	sprite.SetScale({ monsterScale.width, monsterScale.height });
	sprite.SetActive(false);
	sprite.SetTexture(&mRectangleTexture);
	mSpriteLayers[uint32_t(Layer::Monster)].push_back(&sprite);

	// Hp바 배경 Sprite의 기본 정보를 초기화한다.
	Sprite& hpBackground = monster->backgroundHpBar;
	hpBackground.SetScale({ hpBackgroundScale.width, hpBackgroundScale.height });
	hpBackground.SetCenter({ .x = -0.5f, .y = 0.0f });
	hpBackground.SetActive(false);
	hpBackground.SetTexture(&mWhiteBarTexture);
	mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBackground);

	// Hp바 Sprite의 기본 정보를 초기화한다.
	Sprite& hpBar = monster->hpBar;
	hpBar.SetScale({ hpScale.width, hpScale.height });
	hpBar.SetCenter({ .x = -0.5f, .y = 0.0f });
	hpBar.SetActive(false);
	hpBar.SetTexture(&mRedBarTexture);
	mSpriteLayers[uint32_t(Layer::Monster)].push_back(&hpBar);
}

void MainScene::spawnLongEffect(Sprite* sprites, const uint32_t size, Texture* texture, const Monster& monster)
{
	ASSERT(sprites != nullptr);
	ASSERT(texture != nullptr);

	for (uint32_t i = 0; i < size; ++i)
	{
		Sprite& sprite = sprites[i];
		if (sprite.IsActive())
		{
			continue;
		}

		sprite.SetTexture(texture);

		const D2D1_POINT_2F position = monster.sprite.GetPosition();
		sprite.SetPosition(position);

		D2D1_POINT_2F direction = Math::SubtractVector(position, mHero.sprite.GetPosition());
		direction = Math::NormalizeVector(direction);

		float angle = Math::ConvertRadianToDegree(direction.y);
		sprite.SetAngle(-angle);

		sprite.SetActive(true);
		break;
	}
}

void MainScene::updateLongEffect(const LongEffectDesc& desc)
{
	Sprite* sprites = desc.sprite;
	const uint32_t size = desc.size;
	float* timers = desc.timer;
	const float time = desc.time;
	const D2D1_SIZE_F scale = desc.scale;
	const float deltaTime = desc.deltaTime;

	for (uint32_t i = 0; i < size; ++i)
	{
		Sprite& sprite = sprites[i];
		if (not sprite.IsActive())
		{
			continue;
		}

		timers[i] += deltaTime;
		float t = timers[i] / time;
		D2D1_POINT_2F effectScale = Math::LerpVector({ .x = scale.width, .y = scale.height },
			{ .x = 0.1f, .y = scale.height }, t);
		sprite.SetScale({ .width = effectScale.x, .height = effectScale.y });

		if (t >= 1.0f)
		{
			sprite.SetActive(false);
			timers[i] = 0.0f;
		}
	}
}

void MainScene::spawnDiamondEffect(DiamondEffect* effects, const uint32_t effectCount, const Monster& monster)
{
	ASSERT(effects != nullptr);

	for (uint32_t i = 0; i < effectCount; ++i)
	{
		DiamondEffect& effect = effects[i];

		if (effect.isActive)
		{
			continue;
		}

		effect.position = monster.sprite.GetPosition();
		effect.isActive = true;
		break;
	}
}

void MainScene::updateDiamondEffect(const DiamondEffectDesc& desc)
{
	DiamondEffect* effects = desc.effect;
	const uint32_t size = desc.size;
	const D2D1_SIZE_F effectScale = desc.scale;
	const float speed = desc.speed;
	const float time = desc.time;
	const D2D1_POINT_2F effectThick = desc.thick;
	const float deltaTime = desc.deltaTime;

	for (uint32_t i = 0; i < size; ++i)
	{
		DiamondEffect& effect = effects[i];

		if (not effect.isActive)
		{
			continue;
		}

		// 크기를 보간한다.
		D2D1_POINT_2F scale = Math::LerpVector({ .x = effectScale.width , .y = effectScale.height }, { .x = 0.1f , .y = 0.1f }, speed * deltaTime);
		effect.scale = { scale.x, scale.y };

		// 두께를 보간한다.
		effect.thickTimer += deltaTime;
		float t = effect.thickTimer / time;
		t = std::clamp(t, 0.0f, 1.0f);
		effect.thick = Math::LerpVector({ .x = effectThick.x , .y = effectThick.y }, { .x = 0.1f , .y = 0.1f }, t);

		if (t >= 1.0f)
		{
			effect.isActive = false;
			effect.thickTimer = 0.0f;
		}
	}

}

void MainScene::updateButtonState(const ButtonDesc& desc)
{
	Sprite* sprite = desc.sprite;
	const bool isColliding = desc.isColliding;
	Texture* originalTexture = desc.originalTexture;
	Texture* effectTexture = desc.effectTexture;
	bool* isSoundPlay = desc.isSoundPlay;

	if (isColliding)
	{
		sprite->SetTexture(effectTexture);

		if (not *isSoundPlay)
		{
			mButtonSound.Replay();
			*isSoundPlay = true;
		}
	}
	else
	{
		*isSoundPlay = false;
		sprite->SetTexture(originalTexture);
	}
}
