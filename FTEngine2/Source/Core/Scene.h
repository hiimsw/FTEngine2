#pragma once

class Camera;
class Helper;
class Label;
class Sprite;

class Scene
{
public:
	Scene() = default;
	Scene(const Scene&) = delete;
	const Scene& operator=(const Scene&) = delete;
	virtual ~Scene() = default;

	virtual void Initialize() = 0;
	virtual bool Update(const float deltaTime) = 0;
	virtual void Finalize() = 0;

	[[nodiscard]] Helper* GetHelper() const;

	[[nodiscard]] const std::vector<Sprite*>* GetSpriteLayer(const uint32_t layerIndex) const;
	[[nodiscard]] uint32_t GetSpriteLayerCount() const;
	void SetSpriteLayers(const std::vector<Sprite*>* layers, const uint32_t layerCount);

	[[nodiscard]] const std::vector<Label*>* GetLabelsOrNull() const;
	void SetLabels(std::vector<Label*>* labels);

	[[nodiscard]] const Camera* GetCameraOrNull() const;
	void SetCamera(const Camera* camera);

public:
	void _Preinitialize(Helper* helper);

private:
	static constexpr uint32_t MAX_LAYER_COUNT = 8;

	Helper* mHelper = nullptr;

	const std::vector<Sprite*>* mSpriteLayers[MAX_LAYER_COUNT]{};
	uint32_t mSpriteLayerCount = 0;

	const std::vector<Label*>* mLabels = nullptr;
	const Camera* mCamera = nullptr;
};