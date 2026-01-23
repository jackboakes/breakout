#pragma once
#include "layer.h"
#include <vector>

enum class EntityType {
	None,
	Paddle,
	Ball,
	Block
};

struct Entity
{
	EntityType type = EntityType::None;
	// Bind the raylib texture id to the entity
	unsigned int textureID;
	bool isVisible { true };

	int width;
	int height;

	Vector2 position { 0.0f, 0.0f };
	Vector2 targetPosition { 0.0f, 0.0f };

	float moveSpeed { 0.0f };
};

struct CanvasTransform
{
	float scale;
	Vector2 offset;
};

class GameLayer : public Layer
{
private:
	Camera2D m_Camera2D { 0 };
	// TODO:: Maybe store textures in a map later
	std::vector<Texture2D> m_Textures;
	std::vector<Entity> m_Entities;

	const Color m_BackgroundColour { 32, 32, 32, 0 };
	CanvasTransform CalculateCanvasTransform() const;

public:
	GameLayer();
	~GameLayer() override;

	bool ProcessInput() override;
	void Update(float deltaTime) override;
	void Draw() override;
};