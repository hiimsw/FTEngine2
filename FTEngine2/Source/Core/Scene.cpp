#include "pch.h"
#include "Scene.h"

#include "Helper.h"

Helper* Scene::GetHelper() const
{
	return mHelper;
}

const std::vector<Sprite*>* Scene::GetSpritesOrNull() const
{
	return mSprites;
}

void Scene::SetSprites(std::vector<Sprite*>* sprites)
{
	ASSERT(sprites != nullptr);

	mSprites = sprites;
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