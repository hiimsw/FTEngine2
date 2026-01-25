#include "pch.h"
#include "Core.h"

#include "Camera.h"
#include "Constant.h"
#include "Font.h"
#include "Label.h"
#include "Sprite.h"
#include "Texture.h"
#include "Transformation.h"

using namespace D2D1;

void Core::Initialize(HWND hWnd, Scene* scene)
{
	ASSERT(hWnd != nullptr and scene != nullptr);

	HR(CoInitialize(nullptr));
	HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &mFactory));
	HR(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mWICImagingFactory)));
	HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(mDwriteFactory), reinterpret_cast<IUnknown**>(&mDwriteFactory)));

	D2D1_SIZE_U windowRect = { .width = UINT32(Constant::Get().GetWidth()), .height = UINT32(Constant::Get().GetHeight()) };
	HR(mFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, windowRect), &mRenderTarget));

	FC(FMOD::System_Create(&mSoundSystem));
	FC(mSoundSystem->init(32, FMOD_INIT_NORMAL, nullptr));

	mHelper._Initialize(mWICImagingFactory, mDwriteFactory, mRenderTarget, mSoundSystem);

	ChangeScene(scene);
}

bool Core::Update(const float deltaTime)
{
	if (not mScene->Update(deltaTime))
	{
		return false;
	}

	// Draw
	{
		mRenderTarget->BeginDraw();
		mRenderTarget->Clear(ColorF(ColorF::Black));

		const Camera* camera = mScene->GetCameraOrNull();
		Matrix3x2F view = Matrix3x2F::Identity();
		Matrix3x2F viewForUI = Matrix3x2F::Identity();

		if (camera != nullptr)
		{
			D2D1_POINT_2F centerOffset =
			{
				.x = (Constant::Get().GetWidth() - 1.0f) * 0.5f,
				.y = (Constant::Get().GetHeight() - 1.0f) * 0.5f
			};

			D2D1_POINT_2F position = camera->GetPosition();
			position.x -= centerOffset.x;
			position.y -= centerOffset.y;

			float angle = camera->GetAngle();
			float fieldOfView = camera->GetFieldOfView();

			view = Matrix3x2F::Translation(-centerOffset.x, -centerOffset.y)
				* Matrix3x2F::Scale({ .width = fieldOfView, .height = fieldOfView })
				* Matrix3x2F::Rotation(angle)
				* Matrix3x2F::Translation(centerOffset.x, centerOffset.y)
				* Matrix3x2F::Translation(position.x, -position.y);
			view.Invert();

			viewForUI = Matrix3x2F::Translation(-centerOffset.x, centerOffset.y);
			viewForUI.Invert();
		}

		mScene->PreDraw(view, viewForUI);

		const uint32_t spriteLayerCount = mScene->GetSpriteLayerCount();
		for (uint32_t i = 0; i < spriteLayerCount; ++i)
		{
			const std::vector<Sprite*>* spriteLayer = mScene->GetSpriteLayer(i);
			for (const Sprite* sprite : *spriteLayer)
			{
				if (not sprite->IsActive())
				{
					continue;
				}

				Texture* texture = sprite->_GetTextureOrNull();
				if (texture == nullptr)
				{
					continue;
				}

				ID2D1Bitmap* bitmap = texture->_GetBitmap();

				D2D1_POINT_2F center = sprite->GetCenter();
				center.x = -(center.x + 0.5f) * texture->GetWidth();
				center.y = (center.y - 0.5f) * texture->GetHeight();

				D2D1_POINT_2F position = sprite->GetPosition();
				D2D1_SIZE_F scale = sprite->GetScale();
				float angle = sprite->GetAngle();
				float opacity = sprite->GetOpacity();

				Matrix3x2F worldView = Matrix3x2F::Translation(center.x, center.y) * Transformation::getWorldMatrix(position, angle, scale);
				worldView = worldView * (sprite->IsUI() == false ? view : viewForUI);
				mRenderTarget->SetTransform(worldView);

				mRenderTarget->DrawBitmap(bitmap, nullptr, opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
			}
		}

		// Render labels
		{
			// TODO(이수원): bursh를 멤버 변수로 옮겨 캐싱하여 처리하자.
			static ID2D1SolidColorBrush* brush = nullptr;
			if (brush == nullptr)
			{
				HR(mRenderTarget->CreateSolidColorBrush(ColorF(1.0f, 1.0f, 1.0f), &brush));
			}

			D2D1_RECT_F drawArea = RectF(0.0f, 0.0f, float(Constant::Get().GetWidth()), float(Constant::Get().GetHeight()));

			const std::vector<Label*>* lables = mScene->GetLabelsOrNull();
			if (lables != nullptr)
			{
				for (const Label* label : *lables)
				{
					if (not label->IsActive())
					{
						continue;
					}

					Font* font = label->_GetFontOrNull();
					if (font == nullptr)
					{
						continue;
					}

					IDWriteTextFormat* textFormat = font->_GetTextFormat();
					const std::wstring& text = label->GetTextByWStr();

					D2D1_POINT_2F center = label->GetCenter();
					D2D1_SIZE_F textSize = label->GetTextSize();
					center.x = -(center.x + 0.5f) * (textSize.width - 1.0f);
					center.y = (center.y - 0.5f) * (textSize.height - 1.0f);

					D2D1_POINT_2F position = label->GetPosition();
					D2D1_SIZE_F scale = label->GetScale();
					float angle = label->GetAngle();
					float opacity = label->GetOpacity();

					Matrix3x2F worldView = Matrix3x2F::Translation(center.x, center.y) * Transformation::getWorldMatrix(position, angle, scale);
					worldView = worldView * (label->IsUI() == false ? view : viewForUI);
					mRenderTarget->SetTransform(worldView);

					mRenderTarget->DrawText(text.c_str(), UINT32(text.size()), textFormat, drawArea, brush);
				}
			}
		}

		mScene->PostDraw(view, viewForUI);

		HR(mRenderTarget->EndDraw());
	}

	return true;
}

void Core::Finalize()
{
	RELEASE_D2D1(mRenderTarget);
	RELEASE_D2D1(mDwriteFactory);
	RELEASE_D2D1(mFactory);
	RELEASE_D2D1(mWICImagingFactory);

	mScene->Finalize();
	RELEASE(mScene);

	CoUninitialize();
}

void Core::ChangeScene(Scene* scene)
{
	ASSERT(scene != nullptr);

	if (mScene != nullptr)
	{
		mScene->Finalize();
		RELEASE(mScene);
	}

	mScene = scene;
	mScene->_Preinitialize(&mHelper);
	mScene->Initialize();
}