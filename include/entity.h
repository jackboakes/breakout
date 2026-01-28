#pragma once
#include "raylib.h"
#include <cstdint>

struct UIElement
{
	Rectangle bounds { 0 };
	unsigned int textureID { 0 };
	unsigned int pressedTextureID { 0 };
	bool isPressed { false };
};

enum class EntityType
{
	NONE = 0,
	PLAYER,
	BALL,
	BLOCK
};

enum EntityFlags : uint8_t
{
	NONE = 0,
	VISIBLE = 1 << 0,
	MOVABLE = 1 << 1,
	COLLIDABLE = 1 << 2,
	ANIMATING = 1 << 3
};

struct Entity
{
	EntityType type { EntityType::NONE };
	uint8_t flags { EntityFlags::NONE };

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