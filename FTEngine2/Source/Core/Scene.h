#pragma once

class Camera;
class Helper;
class Label;
class Sprite;

class Scene
{
public:
	enum class SCENE_TYPE
	{
		Start,
		Village,
		Stage01,
		End
	};

public:
	Scene() = default;
	Scene(const Scene&) = delete;
	const Scene& operator=(const Scene&) = delete;
	virtual ~Scene() = default;

	virtual void Initialize() = 0;
	virtual bool Update(const float deltaTime) = 0;
	virtual void Finalize() = 0;

	[[nodiscard]] Helper* GetHelper() const;

	[[nodiscard]] const std::vector<Sprite*>* GetSpritesOrNull() const;
	void SetSprites(std::vector<Sprite*>* sprites);

	[[nodiscard]] const std::vector<Label*>* GetLabelsOrNull() const;
	void SetLabels(std::vector<Label*>* labels);

	[[nodiscard]] const Camera* GetCameraOrNull() const;
	void SetCamera(const Camera* camera);

public:
	void _Preinitialize(Helper* helper);

public:
	[[nodiscard]] const SCENE_TYPE GetType() const;
	void SetType(const SCENE_TYPE type);

private:
	Helper* mHelper = nullptr;
	const std::vector<Sprite*>* mSprites = nullptr;
	const std::vector<Label*>* mLabels = nullptr;
	const Camera* mCamera = nullptr;

protected:
	SCENE_TYPE mSceneType{};
};