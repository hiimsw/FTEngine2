#include "pch.h"
#include "Scene.h"

#include "Helper.h"

Helper* Scene::GetHelper() const
{
	return mHelper;
}

const std::vector<Sprite*>* Scene::GetSpriteLayer(const uint32_t layerIndex) const
{
	MASSERT(layerIndex < mSpriteLayerCount, "레이어 인덱스가 범위를 벗어 납니다.");

	return mSpriteLayers[layerIndex];
}

uint32_t Scene::GetSpriteLayerCount() const
{
	return mSpriteLayerCount;
}

void Scene::SetSpriteLayers(const std::vector<Sprite*>* layers, const uint32_t layerCount)
{
	ASSERT(layers != nullptr);
	MASSERT(layerCount <= MAX_LAYER_COUNT, "지원하는 레이어 최대 개수를 초과합니다.");

	for (uint32_t i = 0; i < layerCount; ++i)
	{
		mSpriteLayers[i] = &layers[i];
	}

	mSpriteLayerCount = layerCount;
}

const std::vector<Label*>* Scene::GetLabelsOrNull() const
{
	return mLabels;
}

void Scene::SetLabels(std::vector<Label*>* labels)
{
	ASSERT(labels != nullptr);

	mLabels = labels;
}

const Camera* Scene::GetCameraOrNull() const
{
	return mCamera;
}

void Scene::SetCamera(const Camera* camera)
{
	ASSERT(camera != nullptr);

	mCamera = camera;
}

void Scene::_Preinitialize(Helper* helper)
{
	ASSERT(helper != nullptr);

	mHelper = helper;
}