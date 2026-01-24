#pragma once
#include "layer.h"
#include "raylib.h"
#include <vector>
#include <cstdint>
#include <unordered_map>

enum EntityFlags : uint32_t {
	NONE = 0,
	VISIBLE = 1 << 0,
	MOVABLE = 1 << 1,
	COLLIDABLE = 1 << 2,
	PLAYER = 1 << 3,
	BLOCK = 1 << 4,
	BALL = 1 << 5
};

struct Entity
{
	uint32_t flags = EntityFlags::NONE;

	// Bind the raylib texture ID to the entity
	unsigned int textureID { 0 };
	int width { 0 };
	int height { 0 };

	Vector2 position { 0.0f, 0.0f };
	Vector2 targetPosition { 0.0f, 0.0f };
	Vector2 direction { 0.0f, 0.0f };
	float moveSpeed { 0.0f };

	inline Rectangle GetCollider() const 
	{
		return Rectangle { position.x, position.y, (float)width, (float)height };
	}

	inline bool HasFlag(uint32_t flag) const 
	{
		return (flags & flag) != 0;
	}

	inline void AddFlag(uint32_t flag) 
	{
		flags |= flag;
	}

	inline void RemoveFlag(uint32_t flag) 
	{
		flags &= ~flag;
	}
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
	std::unordered_map<unsigned int, Texture2D> m_Textures;
	std::vector<Entity> m_Entities;

	const Color m_BackgroundColour { 32, 32, 32, 0 };
	CanvasTransform CalculateCanvasTransform() const;
	bool m_Pause { false };

	int m_Score { 0 };

	// placeholder;
	Font m_Font;

public:
	GameLayer();
	~GameLayer() override;

	bool ProcessInput() override;
	void Update(float deltaTime) override;
	void Draw() override;
};